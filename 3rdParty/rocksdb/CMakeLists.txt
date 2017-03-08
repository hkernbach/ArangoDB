# -*- mode: CMAKE; -*-

project(rocksdb C CXX)

set(SKIP_INSTALL_ALL ON)

if (USE_JEMALLOC)
  set(USE_JEMALLOC_DEFAULT 1)
  set(JEMALLOC_INCLUDE     ${JEMALLOC_HOME}/include)
  set(JEMALLOC_LIB_DEBUG   ${JEMALLOC_HOME}/lib/libjemalloc.a)
  set(JEMALLOC_LIB_RELEASE ${JEMALLOC_HOME}/lib/libjemalloc.a)
endif ()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/rocksdb)
