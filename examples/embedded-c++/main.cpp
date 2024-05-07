#include <iostream>
#include "duckdb.hpp"
#include "duckdb/common/enums/logical_operator_type.hpp"
#include "duckdb/planner/operator/logical_insert.hpp"
#include "duckdb/planner/expression/bound_onnx_expression.hpp"

using namespace duckdb;

DuckDB db(nullptr);
Connection con(db);

void exec_and_explain(const std::string& sql) {
	auto result = con.Query(sql);
	std::cout << "=================" << std::endl;
	result->Print();
	auto plan = con.ExtractPlan(sql);
	std::cout << "=================" << std::endl;
	plan->Print();
	
	if ((*plan).type == LogicalOperatorType::LOGICAL_INSERT) {
		auto& op = (*plan).Cast<LogicalInsert>();
	}
}

void onnx_expression() {
	auto expression = make_uniq<BoundOnnxExpression>("../model.onnx");
	expression->model->PrintDebugString();
	std::cout << "" << std::endl;
}

int main() {
	// exec_and_explain("CREATE TABLE integers(i INTEGER, j INTEGER);");
	// exec_and_explain("INSERT INTO integers VALUES (1, 2), (3, 4), (4, 5), (5, 5), (5, 6);");
	// exec_and_explain("SELECT *, abs(i) FROM integers where j = 5;");

	onnx_expression();

	return 0;
}
