pybind11_add_module(tson tson.cc)
target_link_libraries(tson PRIVATE fmt nlohmann_json::nlohmann_json)