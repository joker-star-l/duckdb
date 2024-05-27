import re
import subprocess
import time
import os
import fuzzer_helper
import multiprocessing
import sqlite3

# this script can be used as a library, but can also be directly called
# example usage:
# python3 scripts/reduce_sql.py --load load.sql --exec exec.sql

try:
    multiprocessing.set_start_method('fork')
except RuntimeError:
    pass
get_reduced_query = '''
SELECT * FROM reduce_sql_statement('${QUERY}');
'''


class MultiStatementReducer:
    delimiter = '\n'

    def __init__(self, multi_statement):
        statements = list(map(lambda x: x.strip(), multi_statement.split(MultiStatementReducer.delimiter)))
        self.statements = []
        for stmt in statements:
            if len(stmt) > 0:
                self.statements.append(stmt)
        self.statements_start = 0
        self.required = []
        self.test_omit = ""

    def is_multi_statement(sql_statement):
        if len(sql_statement.split('\n')) > 1:
            return True
        return False

    def combine_statements(self):
        new_multi_statement = MultiStatementReducer.delimiter.join(self.required)
        new_multi_statement += MultiStatementReducer.delimiter.join(self.statements[self.statements_start :])
        return new_multi_statement

    def done(self):
        return self.statements_start >= len(self.statements)

    def get_last_statement(self):
        return self.statements[-1]

    def generate_next(self):
        if self.done():
            raise "Attempting to generate next with no more statements"
        self.test_omit = self.statements[self.statements_start]
        self.statements_start += 1
        return self.combine_statements()

    def set_omitted_required(self):
        self.required.append(self.test_omit)


def sanitize_error(err):
    err = re.sub(r'Error: near line \d+: ', '', err)
    err = err.replace(os.getcwd() + '/', '')
    err = err.replace(os.getcwd(), '')
    if 'AddressSanitizer' in err:
        match = re.search(r'[ \t]+[#]0 ([A-Za-z0-9]+) ([^\n]+)', err).groups()[1]
        err = 'AddressSanitizer error ' + match
    return err


