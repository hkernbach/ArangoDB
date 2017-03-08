# -*- mode: CMAKE; -*-

project(rocksdb C CXX)

set(SKIP_INSTALL_ALL ON)

if (USE_JEMALLOC)
  set(USE_JEMALLOC_DEFAULT 1)
  set(JEMALLOC_INCLUDE     ${JEMALLOC_HOME}/include)
  set(JEMALLOC_LIB_DEBUG   ${JEMALLOC_HOME}/lib/libjemalloc.a)
  set(JEMALLOC_LIB_RELEASE ${JEMALLOC_HOME}/lib/libjemalloc.a)
endif ()

set(USE_SNAPPY_DEFAULT 1)

set(SNAPPY_HOME ${CMAKE_SOURCE_DIR}/3rdParty/snappy/google-snappy-d53de18/)
set(SNAPPY_INCLUDE ${SNAPPY_HOME})
list(APPEND SNAPPY_INCLUDE ${CMAKE_BINARY_DIR}/3rdParty/snappy/google-snappy-d53de18/)
set(SNAPPY_LIB_DEBUG snappystatic)
set(SNAPPY_LIB_RELEASE snappystatic)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rocksdb)
