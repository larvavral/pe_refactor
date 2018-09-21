cmake_minimum_required(VERSION 3.10.2)

project(price_engine)

set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11")

ADD_DEFINITIONS(/D_DEBUG)

include_directories(include)
include_directories(src)

link_directories(lib)
#set (PROJECT_LINK_LIBS event hiredis event_core boost_program_options pthread )
# Use libhiredis.
#set(PROJECT_LINK_LIBS libevent.a libhiredis.a)
set(PROJECT_LINK_LIBS event libhiredis.a libglog.a)

file(GLOB CPP_SOURCES "src/*.cc")
file(GLOB COMMON_SOURCES "src/common/*.cc")
#add_executable(PE ${CPP_SOURCES} ${COMMON_SOURCES} ${REDIS_SOURCES})
add_executable(PE ${CPP_SOURCES} ${COMMON_SOURCES})

target_link_libraries(PE ${PROJECT_LINK_LIBS})
