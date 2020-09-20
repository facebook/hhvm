include(FindPythonInterp)

if(NOT PYTHONINTERP_FOUND)
  message(FATAL_ERROR "Python is required for building XED")
endif()

set(XED_SRC_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/xed")
set(MBUILD_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/mbuild")
set(XED_BUILD_DIR  "${XED_SRC_DIR}/build")
set(XED_STATIC_LIB  "${XED_BUILD_DIR}/lib/libxed.a")
set(XED_INCLUDE_DIR "${XED_BUILD_DIR}/include/xed")
set(XED_INTERFACE_H "${XED_INCLUDE_DIR}/xed-interface.h")

# Generate XED header and C files, this is run at configure time
execute_process(COMMAND ${PYTHON_EXECUTABLE} mfile.py just-gen WORKING_DIRECTORY ${XED_SRC_DIR})

file(GLOB XED_PUBLIC_HEADERS ${XED_SRC_DIR}/include/public/xed/*.h)
file(GLOB XED_PRIVATE_HEADERS ${XED_SRC_DIR}/obj/*.h)
file(COPY ${XED_PUBLIC_HEADERS} ${XED_PRIVATE_HEADERS}  DESTINATION ${XED_INCLUDE_DIR})

# Build XED, this is run at build time
add_custom_command(OUTPUT ${XED_STATIC_LIB} ${XED_INTERFACE_H}
                   COMMAND ${PYTHON_EXECUTABLE} mfile.py install --install-dir ${XED_BUILD_DIR}
                   WORKING_DIRECTORY ${XED_SRC_DIR})

add_custom_target(xed_target DEPENDS ${XED_STATIC_LIB})
add_library(xed STATIC IMPORTED GLOBAL)
add_dependencies(xed xed_target)
set_target_properties(xed PROPERTIES IMPORTED_LOCATION ${XED_STATIC_LIB})
