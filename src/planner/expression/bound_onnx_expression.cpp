#include "duckdb/planner/expression/bound_onnx_expression.hpp"
#include "onnx/common/file_utils.h"

namespace duckdb {

BoundOnnxExpression::BoundOnnxExpression(string path) 
    : Expression(
        ExpressionType::BOUND_ONNX, 
        ExpressionClass::BOUND_ONNX,
        LogicalType()
    ), 
    path(path) 
{   
    this->model = make_uniq<onnx::ModelProto>();
    ONNX_NAMESPACE::LoadProtoFromPath<onnx::ModelProto>(path, *this->model.get());
    // TODO 解析输入类型和输出类型
}

string BoundOnnxExpression::ToString() const {
	// TODO
    return "";
}

unique_ptr<Expression> BoundOnnxExpression::Copy() {
    // TODO
    return nullptr;
}

} // namespace duckdb