def run_shell_command(shell, cmd):
    command = [shell, '-csv', '--batch', '-init', '/dev/null']

    res = subprocess.run(command, input=bytearray(cmd, 'utf8'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout = res.stdout.decode('utf8').strip()
    stderr = res.stderr.decode('utf8').strip()
    return (stdout, stderr, res.returncode)


def get_reduced_sql(shell, sql_query):
    reduce_query = get_reduced_query.replace('${QUERY}', sql_query.replace("'", "''"))
    (stdout, stderr, returncode) = run_shell_command(shell, reduce_query)
    if returncode != 0:
        print(stdout)
        print(stderr)
        raise Exception("Failed to reduce query")
    reduce_candidates = []
    for line in stdout.split('\n'):
        reduce_candidates.append(line.strip('"').replace('""', '"'))
    return reduce_candidates[1:]


def reduce(sql_query, data_load, shell, error_msg, max_time_seconds=300):
    start = time.time()
    while True:
        found_new_candidate = False
        reduce_candidates = get_reduced_sql(shell, sql_query)
        for reduce_candidate in reduce_candidates:
            if reduce_candidate == sql_query:
                continue
            current_time = time.time()
            if current_time - start > max_time_seconds:
                break

            (stdout, stderr, returncode) = run_shell_command(shell, data_load + reduce_candidate)
            new_error = sanitize_error(stderr)
            if new_error == error_msg:
                sql_query = reduce_candidate
                found_new_candidate = True
                print("Found new reduced query")
                print("=======================")
                print(sql_query)
                print("=======================")
                break
        if not found_new_candidate:
            break
    return sql_query


def is_ddl_query(query):
    query = query.lower()
    if 'create' in query or 'insert' in query or 'update' in query or 'delete' in query:
        return True
    return False


def initial_cleanup(query_log):
    query_log = query_log.replace('SELECT * FROM pragma_version()\n', '')
    return query_log


def run_queries_until_crash_mp(queries, result_file):
    import duckdb

    con = duckdb.connect()
    sqlite_con = sqlite3.connect(result_file)
    sqlite_con.execute('CREATE TABLE queries(id INT, text VARCHAR)')
    sqlite_con.execute('CREATE TABLE result(text VARCHAR)')
    sqlite_con.execute("INSERT INTO result VALUES ('__CRASH__')")
    id = 1
    is_internal_error = False
    for q in queries:
        # insert the current query into the database
        # we do this pre-emptively in case the program crashes
        sqlite_con.execute('INSERT INTO queries VALUES (?, ?)', (id, q))
        sqlite_con.commit()

        keep_query = False
        try:
            con.execute(q)
            keep_query = is_ddl_query(q)
        except Exception as e:
            exception_error = str(e)
            is_internal_error = fuzzer_helper.is_internal_error(exception_error)
            if is_internal_error:
                keep_query = True
                sqlite_con.execute('UPDATE result SET text=?', (exception_error,))
        if not keep_query:
            sqlite_con.execute('DELETE FROM queries WHERE id=?', (id,))
        if is_internal_error:
            # found internal error: no need to try further queries
            break
        id += 1
    if not is_internal_error:
        # failed to reproduce: delete result
        sqlite_con.execute('DELETE FROM result')
        sqlite_con.commit()
    sqlite_con.close()


def run_queries_until_crash(queries):
    sqlite_file = 'cleaned_queries.db'
    if os.path.isfile(sqlite_file):
        os.remove(sqlite_file)
    # run the queries in a separate process because it might crash
    p = multiprocessing.Process(target=run_queries_until_crash_mp, args=(queries, sqlite_file))
    p.start()
    p.join()

    # read the queries back from the file
    sqlite_con = sqlite3.connect(sqlite_file)
    queries = sqlite_con.execute('SELECT text FROM queries ORDER BY id').fetchall()
    results = sqlite_con.execute('SELECT text FROM result').fetchall()
    sqlite_con.close()
    if len(results) == 0:
        # no internal error or crash found
        return (None, None)
    assert len(results) == 1
    return ([x[0] for x in queries], results[0][0])


def cleanup_irrelevant_queries(query_log):
    query_log = initial_cleanup(query_log)

    queries = [x for x in query_log.split(';\n') if len(x) > 0]
    return run_queries_until_crash(queries)


# def reduce_internal(start, sql_query, data_load, queries_final, shell, error_msg, max_time_seconds=300):


def reduce_query_log_query(start, shell, queries, query_index, max_time_seconds):
    new_query_list = queries[:]
    sql_query = queries[query_index]
    while True:
        found_new_candidate = False
        reduce_candidates = get_reduced_sql(shell, sql_query)
        for reduce_candidate in reduce_candidates:
            if reduce_candidate == sql_query:
                continue
            current_time = time.time()
            if current_time - start > max_time_seconds:
                break

            new_query_list[query_index] = reduce_candidate
            (_, error) = run_queries_until_crash(new_query_list)

            if error is not None:
                sql_query = reduce_candidate
                found_new_candidate = True
                print("Found new reduced query")
                print("=======================")
                print(sql_query)
                print("========ERROR==========")
                print(error)
                print("=======================")
                print("")
                break
        if not found_new_candidate:
            break
    return sql_query


def statement_error(shell, statements) -> bool:
    (stdout, stderr, returncode) = run_shell_command(shell, statements)
    expected_error = sanitize_error(stderr).strip()
    return len(expected_error) != 0


# reduces collection of many sql statements to just the sql statements that produce the
# error. A heuristic is also applied to check if only the last statement produces the error
def reduce_multi_statement(sql_statements, shell, data_load):
    multi_statement_reducer = MultiStatementReducer(sql_statements)
    reduced_sql = ""
    last_statement = multi_statement_reducer.get_last_statement()
    print(f"testing if {last_statement} creates the error")
    if statement_error(shell, data_load + last_statement):
        return last_statement
    while not multi_statement_reducer.done():
        reduced_sql = multi_statement_reducer.generate_next()
        print(f"testing if {reduced_sql} creates the error")
        # if there is no error, it means the omitted line is necessary
        # to cause the error, so we add it to our required statements
        if not statement_error(shell, data_load + reduced_sql):
            multi_statement_reducer.set_omitted_required()
    return reduced_sql


def reduce_query_log(queries, shell, max_time_seconds=300):
    start = time.time()
    current_index = 0
    # first try to remove as many queries as possible
    while current_index < len(queries):
        print("Attempting to remove query at position %d (of %d total queries)" % (current_index, len(queries)))
        current_time = time.time()
        if current_time - start > max_time_seconds:
            break
        # remove the query at "current_index"
        new_queries = queries[:current_index] + queries[current_index + 1 :]
        # try to run the queries and check if we still get the same error
        (new_queries_x, current_error) = run_queries_until_crash(new_queries)
        if current_error is None:
            # cannot remove this query without invalidating the test case
            current_index += 1
        else:
            # we can remove this query
            queries = new_queries
    # now try to reduce individual queries
    for i in range(len(queries)):
        if is_ddl_query(queries[i]):
            continue
        current_time = time.time()
        if current_time - start > max_time_seconds:
            break
        queries[i] = reduce_query_log_query(start, shell, queries, i, max_time_seconds)
    return queries


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description='Reduce a problematic SQL query')
    parser.add_argument(
        '--shell', dest='shell', action='store', help='Path to the shell executable', default='build/debug/duckdb'
    )
    parser.add_argument('--load', dest='load', action='store', help='Path to the data load script', required=True)
    parser.add_argument('--exec', dest='exec', action='store', help='Path to the executable script', required=True)
    parser.add_argument(
        '--inplace', dest='inplace', action='store_true', help='If true, overrides the exec script with the final query'
    )
    parser.add_argument(
        '--max-time', dest='max_time', action='store', help='Maximum time in seconds to run the reducer', default=300
    )

    args = parser.parse_args()
    print("Starting reduce process")

    shell = args.shell
    data_load = open(args.load).read()
    sql_query = open(args.exec).read()
    (stdout, stderr, returncode) = run_shell_command(shell, data_load + sql_query)
    expected_error = sanitize_error(stderr).strip()
    if len(expected_error) == 0:
        print("===================================================")
        print("Could not find expected error - no error encountered")
        print("===================================================")
        exit(1)

    print("===================================================")
    print("Found expected error")
    print("===================================================")
    print(expected_error)
    print("===================================================")

    if MultiStatementReducer.is_multi_statement(sql_query):
        sql_query = reduce_multi_statement(sql_query, shell, data_load)

    fake_load_statements = ""
    # if it is still a multi statement, move the previous statements
    # to the data_load and only reduce the last statement
    if MultiStatementReducer.is_multi_statement(sql_query):
        # strip whitespace and the last ';' if it exists
        # then split into individual statements with .split(';')
        queries = sql_query.strip().strip(';').split(';')
        fake_load_statements = ";\n".join(queries[:-1]).strip().strip(';')
        data_load += ";\n".join(queries[:-1])
        sql_query = queries[-1].strip()

    final_query = reduce(sql_query, data_load, shell, expected_error, int(args.max_time))
    if len(fake_load_statements) > 0:
        final_query = fake_load_statements + ";\n" + final_query
    print("Found final reduced query")
    print("===================================================")
    print(final_query)
    print("===================================================")
    if args.inplace:
        print(f"Writing to file {args.exec}")
        with open(args.exec, 'w+') as f:
            f.write(final_query)


# Example usage:
# error_msg = 'INTERNAL Error: Assertion triggered in file "/Users/myth/Programs/duckdb-bugfix/src/common/types/data_chunk.cpp" on line 41: !types.empty()'
# shell = 'build/debug/duckdb'
# data_load = 'create table all_types as select * from test_all_types();'
# sql_query = '''
# select
#   subq_0.c0 as c0,
#   contains(
#     cast(cast(nullif(
#         argmax(
#           cast(case when 0 then (select varchar from main.all_types limit 1 offset 5)
#                else (select varchar from main.all_types limit 1 offset 5)
#                end
#              as varchar),
#           cast(decode(
#             cast(cast(null as blob) as blob)) as varchar)) over (partition by subq_0.c1 order by subq_0.c1),
#       current_schema()) as varchar) as varchar),
#     cast(cast(nullif(cast(null as varchar),
#       cast(null as varchar)) as varchar) as varchar)) as c1,
#   (select min(time) from main.all_types)
#      as c2,
#   subq_0.c1 as c3,
#   subq_0.c1 as c4,
#   cast(nullif(subq_0.c1,
#     subq_0.c1) as decimal(4,1)) as c5
# from
#   (select
#         ref_0.timestamp_ns as c0,
#         case when (EXISTS (
#               select
#                   ref_0.timestamp_ns as c0,
#                   ref_0.timestamp_ns as c1,
#                   (select timestamp_tz from main.all_types limit 1 offset 4)
#                      as c2,
#                   ref_1.int_array as c3,
#                   ref_1.dec_4_1 as c4,
#                   ref_0.utinyint as c5,
#                   ref_1.int as c6,
#                   ref_0.double as c7,
#                   ref_0.medium_enum as c8,
#                   ref_1.array_of_structs as c9,
#                   ref_1.varchar as c10
#                 from
#                   main.all_types as ref_1
#                 where ref_1.varchar ~~~ ref_1.varchar
#                 limit 28))
#             or (ref_0.varchar ~~~ ref_0.varchar) then ref_0.dec_4_1 else ref_0.dec_4_1 end
#            as c1
#       from
#         main.all_types as ref_0
#       where (0)
#         and (ref_0.varchar ~~ ref_0.varchar)) as subq_0
# where writefile() !~~* writefile()
# limit 88
# '''
#
# print(reduce(sql_query, data_load, shell, error_msg))
