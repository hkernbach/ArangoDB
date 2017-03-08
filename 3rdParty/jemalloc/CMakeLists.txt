# -*- mode: CMAKE; -*-

project(jemalloc C)

set(JEMALLOC_VERSION "4.5.0")
set(JEMALLOC_VERSION "${JEMALLOC_VERSION}" PARENT_SCOPE)

file(
  COPY 
    "${CMAKE_CURRENT_SOURCE_DIR}/${JEMALLOC_VERSION}"
  DESTINATION
    "${CMAKE_CURRENT_BINARY_DIR}"
)

include(ExternalProject)
ExternalProject_Add(
  jemalloc
  SOURCE_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/${JEMALLOC_VERSION}
  PATCH_COMMAND
    cmake -E touch
      ${CMAKE_CURRENT_BINARY_DIR}/${JEMALLOC_VERSION}/doc/jemalloc.html
      ${CMAKE_CURRENT_BINARY_DIR}/${JEMALLOC_VERSION}/doc/jemalloc.3
  CONFIGURE_COMMAND
    ./autogen.sh --prefix=${CMAKE_CURRENT_BINARY_DIR} --disable-munmap --with-version=${JEMALLOC_VERSION}-0-g0
  PREFIX
    ${CMAKE_CURRENT_BINARY_DIR}/${JEMALLOC_VERSION}
  BUILD_COMMAND
    make
  BUILD_IN_SOURCE
    1
)
