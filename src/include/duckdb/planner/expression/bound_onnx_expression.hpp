//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/planner/expression/bound_onnx_expression.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/planner/expression.hpp"
#include "onnx/onnx_pb.h"

namespace duckdb {
    
class BoundOnnxExpression : public Expression {
public:
	static constexpr const ExpressionClass TYPE = ExpressionClass::BOUND_ONNX;

public:
    BoundOnnxExpression(string path);
    
public:
    string ToString() const override;
    unique_ptr<Expression> Copy() override;

public:
    string path;
    unique_ptr<onnx::ModelProto> model;
};

} // namespace duckdb
