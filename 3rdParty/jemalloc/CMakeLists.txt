project(jemalloc)

set(jemalloc_version "4.4.0")

set(install_suffix "")
set(private_namespace "je_")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/${jemalloc_version}/include/jemalloc/internal/jemalloc_internal.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${jemalloc_version}/include/jemalloc/internal/jemalloc_internal.h"
  NEWLINE_STYLE UNIX
)

set(jemalloc_src "${CMAKE_CURRENT_SOURCE_DIR}/${jemalloc_version}")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${jemalloc_version}/include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/${jemalloc_version}/include")

add_library(jemalloc STATIC
  ${jemalloc_src}/src/arena.c
)
