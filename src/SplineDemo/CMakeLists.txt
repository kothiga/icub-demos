
SET(KEYWORD "spline")
PROJECT(${KEYWORD})
cmake_minimum_required(VERSION 2.6)

# Search for source code.
FILE(GLOB folder_source *.cpp *.cc *.c)

SOURCE_GROUP("Source Files" FILES ${folder_source})
SOURCE_GROUP("Header Files" FILES ${folder_header})

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/include
)

# Set up the main executable.
ADD_EXECUTABLE(${KEYWORD}
    ${folder_source}
    ${folder_header}
)

TARGET_LINK_LIBRARIES(${KEYWORD})
