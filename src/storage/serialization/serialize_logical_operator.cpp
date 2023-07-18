//===----------------------------------------------------------------------===//
// This file is automatically generated by scripts/generate_serialization.py
// Do not edit this file manually, your changes will be overwritten
//===----------------------------------------------------------------------===//

#include "duckdb/common/serializer/format_serializer.hpp"
#include "duckdb/common/serializer/format_deserializer.hpp"
#include "duckdb/planner/operator/list.hpp"

namespace duckdb {

void LogicalOperator::FormatSerialize(FormatSerializer &serializer) const {
	serializer.WriteProperty("type", type);
	serializer.WriteProperty("children", children);
}

unique_ptr<LogicalOperator> LogicalOperator::FormatDeserialize(FormatDeserializer &deserializer) {
	auto type = deserializer.ReadProperty<LogicalOperatorType>("type");
	auto children = deserializer.ReadProperty<vector<unique_ptr<LogicalOperator>>>("children");
	deserializer.Set<LogicalOperatorType>(type);
	unique_ptr<LogicalOperator> result;
	switch (type) {
	case LogicalOperatorType::LOGICAL_AGGREGATE_AND_GROUP_BY:
		result = LogicalAggregate::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_CHUNK_GET:
		result = LogicalColumnDataGet::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_COMPARISON_JOIN:
		result = LogicalComparisonJoin::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_CTE_REF:
		result = LogicalCTERef::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_DELIM_GET:
		result = LogicalDelimGet::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_DELIM_JOIN:
		result = LogicalDelimJoin::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_DISTINCT:
		result = LogicalDistinct::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_DUMMY_SCAN:
		result = LogicalDummyScan::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_EMPTY_RESULT:
		result = LogicalEmptyResult::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_EXPRESSION_GET:
		result = LogicalExpressionGet::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_FILTER:
		result = LogicalFilter::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_LIMIT:
		result = LogicalLimit::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_LIMIT_PERCENT:
		result = LogicalLimitPercent::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_ORDER_BY:
		result = LogicalOrder::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_PROJECTION:
		result = LogicalProjection::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_SAMPLE:
		result = LogicalSample::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_TOP_N:
		result = LogicalTopN::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_UNNEST:
		result = LogicalUnnest::FormatDeserialize(deserializer);
		break;
	case LogicalOperatorType::LOGICAL_WINDOW:
		result = LogicalWindow::FormatDeserialize(deserializer);
		break;
	default:
		throw SerializationException("Unsupported type for deserialization of LogicalOperator!");
	}
	deserializer.Unset<LogicalOperatorType>();
	result->children = std::move(children);
	return result;
}

void LogicalAggregate::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("expressions", expressions);
	serializer.WriteProperty("group_index", group_index);
	serializer.WriteProperty("aggregate_index", aggregate_index);
	serializer.WriteProperty("groupings_index", groupings_index);
	serializer.WriteProperty("groups", groups);
	serializer.WriteProperty("grouping_sets", grouping_sets);
	serializer.WriteProperty("grouping_functions", grouping_functions);
}

unique_ptr<LogicalOperator> LogicalAggregate::FormatDeserialize(FormatDeserializer &deserializer) {
	auto expressions = deserializer.ReadProperty<vector<unique_ptr<Expression>>>("expressions");
	auto group_index = deserializer.ReadProperty<idx_t>("group_index");
	auto aggregate_index = deserializer.ReadProperty<idx_t>("aggregate_index");
	auto result = duckdb::unique_ptr<LogicalAggregate>(new LogicalAggregate(group_index, aggregate_index, std::move(expressions)));
	deserializer.ReadProperty("groupings_index", result->groupings_index);
	deserializer.ReadProperty("groups", result->groups);
	deserializer.ReadProperty("grouping_sets", result->grouping_sets);
	deserializer.ReadProperty("grouping_functions", result->grouping_functions);
	return std::move(result);
}

void LogicalCTERef::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("table_index", table_index);
	serializer.WriteProperty("cte_index", cte_index);
	serializer.WriteProperty("chunk_types", chunk_types);
	serializer.WriteProperty("bound_columns", bound_columns);
	serializer.WriteProperty("materialized_cte", materialized_cte);
}

