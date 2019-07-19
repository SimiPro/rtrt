cmake_minimum_required (VERSION 3.2)
project (SummerBreeze_RT)

add_subdirectory(ext/glfw)
include_directories(ext/glm)

find_package(Vulkan REQUIRED)
message("Found vulkan: ${Vulkan_INCLUDE_DIRS}, ${Vulkan_LIBRARY}")

add_executable(app main.cc)

target_include_directories(app PUBLIC ${Vulkan_INCLUDE_DIRS})
target_include_directories(app PUBLIC include)


target_link_libraries(app glfw ${Vulkan_LIBRARY})

message("compiling shaders...")
execute_process (
    COMMAND bash -c "glslangValidator -V ${CMAKE_CURRENT_SOURCE_DIR}/shaders/helloTriangle.vert"
)

execute_process (
    COMMAND bash -c "glslangValidator -V ${CMAKE_CURRENT_SOURCE_DIR}/shaders/helloTriangle.frag"
)
