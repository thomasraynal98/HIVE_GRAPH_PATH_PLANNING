CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

# set the project name.
PROJECT(HIVE_GRAPH_PATH_PLANNING VERSION 1.0)

# include directories.
INCLUDE_DIRECTORIES(include)

# set good gcc version.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -L/usr/local/lib")

# found package.
FIND_PACKAGE(OpenCV 4.2 REQUIRED)
FIND_PACKAGE(OpenXLSX REQUIRED)

# add the executable exemple.
ADD_EXECUTABLE(GRAPH_ALGO 
    software/main.cpp
    src/useful.cpp
    include/useful.h)

TARGET_LINK_LIBRARIES(GRAPH_ALGO
    pthread
    OpenXLSX::OpenXLSX
    ${OpenCV_LIBS})