unique_ptr<LogicalOperator> LogicalCTERef::FormatDeserialize(FormatDeserializer &deserializer) {
	auto table_index = deserializer.ReadProperty<idx_t>("table_index");
	auto cte_index = deserializer.ReadProperty<idx_t>("cte_index");
	auto chunk_types = deserializer.ReadProperty<vector<LogicalType>>("chunk_types");
	auto bound_columns = deserializer.ReadProperty<vector<string>>("bound_columns");
	auto materialized_cte = deserializer.ReadProperty<CTEMaterialize>("materialized_cte");
	auto result = duckdb::unique_ptr<LogicalCTERef>(new LogicalCTERef(table_index, cte_index, std::move(chunk_types), std::move(bound_columns), materialized_cte));
	return std::move(result);
}

void LogicalColumnDataGet::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("table_index", table_index);
	serializer.WriteProperty("chunk_types", chunk_types);
	serializer.WriteProperty("collection", *collection);
}

unique_ptr<LogicalOperator> LogicalColumnDataGet::FormatDeserialize(FormatDeserializer &deserializer) {
	auto table_index = deserializer.ReadProperty<idx_t>("table_index");
	auto chunk_types = deserializer.ReadProperty<vector<LogicalType>>("chunk_types");
	auto collection = deserializer.ReadProperty<unique_ptr<ColumnDataCollection>>("collection");
	auto result = duckdb::unique_ptr<LogicalColumnDataGet>(new LogicalColumnDataGet(table_index, std::move(chunk_types), std::move(collection)));
	return std::move(result);
}

void LogicalComparisonJoin::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("join_type", join_type);
	serializer.WriteProperty("mark_index", mark_index);
	serializer.WriteProperty("left_projection_map", left_projection_map);
	serializer.WriteProperty("right_projection_map", right_projection_map);
	serializer.WriteProperty("conditions", conditions);
	serializer.WriteProperty("mark_types", mark_types);
}

unique_ptr<LogicalOperator> LogicalComparisonJoin::FormatDeserialize(FormatDeserializer &deserializer) {
	auto join_type = deserializer.ReadProperty<JoinType>("join_type");
	auto result = duckdb::unique_ptr<LogicalComparisonJoin>(new LogicalComparisonJoin(join_type, deserializer.Get<LogicalOperatorType>()));
	deserializer.ReadProperty("mark_index", result->mark_index);
	deserializer.ReadProperty("left_projection_map", result->left_projection_map);
	deserializer.ReadProperty("right_projection_map", result->right_projection_map);
	deserializer.ReadProperty("conditions", result->conditions);
	deserializer.ReadProperty("mark_types", result->mark_types);
	return std::move(result);
}

void LogicalDelimGet::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("table_index", table_index);
	serializer.WriteProperty("chunk_types", chunk_types);
}

unique_ptr<LogicalOperator> LogicalDelimGet::FormatDeserialize(FormatDeserializer &deserializer) {
	auto table_index = deserializer.ReadProperty<idx_t>("table_index");
	auto chunk_types = deserializer.ReadProperty<vector<LogicalType>>("chunk_types");
	auto result = duckdb::unique_ptr<LogicalDelimGet>(new LogicalDelimGet(table_index, std::move(chunk_types)));
	return std::move(result);
}

void LogicalDelimJoin::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("join_type", join_type);
	serializer.WriteProperty("mark_index", mark_index);
	serializer.WriteProperty("left_projection_map", left_projection_map);
	serializer.WriteProperty("right_projection_map", right_projection_map);
	serializer.WriteProperty("conditions", conditions);
	serializer.WriteProperty("mark_types", mark_types);
	serializer.WriteProperty("duplicate_eliminated_columns", duplicate_eliminated_columns);
}

unique_ptr<LogicalOperator> LogicalDelimJoin::FormatDeserialize(FormatDeserializer &deserializer) {
	auto join_type = deserializer.ReadProperty<JoinType>("join_type");
	auto result = duckdb::unique_ptr<LogicalDelimJoin>(new LogicalDelimJoin(join_type));
	deserializer.ReadProperty("mark_index", result->mark_index);
	deserializer.ReadProperty("left_projection_map", result->left_projection_map);
	deserializer.ReadProperty("right_projection_map", result->right_projection_map);
	deserializer.ReadProperty("conditions", result->conditions);
	deserializer.ReadProperty("mark_types", result->mark_types);
	deserializer.ReadProperty("duplicate_eliminated_columns", result->duplicate_eliminated_columns);
	return std::move(result);
}

