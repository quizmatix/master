cmake_minimum_required(VERSION 3.10)

project(CppWebSocketServer)

set(CMAKE_CXX_STANDARD 17)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system filesystem thread)

include_directories(3rdparty)

file(GLOB SOURCES "src/*.cpp" "src/*.hpp" "src/classes/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Link Boost libraries
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})