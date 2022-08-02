cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME HelloSim)
set(CMAKE_CXX_STANDARD 17)

set(WINDOW_NAME "HelloSim")
set(WINDOW_WIDTH 960)
set(WINDOW_HEIGHT 540)

project(${PROJECT_NAME})
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/GlfwOcctWindow.cpp  src/GlfwOcctWindow.h
    src/GlfwOcctView.cpp  src/GlfwOcctView.h
)

include(Dependency.cmake)

# 우리 프로젝트에 include / lib 관련 옵션 추가
target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS})

# define 변수
target_compile_definitions(${PROJECT_NAME} PUBLIC
    WINDOW_NAME="${WINDOW_NAME}"
    WINDOW_WIDTH=${WINDOW_WIDTH}
    WINDOW_HEIGHT=${WINDOW_HEIGHT}
)

# Dependency 들이 먼저 build 될 수 있게 관계 설정
add_dependencies(${PROJECT_NAME} ${DEP_LIST})