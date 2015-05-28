# - Try to find libdwarf
# Once done this will define
#
#  LIBDWARF_FOUND - system has libdwarf
#  LIBDWARF_INCLUDE_DIRS - the libdwarf include directory
#  LIBDWARF_LIBRARIES - Link these to use libdwarf
#  LIBDWARF_DEFINITIONS - Compiler switches required for using libdwarf
#

# Locate libelf library at first
if (NOT LIBELF_FOUND)
  find_package (LibElf)
endif (NOT LIBELF_FOUND)

if (LIBDWARF_LIBRARIES AND LIBDWARF_INCLUDE_DIRS)
  set (LibDwarf_FIND_QUIETLY TRUE)
endif (LIBDWARF_LIBRARIES AND LIBDWARF_INCLUDE_DIRS)

find_path (DWARF_INCLUDE_DIR
  NAMES
    libdwarf.h dwarf.h
  PATHS
    /usr/include
    /usr/include/libdwarf
    /usr/local/include
    /usr/local/include/libdwarf
    /opt/local/include
    /sw/include
    ENV CPATH) # PATH and INCLUDE will also work

if (DWARF_INCLUDE_DIR)
  set (LIBDWARF_INCLUDE_DIRS  ${DWARF_INCLUDE_DIR})
endif ()

find_library (LIBDWARF_LIBRARIES
  NAMES
    dwarf libdwarf
  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    ENV LIBRARY_PATH   # PATH and LIB will also work
    ENV LD_LIBRARY_PATH)
include (FindPackageHandleStandardArgs)


# handle the QUIETLY and REQUIRED arguments and set LIBDWARF_FOUND to TRUE
# if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibDwarf DEFAULT_MSG
  LIBELF_FOUND
  LIBDWARF_LIBRARIES
  LIBDWARF_INCLUDE_DIRS)

if (LIBDWARF_LIBRARIES AND LIBDWARF_INCLUDE_DIRS)
  set(CMAKE_REQUIRED_INCLUDES ${LIBDWARF_INCLUDE_DIRS})
  set(CMAKE_REQUIRED_LIBRARIES ${LIBDWARF_LIBRARIES} ${LIBELF_LIBRARIES})

  # libdwarf makes breaking changes occasionally and doesn't provide an easy
  # way to test for them. The following checks should detect the changes and
  # pass that information on accordingly.
  INCLUDE(CheckCXXSourceCompiles)
  INCLUDE(CheckFunctionExists)

  MACRO(CHECK_LIBDWARF_INIT init params var)
    # Check for the existence of this particular init function.
    unset(INIT_EXISTS CACHE)
    CHECK_FUNCTION_EXISTS(${init} INIT_EXISTS)
    if (INIT_EXISTS)
      set(LIBDWARF_USE_INIT_C ${var})

      # Check to see if we can use a const name.
      unset(DW_CONST CACHE)

      if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        # -std=c++11 is already set in HPHPCompiler.cmake, don't
        # add -std=c++0x on top of that or clang will give errors
        set(CMAKE_REQUIRED_FLAGS "-std=c++0x")
      endif()

      CHECK_CXX_SOURCE_COMPILES("
      #include <libdwarf.h>
      #include <cstddef>
      int dwarfCallback(const char * a, int b, Dwarf_Unsigned c,
        Dwarf_Unsigned d, Dwarf_Unsigned e, Dwarf_Unsigned f,
        Dwarf_Unsigned * g, Dwarf_Ptr h, int * i) {}
      int main() { ${init}(${params}); return 0; }" DW_CONST)
      if (DW_CONST)
        set(LIBDWARF_CONST_NAME 1)
      else()
        set(LIBDWARF_CONST_NAME 0)
      endif()
    endif()
  ENDMACRO(CHECK_LIBDWARF_INIT)

  # Order is important, last one is used.
  CHECK_LIBDWARF_INIT("dwarf_producer_init"  
	"0, dwarfCallback, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr" 0)
  CHECK_LIBDWARF_INIT("dwarf_producer_init_c" "0, dwarfCallback, nullptr, nullptr, nullptr, nullptr" 1)
endif()

if(LIBDWARF_CONST_NAME)
  message(STATUS "libdwarf uses const char* type")
else()
  message(STATUS "libdwarf uses char* type")
endif()
if(LIBDWARF_USE_INIT_C)
  message(STATUS "libdwarf has dwarf_producer_init_c")
else()
  message(STATUS "libdwarf does not have dwarf_producer_init_c, using dwarf_producer_init")
endif()

mark_as_advanced(LIBDW_INCLUDE_DIR DWARF_INCLUDE_DIR)
mark_as_advanced(LIBDWARF_INCLUDE_DIRS LIBDWARF_LIBRARIES)
mark_as_advanced(LIBDWARF_CONST_NAME LIBDWARF_USE_INIT_C)
