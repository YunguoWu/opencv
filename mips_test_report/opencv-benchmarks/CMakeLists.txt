cmake_minimum_required(VERSION 2.8.3)

project(opencv-benchmarks)

# Save the command line compile commands in the build output
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
# View the makefile commands during build
#set(CMAKE_VERBOSE_MAKEFILE on)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(contours2 src/contours2.cpp)
target_link_libraries(contours2 ${DEPENDENCIES} ${OpenCV_LIBS})

add_executable(discrete_fourier_transform src/discrete_fourier_transform.cpp)
target_link_libraries(discrete_fourier_transform ${DEPENDENCIES} ${OpenCV_LIBS})

add_executable(edge src/edge.cpp)
target_link_libraries(edge ${DEPENDENCIES} ${OpenCV_LIBS})

add_executable(morphology_2 src/morphology_2.cpp)
target_link_libraries(morphology_2 ${DEPENDENCIES} ${OpenCV_LIBS})

add_executable(smoothing src/smoothing.cpp)
target_link_libraries(smoothing ${DEPENDENCIES} ${OpenCV_LIBS})

set(EXECUTABLE_OUTPUT_PATH "bin/")
