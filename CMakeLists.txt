cmake_minimum_required(VERSION 3.13)
project(cpparse)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE sources "src/*.cpp")

add_library(cpparse ${sources})

option(DEV_SANDBOX "" ON)
option(OVERLOAD_OSTREAMS "provide overloads for std::ostream" ON)

if(DEV_SANDBOX)
	add_subdirectory(sandbox/)
endif()

target_compile_options(cpparse PUBLIC -Wall)
target_include_directories(cpparse PUBLIC
	"include/"
	"src/"
)
target_link_libraries(cpparse
  
)

if(OVERLOAD_OSTREAMS)
	target_compile_definitions(cpparse PUBLIC OVERLOAD_OSTREAMS)
endif()