void LogicalDistinct::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("distinct_type", distinct_type);
	serializer.WriteProperty("distinct_targets", distinct_targets);
	serializer.WriteOptionalProperty("order_by", order_by);
}

unique_ptr<LogicalOperator> LogicalDistinct::FormatDeserialize(FormatDeserializer &deserializer) {
	auto distinct_type = deserializer.ReadProperty<DistinctType>("distinct_type");
	auto distinct_targets = deserializer.ReadProperty<vector<unique_ptr<Expression>>>("distinct_targets");
	auto result = duckdb::unique_ptr<LogicalDistinct>(new LogicalDistinct(std::move(distinct_targets), distinct_type));
	deserializer.ReadOptionalProperty("order_by", result->order_by);
	return std::move(result);
}

void LogicalDummyScan::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("table_index", table_index);
}

unique_ptr<LogicalOperator> LogicalDummyScan::FormatDeserialize(FormatDeserializer &deserializer) {
	auto table_index = deserializer.ReadProperty<idx_t>("table_index");
	auto result = duckdb::unique_ptr<LogicalDummyScan>(new LogicalDummyScan(table_index));
	return std::move(result);
}

void LogicalEmptyResult::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("return_types", return_types);
	serializer.WriteProperty("bindings", bindings);
}

unique_ptr<LogicalOperator> LogicalEmptyResult::FormatDeserialize(FormatDeserializer &deserializer) {
	auto result = duckdb::unique_ptr<LogicalEmptyResult>(new LogicalEmptyResult());
	deserializer.ReadProperty("return_types", result->return_types);
	deserializer.ReadProperty("bindings", result->bindings);
	return std::move(result);
}

void LogicalExpressionGet::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("table_index", table_index);
	serializer.WriteProperty("expr_types", expr_types);
	serializer.WriteProperty("expressions", expressions);
}

unique_ptr<LogicalOperator> LogicalExpressionGet::FormatDeserialize(FormatDeserializer &deserializer) {
	auto table_index = deserializer.ReadProperty<idx_t>("table_index");
	auto expr_types = deserializer.ReadProperty<vector<LogicalType>>("expr_types");
	auto expressions = deserializer.ReadProperty<vector<vector<unique_ptr<Expression>>>>("expressions");
	auto result = duckdb::unique_ptr<LogicalExpressionGet>(new LogicalExpressionGet(table_index, std::move(expr_types), std::move(expressions)));
	return std::move(result);
}

void LogicalFilter::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("expressions", expressions);
	serializer.WriteProperty("projection_map", projection_map);
}

unique_ptr<LogicalOperator> LogicalFilter::FormatDeserialize(FormatDeserializer &deserializer) {
	auto result = duckdb::unique_ptr<LogicalFilter>(new LogicalFilter());
	deserializer.ReadProperty("expressions", result->expressions);
	deserializer.ReadProperty("projection_map", result->projection_map);
	return std::move(result);
}

void LogicalLimit::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("limit_val", limit_val);
	serializer.WriteProperty("offset_val", offset_val);
	serializer.WriteOptionalProperty("limit", limit);
	serializer.WriteOptionalProperty("offset", offset);
}

unique_ptr<LogicalOperator> LogicalLimit::FormatDeserialize(FormatDeserializer &deserializer) {
	auto limit_val = deserializer.ReadProperty<int64_t>("limit_val");
	auto offset_val = deserializer.ReadProperty<int64_t>("offset_val");
	auto limit = deserializer.ReadOptionalProperty<unique_ptr<Expression>>("limit");
	auto offset = deserializer.ReadOptionalProperty<unique_ptr<Expression>>("offset");
	auto result = duckdb::unique_ptr<LogicalLimit>(new LogicalLimit(limit_val, offset_val, std::move(limit), std::move(offset)));
	return std::move(result);
}

