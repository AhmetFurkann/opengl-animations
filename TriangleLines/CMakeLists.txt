set(CMAKE_CXX_FLAGS "-fPIC")

add_executable(TriangleLines
    triangleLines.cpp
    ${PROJECT_SOURCE_DIR}/include/glad.c
    ${PROJECT_SOURCE_DIR}/misc/textRenderer.cpp
    ${PROJECT_SOURCE_DIR}/misc/utils.cpp
)

target_compile_definitions(TriangleLines PRIVATE RESOURCE_PATH="${CMAKE_SOURCE_DIR}/resources/fonts")

target_include_directories(TriangleLines PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(TriangleLines PRIVATE ${PROJECT_SOURCE_DIR}/misc)
target_include_directories(TriangleLines PRIVATE ${PROJECT_SOURCE_DIR}/shared)

set(LIBS glfw freetype GL EGL pthread dl)
target_link_directories(TriangleLines PUBLIC ${PROJECT_SOURCE_DIR}/libs)

target_link_libraries(TriangleLines shared)
target_link_libraries(TriangleLines ${LIBS})

set_target_properties(TriangleLines PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/TriangleLines/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/TriangleLines/Release
)