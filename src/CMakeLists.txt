project(encoder)
project(decoder)

find_package(OpenCV REQUIRED)
message(${OpenCV_INCLUDE_DIRS})

add_executable(encoder encoder.cpp)
add_executable(decoder decoder.cpp)

target_link_libraries(decoder ${OpenCV_LIBS})
include_directories(${OpenCV_INLCUDE_DIRS})

