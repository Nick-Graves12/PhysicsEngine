cmake_minimum_required(VERSION 3.20)

project(PhysicsEngine)

set(CMAKE_CXX_STANDARD 17)

find_package(raylib REQUIRED)

add_executable(PhysicsEngine src/main.cpp
                src/main.cpp
                src/PhysicsWorld.cpp
                src/Physics.cpp
                src/PhysicsWorld.cpp
                src/Collision.cpp
                src/OrbitalWorld.cpp
                src/FluidWorld.cpp
                src/UI.cpp
                src/WindWorld.cpp
)

target_link_libraries(PhysicsEngine PRIVATE raylib)

add_custom_command(TARGET PhysicsEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:PhysicsEngine>/resources
)