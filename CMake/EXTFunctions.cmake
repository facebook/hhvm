# Functions/Macros for use when building extensions statically

# These functions also exist in CMake/HPHPIZEFunctions.cmake
# Their signatures should be kept consistent, though their behavior
# will differ slightly.

macro(HHVM_LINK_LIBRARIES EXTNAME)
  foreach (lib ${ARGN})
    list(APPEND HRE_LIBRARIES ${lib})
  endforeach()
endmacro()

function(HHVM_ADD_INCLUDES EXTNAME)
  include_directories(${ARGN})
endfunction()

macro(HHVM_EXTENSION EXTNAME)
  foreach (src ${ARGN})
    list(APPEND CXX_SOURCES "${HRE_CURRENT_EXT_PATH}/${src}")
  endforeach()
endmacro()

macro(HHVM_SYSTEMLIB EXTNAME)
  foreach (slib ${ARGN})
    list(APPEND PHP_SOURCES "${HRE_CURRENT_EXT_PATH}/${slib}")
  endforeach()
endmacro()

function(HHVM_DEFINE EXTNAME)
  add_definitions(${ARGN})
endfunction()
