cmake_minimum_required(VERSION 3.15)

# set project name and language
project(ecs CXX)

# set c++ standard to 20
set(CMAKE_CXX_STANDARD 20)

# enable symbol re-exports
set(CMAKE_ENABLE_EXPORTS ON)

set(LIB_TARGET ${PROJECT_NAME}_lib)
set(BIN_TARGET ${PROJECT_NAME}_bin)
set(TEST_TARGET ${PROJECT_NAME}_test)

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(TEST_DIR ${PROJECT_SOURCE_DIR}/tests)

file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.cc)
file(GLOB_RECURSE HEADER_FILES ${SOURCE_DIR}/*.h)
file(GLOB_RECURSE TEST_SOURCE_FILES ${TEST_DIR}/*.cc)

# add lib target
add_library(${LIB_TARGET} STATIC ${SOURCE_FILES} ${HEADER_FILES})

# add include directories to the lib target
target_include_directories(${LIB_TARGET} PUBLIC ${SOURCE_DIR})

# add executable target
add_executable(${BIN_TARGET} ${SOURCE_FILES})


# add test target
add_executable(${TEST_TARGET} ${TEST_SOURCE_FILES})

# dependencies
find_package(SFML REQUIRED)
target_link_libraries(${LIB_TARGET} PUBLIC sfml-graphics sfml-window sfml-system sfml-audio sfml-network)

find_package(fmt REQUIRED)
target_link_libraries(${LIB_TARGET} PUBLIC fmt::fmt)

find_package(spdlog REQUIRED)
target_link_libraries(${LIB_TARGET} PUBLIC spdlog::spdlog)
# end dependencies

# this links all library dependencies to the binary target
target_link_libraries(${BIN_TARGET} PRIVATE ${LIB_TARGET})

# this links all library dependencies to the test target
target_link_libraries(${TEST_TARGET} PRIVATE ${LIB_TARGET})

# test dependencies
find_package(GTest REQUIRED)
target_link_libraries(${TEST_TARGET} PRIVATE GTest::gtest GTest::gtest_main)
# end test dependencies

# this enables one to install the binary target locally
install(TARGETS ${BIN_TARGET} DESTINATION "."
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
)