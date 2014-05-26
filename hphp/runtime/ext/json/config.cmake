# The strange set-up here is because Debian doesn't like the license on the the
# bundled parser. The built in one is still preferred though, so only use json-c
# if the USE_JSONC option is set

option(USE_JSONC "Use json-c parser instead of the bundled parser" OFF)

if (USE_JSONC)
    find_package(Libjsonc REQUIRED)

    if (LIBJSONC_LIBRARY)
        message(STATUS "Using libjson-c as JSON parser")
        HHVM_EXTENSION(json ext_json.cpp jsonc_parser.cpp)
        HHVM_ADD_INCLUDES(json ${LIBJSONC_INCLUDE_DIR})
        HHVM_LINK_LIBRARIES(json ${LIBJSONC_LIBRARY})
    else()
        message(FATAL_ERROR "Cannot find libjson-c")
    endif()
else()
    message(STATUS "Using built-in JSON parser")
    HHVM_EXTENSION(json ext_json.cpp JSON_parser.cpp)
endif()

HHVM_SYSTEMLIB(json ext_json.php)
