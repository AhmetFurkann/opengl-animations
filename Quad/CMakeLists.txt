set(CMAKE_CXX_FLAGS "-fPIC")

add_executable(Quad
    quad.cpp
    ${PROJECT_SOURCE_DIR}/include/glad.c
    ${PROJECT_SOURCE_DIR}/misc/textRenderer.cpp
    ${PROJECT_SOURCE_DIR}/misc/utils.cpp
)

target_compile_definitions(Quad PRIVATE RESOURCE_PATH="${CMAKE_SOURCE_DIR}/resources/fonts")

target_include_directories(Quad PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Quad PRIVATE ${PROJECT_SOURCE_DIR}/misc)
target_include_directories(Quad PRIVATE ${PROJECT_SOURCE_DIR}/shared)

set(LIBS glfw freetype GL EGL pthread dl)
target_link_directories(Quad PUBLIC ${PROJECT_SOURCE_DIR}/libs)

target_link_libraries(Quad shared)
target_link_libraries(Quad ${LIBS})

set_target_properties(Quad PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/Quad/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/Quad/Release
)