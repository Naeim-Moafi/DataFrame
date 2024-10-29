cmake_minimum_required(VERSION 3.14) 
project(DataFrame)
# message(STATUS "The C++ compiler ID is: ${CMAKE_CXX_COMPILER_ID}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include(FetchContent)

# include GEM=================================================================
#add_subdirectory(GEM)

# CMake options=================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch fmt ======================================================
FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)
#=================================================================
file(GLOB SRC "src/*.cpp")

# Concatenate the two lists of source files
set(SOURCES ${SRC})

#=================================================================

#=================================================================
# Add the executable with the concatenated source files
add_executable(run_main ${SOURCES} )

 target_link_libraries(run_main
  PRIVATE
    fmt::fmt
    )
    
#=================================================================

# Set compiler optimization flags 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall") #-Wall 