cmake_minimum_required(VERSION 3.22)

project(noise_reduction)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)

add_executable(noise_reduction main.cpp noise_reduction.hpp noise_reduction.cpp)

include_directories(${PROJECT_NAME} PUBLIC
${OPENCV_INCLUDE_DIRS}
${PROJECT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} 
PRIVATE 
    opencv_core
    opencv_videoio
    opencv_highgui
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/known_capture_resolutions.csv
            ${CMAKE_CURRENT_BINARY_DIR}/known_capture_resolutions.csv)