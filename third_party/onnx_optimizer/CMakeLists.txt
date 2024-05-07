if(NOT MSVC)
 set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
 # fix DEBUG macro bug
 string(REPLACE "-DDEBUG" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
 string(REPLACE "-pedantic" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

add_subdirectory(onnx_optimizer)

disable_target_warnings(onnx_optimizer)
disable_target_warnings(onnx_optimizer_c_api)
disable_target_warnings(onnx)
disable_target_warnings(onnx_proto)
