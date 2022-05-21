cmake_minimum_required(VERSION 3.15)
project(cgame_engine)

set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_executable(
  cgame_engine
  src/main.cpp
  src/external/glad.c
  src/external/glad.h
  src/external/khrplatform.h
  src/cod3rGL.h
  src/interactions.cpp
  src/interactions.h
)

include_directories(src/external/include)

include_directories(${OPENGL_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS})

target_include_directories(cgame_engine PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(cgame_engine PUBLIC glfw ${OPENGL_LIBRARIES} ${OPENGL_gl_LIBRARY} ${CMAKE_DL_LIBS} -lm -lstdc++)
