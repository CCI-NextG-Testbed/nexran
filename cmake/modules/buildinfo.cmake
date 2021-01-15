execute_process(
  COMMAND git log --pretty=format:'%h' -n 1
  OUTPUT_VARIABLE NEXRAN_GIT_COMMIT
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE)

if ("${NEXRAN_GIT_COMMIT}" STREQUAL "")
  set(NEXRAN_GIT_COMMIT "")
  set(NEXRAN_GIT_TREE_DIRTY "")
  set(NEXRAN_GIT_TAG "")
  set(NEXRAN_GIT_BRANCH "")
else()
  execute_process(
    COMMAND sh -c "git diff --quiet --exit-code || echo dirty"
    OUTPUT_VARIABLE NEXRAN_GIT_TREE_DIRTY
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)
  execute_process(
    COMMAND git describe --exact-match --tags
    OUTPUT_VARIABLE NEXRAN_GIT_TAG
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)
  execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE NEXRAN_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)
endif()



set(BUILDINFO "const char* NEXRAN_GIT_COMMIT=\"${NEXRAN_GIT_COMMIT}${NEXRAN_GIT_TREE_DIRTY}\";
const char* NEXRAN_GIT_TAG = \"${NEXRAN_GIT_TAG}\";
const char* NEXRAN_GIT_BRANCH = \"${NEXRAN_GIT_BRANCH}\";")

string(TIMESTAMP NEXRAN_BUILD_TIMESTAMP UTC)
set(BUILDINFO "${BUILDINFO}
const char* NEXRAN_BUILD_TIMESTAMP = \"${NEXRAN_BUILD_TIMESTAMP}\";")

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/src/buildinfo.cc)
  file(READ ${CMAKE_CURRENT_BINARY_DIR}/src/buildinfo.cc BUILDINFO_)
else()
  set(BUILDINFO_ "")
endif()

if (NOT "${BUILDINFO}" STREQUAL "${BUILDINFO_}")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/src/buildinfo.cc "${BUILDINFO}")
endif()

ADD_CUSTOM_COMMAND(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/buildinfo.cc
         ${CMAKE_CURRENT_BINARY_DIR}/_buildinfo.cc
  COMMAND ${CMAKE_COMMAND} -P
          ${CMAKE_CURRENT_SOURCE_DIR}/buildinfo.cmake)
