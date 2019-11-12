cmake_minimum_required(VERSION 3.15)
project(cgame_engine C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_executable(cgame_engine src/main.c src/external/glad.c src/external/glad.h src/external/khrplatform.h)

include_directories(${OPENGL_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS})
target_include_directories(cgame_engine PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(cgame_engine PUBLIC glfw ${OPENGL_LIBRARIES} ${OPENGL_gl_LIBRARY} ${CMAKE_DL_LIBS})
