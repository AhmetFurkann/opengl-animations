add_library(shared STATIC GlfwWindowUtils.cpp)
target_include_directories(shared PUBLIC ${PROJECT_SOURCE_DIR}/include)