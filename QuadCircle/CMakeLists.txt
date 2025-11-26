set(CMAKE_CXX_FLAGS "-fPIC")

add_executable(QuadCircle
    quadCircle.cpp
    ${PROJECT_SOURCE_DIR}/include/glad.c
    ${PROJECT_SOURCE_DIR}/misc/textRenderer.cpp
    ${PROJECT_SOURCE_DIR}/misc/utils.cpp
)

target_compile_definitions(QuadCircle PRIVATE RESOURCE_PATH="${CMAKE_SOURCE_DIR}/resources/fonts")

target_include_directories(QuadCircle PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(QuadCircle PRIVATE ${PROJECT_SOURCE_DIR}/misc)
target_include_directories(QuadCircle PRIVATE ${PROJECT_SOURCE_DIR}/shared)

set(LIBS glfw freetype GL EGL pthread dl)
target_link_directories(QuadCircle PUBLIC ${PROJECT_SOURCE_DIR}/libs)

target_link_libraries(QuadCircle shared)
target_link_libraries(QuadCircle ${LIBS})

set_target_properties(QuadCircle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/QuadCircle/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/QuadCircle/Release
)