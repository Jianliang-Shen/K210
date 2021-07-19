FILE(GLOB_RECURSE MBEDTLS_SRC
        "${CMAKE_CURRENT_LIST_DIR}/include/mbedtls/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/include/mbedtls/*.h"
        "${CMAKE_CURRENT_LIST_DIR}/include/psa/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/include/psa/*.h"
        "${CMAKE_CURRENT_LIST_DIR}/library/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/library/*.h"
)

ADD_LIBRARY(mbedtls
        ${MBEDTLS_SRC}
)
TARGET_COMPILE_OPTIONS(mbedtls PRIVATE -O2)
