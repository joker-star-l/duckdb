#include "duckdb/parser/statement/drop_statement.hpp"
#include "duckdb/planner/binder.hpp"
#include "duckdb/planner/operator/logical_simple.hpp"
#include "duckdb/catalog/catalog.hpp"
#include "duckdb/catalog/standard_entry.hpp"
#include "duckdb/catalog/catalog_entry/schema_catalog_entry.hpp"
#include "duckdb/parser/parsed_data/drop_info.hpp"

namespace duckdb {

BoundStatement Binder::Bind(DropStatement &stmt) {
	BoundStatement result;

	switch (stmt.info->type) {
	case CatalogType::PREPARED_STATEMENT:
		// dropping prepared statements is always possible
		// it also does not require a valid transaction
		properties.requires_valid_transaction = false;
		break;
	case CatalogType::SCHEMA_ENTRY: {
		// dropping a schema is never read-only because there are no temporary schemas
		auto &catalog = Catalog::GetCatalog(context, stmt.info->catalog);
		properties.modified_databases.insert(catalog.GetName());
		break;
	}
	case CatalogType::VIEW_ENTRY:
	case CatalogType::SEQUENCE_ENTRY:
	case CatalogType::MACRO_ENTRY:
	case CatalogType::TABLE_MACRO_ENTRY:
	case CatalogType::INDEX_ENTRY:
	case CatalogType::TABLE_ENTRY:
	case CatalogType::TYPE_ENTRY: {
		BindSchemaOrCatalog(stmt.info->catalog, stmt.info->schema);
		auto entry = (StandardEntry *)Catalog::GetEntry(context, stmt.info->type, stmt.info->catalog, stmt.info->schema,
		                                                stmt.info->name, true);
		if (!entry) {
			break;
		}
		stmt.info->catalog = entry->catalog->GetName();
		if (!entry->temporary) {
			// we can only drop temporary tables in read-only mode
			properties.modified_databases.insert(stmt.info->catalog);
		}
		stmt.info->schema = entry->schema->name;
		break;
	}
	case CatalogType::DATABASE_ENTRY: {
		// allow extensions to handle drop database impl
		auto &base = (DropInfo &)*stmt.info;
		string database_name = base.name;

		auto &config = DBConfig::GetConfig(context);

		// for now only handling the case where there is one storage extension registered
		if (config.storage_extensions.empty()) {
			// use DuckDB default impl
			// attaching and detaching is read-only
			stmt.info->catalog = SYSTEM_CATALOG;
		} else if (config.storage_extensions.size() > 1) {
			throw BinderException("DROP DATABASE only supported when there is one storage extension registered.");
		}
		auto entry = config.storage_extensions.begin();
		auto &storage_extension = entry->second;
		if (storage_extension->drop_database_extension_function == nullptr) {
			throw BinderException("DROP DATABASE is not supported in \"%s\" extension.", entry->first);
		}
		auto drop_database_function_ref = storage_extension->drop_database_extension_function(
		    context, database_name, storage_extension->storage_info.get());
		if (drop_database_function_ref) {
			auto bound_drop_database_func = Bind(*drop_database_function_ref);
			result.plan = CreatePlan(*bound_drop_database_func);
			break;
		}
		break;
	}
	default:
		throw BinderException("Unknown catalog type for drop statement!");
	}
	result.plan = make_unique<LogicalSimple>(LogicalOperatorType::LOGICAL_DROP, std::move(stmt.info));
	result.names = {"Success"};
	result.types = {LogicalType::BOOLEAN};
	properties.allow_stream_result = false;
	properties.return_type = StatementReturnType::NOTHING;
	return result;
}

} // namespace duckdb
