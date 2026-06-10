cmake_minimum_required(VERSION 3.20)

project(GemsGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

set(SOURCES
        src/main.cpp
        src/Tech/Board.cpp
        src/Tech/Renderer.cpp
        src/Tech/Game.cpp
)

set(HEADERS
        src/Containers/Config.hpp
        src/Containers/Types.hpp
        src/Tech/Board.hpp
        src/Tech/Renderer.hpp
        src/Tech/Game.hpp
)

add_executable(GemsGame ${SOURCES} ${HEADERS})

target_include_directories(GemsGame PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Containers
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Tech
)

target_link_libraries(GemsGame PRIVATE
        glfw
        glad::glad
        glm::glm
)