void LogicalLimitPercent::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("limit_percent", limit_percent);
	serializer.WriteProperty("offset_val", offset_val);
	serializer.WriteOptionalProperty("limit", limit);
	serializer.WriteOptionalProperty("offset", offset);
}

unique_ptr<LogicalOperator> LogicalLimitPercent::FormatDeserialize(FormatDeserializer &deserializer) {
	auto limit_percent = deserializer.ReadProperty<double>("limit_percent");
	auto offset_val = deserializer.ReadProperty<int64_t>("offset_val");
	auto limit = deserializer.ReadOptionalProperty<unique_ptr<Expression>>("limit");
	auto offset = deserializer.ReadOptionalProperty<unique_ptr<Expression>>("offset");
	auto result = duckdb::unique_ptr<LogicalLimitPercent>(new LogicalLimitPercent(limit_percent, offset_val, std::move(limit), std::move(offset)));
	return std::move(result);
}

void LogicalOrder::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("orders", orders);
	serializer.WriteProperty("projections", projections);
}

unique_ptr<LogicalOperator> LogicalOrder::FormatDeserialize(FormatDeserializer &deserializer) {
	auto orders = deserializer.ReadProperty<vector<BoundOrderByNode>>("orders");
	auto result = duckdb::unique_ptr<LogicalOrder>(new LogicalOrder(std::move(orders)));
	deserializer.ReadProperty("projections", result->projections);
	return std::move(result);
}

void LogicalProjection::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("table_index", table_index);
	serializer.WriteProperty("expressions", expressions);
}

unique_ptr<LogicalOperator> LogicalProjection::FormatDeserialize(FormatDeserializer &deserializer) {
	auto table_index = deserializer.ReadProperty<idx_t>("table_index");
	auto expressions = deserializer.ReadProperty<vector<unique_ptr<Expression>>>("expressions");
	auto result = duckdb::unique_ptr<LogicalProjection>(new LogicalProjection(table_index, std::move(expressions)));
	return std::move(result);
}

void LogicalSample::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("sample_options", sample_options);
}

unique_ptr<LogicalOperator> LogicalSample::FormatDeserialize(FormatDeserializer &deserializer) {
	auto result = duckdb::unique_ptr<LogicalSample>(new LogicalSample());
	deserializer.ReadProperty("sample_options", result->sample_options);
	return std::move(result);
}

void LogicalTopN::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("orders", orders);
	serializer.WriteProperty("limit", limit);
	serializer.WriteProperty("offset", offset);
}

unique_ptr<LogicalOperator> LogicalTopN::FormatDeserialize(FormatDeserializer &deserializer) {
	auto orders = deserializer.ReadProperty<vector<BoundOrderByNode>>("orders");
	auto limit = deserializer.ReadProperty<idx_t>("limit");
	auto offset = deserializer.ReadProperty<idx_t>("offset");
	auto result = duckdb::unique_ptr<LogicalTopN>(new LogicalTopN(std::move(orders), limit, offset));
	return std::move(result);
}

void LogicalUnnest::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("unnest_index", unnest_index);
	serializer.WriteProperty("expressions", expressions);
}

unique_ptr<LogicalOperator> LogicalUnnest::FormatDeserialize(FormatDeserializer &deserializer) {
	auto unnest_index = deserializer.ReadProperty<idx_t>("unnest_index");
	auto result = duckdb::unique_ptr<LogicalUnnest>(new LogicalUnnest(unnest_index));
	deserializer.ReadProperty("expressions", result->expressions);
	return std::move(result);
}

void LogicalWindow::FormatSerialize(FormatSerializer &serializer) const {
	LogicalOperator::FormatSerialize(serializer);
	serializer.WriteProperty("window_index", window_index);
	serializer.WriteProperty("expressions", expressions);
}

unique_ptr<LogicalOperator> LogicalWindow::FormatDeserialize(FormatDeserializer &deserializer) {
	auto window_index = deserializer.ReadProperty<idx_t>("window_index");
	auto result = duckdb::unique_ptr<LogicalWindow>(new LogicalWindow(window_index));
	deserializer.ReadProperty("expressions", result->expressions);
	return std::move(result);
}

} // namespace duckdb
