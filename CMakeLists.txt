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
)

target_link_libraries(PhysicsEngine PRIVATE raylib)