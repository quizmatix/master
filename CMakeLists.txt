cmake_minimum_required(VERSION 3.10)

project(master)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

include_directories(3rdparty)

file(GLOB SOURCES "src/*.cpp" "src/*.hpp" "src/classes/*.cpp" "src/classes/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${Boost_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${Boost_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)