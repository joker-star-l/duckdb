#include "duckdb/planner/expression_binder/having_binder.hpp"

#include "duckdb/parser/expression/columnref_expression.hpp"
#include "duckdb/planner/binder.hpp"
#include "duckdb/planner/expression_binder/aggregate_binder.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/planner/query_node/bound_select_node.hpp"

namespace duckdb {

HavingBinder::HavingBinder(Binder &binder, ClientContext &context, BoundSelectNode &node, BoundGroupInformation &info,
                           case_insensitive_map_t<idx_t> &alias_map, AggregateHandling aggregate_handling)
    : BaseSelectBinder(binder, context, node, info), column_alias_binder(node, alias_map),
      aggregate_handling(aggregate_handling) {
	target_type = LogicalType(LogicalTypeId::BOOLEAN);
}

BindResult HavingBinder::BindColumnRef(unique_ptr<ParsedExpression> &expr_ptr, idx_t depth, bool root_expression) {

	// Keep the original column name to return a meaningful error message.
	auto column_name = expr_ptr->Cast<ColumnRefExpression>().GetColumnName();

	// Bind the alias.
	BindResult alias_result;
	auto found_alias = column_alias_binder.BindAlias(*this, expr_ptr, depth, root_expression, alias_result);

	if (found_alias) {
		if (depth > 0) {
			throw BinderException("Having clause cannot reference alias \"%s\" in correlated subquery", column_name);
		}
		return alias_result;
	}

	if (aggregate_handling == AggregateHandling::FORCE_AGGREGATES) {
		if (depth > 0) {
			throw BinderException(
			    "Having clause cannot reference column \"%s\" in correlated subquery and group by all", column_name);
		}

		auto expr = duckdb::BaseSelectBinder::BindExpression(expr_ptr, depth);
		if (expr.HasError()) {
			return expr;
		}

		// Return a GROUP BY column reference expression.
		auto return_type = expr.expression->return_type;
		auto column_binding = ColumnBinding(node.group_index, node.groups.group_expressions.size());
		auto group_ref = make_uniq<BoundColumnRefExpression>(return_type, column_binding);
		node.groups.group_expressions.push_back(std::move(expr.expression));
		return BindResult(std::move(group_ref));
	}

	return BindResult(StringUtil::Format(
	    "column %s must appear in the GROUP BY clause or be used in an aggregate function", column_name));
}

BindResult HavingBinder::BindExpression(unique_ptr<ParsedExpression> &expr_ptr, idx_t depth, bool root_expression) {
	auto &expr = *expr_ptr;
	// check if the expression binds to one of the groups
	auto group_index = TryBindGroup(expr);
	if (group_index != DConstants::INVALID_INDEX) {
		return BindGroup(expr, depth, group_index);
	}
	switch (expr.expression_class) {
	case ExpressionClass::WINDOW:
		return BindResult("HAVING clause cannot contain window functions!");
	case ExpressionClass::COLUMN_REF:
		return BindColumnRef(expr_ptr, depth, root_expression);
	default:
		return duckdb::BaseSelectBinder::BindExpression(expr_ptr, depth);
	}
}

} // namespace duckdb
