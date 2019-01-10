//===----------------------------------------------------------------------===//
//                         DuckDB
//
// parser/expression/common_subexpression.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "parser/expression.hpp"
#include "parser/sql_node_visitor.hpp"

namespace duckdb {
//! Represents a CommonSubExpression, this is only generated by the optimizers. CSEs cannot be serialized, deserialized
//! or copied.
class CommonSubExpression : public Expression {
public:
	CommonSubExpression(unique_ptr<Expression> child, string alias)
	    : Expression(ExpressionType::COMMON_SUBEXPRESSION, child->return_type) {
		this->child = child.get();
		this->owned_child = move(child);
		this->alias = alias;
		return_type = this->child->return_type;
		assert(this->child);
	}
	CommonSubExpression(Expression *child, string alias)
	    : Expression(ExpressionType::COMMON_SUBEXPRESSION, child->return_type), child(child) {
		this->alias = alias;
		return_type = this->child->return_type;
		assert(child);
	}

	void ResolveType() override {
		child->ResolveType();
		return_type = child->return_type;
	}

	unique_ptr<Expression> Copy() override;

	size_t ChildCount() const override;
	Expression *GetChild(size_t index) const override;
	void ReplaceChild(std::function<unique_ptr<Expression>(unique_ptr<Expression> expression)> callback,
	                  size_t index) override;

	void Serialize(Serializer &serializer) override;

	bool Equals(const Expression *other) const override;

	string ToString() const override {
		return child->ToString();
	}

	unique_ptr<Expression> Accept(SQLNodeVisitor *v) override {
		return v->Visit(*this);
	}
	ExpressionClass GetExpressionClass() override {
		return ExpressionClass::COMMON_SUBEXPRESSION;
	}
	//! The child of the CSE
	Expression *child;
	//! The owned child of the CSE (if any)
	unique_ptr<Expression> owned_child;
};
} // namespace duckdb
