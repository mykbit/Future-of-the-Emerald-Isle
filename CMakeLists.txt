cmake_minimum_required(VERSION 3.0)
project(emerald_isle)

find_package(OpenGL REQUIRED)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_subdirectory(external)

include_directories(
	external/glfw-3.1.2/include/
	external/glm-0.9.7.1/
	external/glad-opengl-3.3/include/
	external/glew-2.1.0/include/
	external/tinygltf-2.9.3/
	src/
)

add_executable(emerald_isle
	src/main.cpp
	src/static_model.cpp
	src/skybox.cpp
	src/surface.cpp
	src/building.cpp
)
target_link_libraries(emerald_isle
	${OPENGL_LIBRARY}
	glfw
	glad
	glew
)