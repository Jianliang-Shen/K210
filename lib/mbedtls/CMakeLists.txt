include_directories(${CMAKE_CURRENT_LIST_DIR}/mbedtls/include)

FILE(GLOB_RECURSE MBEDTLS_SRC
        "${CMAKE_CURRENT_LIST_DIR}/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/*.h"
)
