cmake_minimum_required(VERSION 3.10)

# project statement
project(
    SMTCell
    VERSION 1.2
    LANGUAGES CXX)

# include external libraries
include(ExternalProject)

# Boost
set(BOOST_VERSION 1.83.0)
set(BOOST_VERSION_US 1_83_0)
set(BOOST_URL https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_US}.tar.bz2)
set(BOOST_INSTALL ${CMAKE_BINARY_DIR}/boost-prefix/src/boost)

ExternalProject_Add(
    boost
    URL ${BOOST_URL}
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ./bootstrap.sh --prefix=${BOOST_INSTALL}
    BUILD_COMMAND ./b2 --with-regex install link=static variant=release threading=multi runtime-link=static --prefix=${BOOST_INSTALL}
    INSTALL_COMMAND ""
    INSTALL_DIR ${BOOST_INSTALL} 
)

set(BOOST_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/boost-prefix/src/boost/include/)
set(BOOST_LIBRARY_DIRS ${CMAKE_BINARY_DIR}/boost-prefix/src/boost/lib/)

# fmt
ExternalProject_Add(
    fmt
    URL https://github.com/fmtlib/fmt/archive/refs/tags/10.1.1.tar.gz
    CMAKE_CACHE_ARGS
    "-DFMT_INSTALL:BOOL=ON"
    "-DFMT_DOC:BOOL=OFF"
    "-DFMT_TEST:BOOL=OFF"
    "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/fmt"
    BUILD_COMMAND make -j8
    INSTALL_COMMAND make install
)

set(FMT_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/fmt/include)
set(FMT_LIBRARY_DIRS ${CMAKE_BINARY_DIR}/fmt/lib64)

# nlohmann-json
ExternalProject_Add(
    nlohmann-json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.tar.gz
    CMAKE_CACHE_ARGS
    "-DFMT_INSTALL:BOOL=ON"
    "-DFMT_DOC:BOOL=OFF"
    "-DFMT_TEST:BOOL=OFF"
    "-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/nlohmann-json"
    BUILD_COMMAND make -j8
    INSTALL_COMMAND make install
)

set(NLOHMANN-JSON_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/nlohmann-json/include)

# Add the source files for each target
set(GENSMTINPUTAGR_SRC
    ./src/genSMTInputAGR.cpp
    ./src/obj.cpp
    ./src/graph.cpp
    ./src/dbConfig.cpp
    ./src/SMTCell.cpp
    ./src/flow.cpp
    ./src/design_rule.cpp
    ./src/placement.cpp
    ./src/layoutParser.cpp
)

set(GENTESTCASE_SRC
    ./src/genTestCase.cpp
    ./src/obj.cpp
)

set(CONVSMTRESULT_SRC
    ./src/convSMTResult.cpp
    ./src/obj.cpp
    ./src/graph.cpp
    ./src/SMTCell.cpp
)

# Set compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Add the targets
# Step 1: genTestCase
add_executable(genTestCase ${GENTESTCASE_SRC})
add_dependencies(genTestCase boost fmt)
target_include_directories(genTestCase PRIVATE ${BOOST_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS})
target_link_libraries(genTestCase PRIVATE ${BOOST_LIBRARY_DIRS}/libboost_regex.a ${FMT_LIBRARY_DIRS}/libfmt.a stdc++fs)

# Step 2: genSMTInputAGR
add_executable(genSMTInputAGR ${GENSMTINPUTAGR_SRC})
add_dependencies(genSMTInputAGR boost fmt nlohmann-json)
target_include_directories(genSMTInputAGR PRIVATE ${BOOST_INCLUDE_DIRS} ${NLOHMANN-JSON_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS})
target_link_libraries(genSMTInputAGR PRIVATE ${BOOST_LIBRARY_DIRS}/libboost_regex.a ${FMT_LIBRARY_DIRS}/libfmt.a stdc++fs)

# Step 3: convSMTResult
add_executable(convSMTResult ${CONVSMTRESULT_SRC})
add_dependencies(convSMTResult boost fmt nlohmann-json)
target_include_directories(convSMTResult PRIVATE ${BOOST_INCLUDE_DIRS} ${NLOHMANN-JSON_INCLUDE_DIRS} ${FMT_INCLUDE_DIRS})
target_link_libraries(convSMTResult PRIVATE ${BOOST_LIBRARY_DIRS}/libboost_regex.a ${FMT_LIBRARY_DIRS}/libfmt.a stdc++fs)

