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
   find_package (LibElf REQUIRED)
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
    /opt/local/include
    /sw/include
    ENV CPATH) # PATH and INCLUDE will also work

if (DWARF_INCLUDE_DIR)
  set (LIBDWARF_INCLUDE_DIRS  ${DWARF_INCLUDE_DIR})
endif ()

find_library (LIBDWARF_LIBRARIES
  NAMES
    dwarf
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
  LIBDWARF_LIBRARIES
  LIBDWARF_INCLUDE_DIRS)

if (LIBDWARF_LIBRARIES AND LIBDWARF_INCLUDE_DIRS)
  set(CMAKE_REQUIRED_INCLUDES ${LIBDWARF_INCLUDE_DIRS})
  set(CMAKE_REQUIRED_LIBRARIES ${LIBDWARF_LIBRARIES} ${LIBELF_LIBRARIES})
  
  # libdwarf makes breaking changes occasionally and doesn't provide an easy
  # way to test for them. The following checks should detect the changes and
  # pass that information on accordingly.
  INCLUDE(CheckCSourceCompiles)
  CHECK_C_SOURCE_COMPILES("#include <libdwarf.h>
  int dwarfCallback(const char * a, int b, Dwarf_Unsigned c,
  Dwarf_Unsigned d, Dwarf_Unsigned e, Dwarf_Unsigned f,
  Dwarf_Unsigned * g, Dwarf_Ptr h, int * i) {}
int main() {
  dwarf_producer_init_c(0, dwarfCallback, 0, 0, 0, 0);
  return 0;
}" DW_INIT_C)

  if (NOT DW_INIT_C)
	CHECK_C_SOURCE_COMPILES("#include <libdwarf.h>
  int dwarfCallback(const char * a, int b, Dwarf_Unsigned c,
  Dwarf_Unsigned d, Dwarf_Unsigned e, Dwarf_Unsigned f,
  Dwarf_Unsigned * g, Dwarf_Ptr h, int * i) {}
  int main() {
	dwarf_producer_init(0, dwarfCallback, 0, 0, 0, 0, 0, 0, 0, 0);
    return 0;
  }" DW_INIT)
	if (DW_INIT)
	  set(LIBDWARF_CONST_NAME 1)
	  set(LIBDWARF_USE_INIT_C 0)
	else()
	  set(LIBDWARF_CONST_NAME 0)
	  set(LIBDWARF_USE_INIT_C 1)
	endif()
  else()
	set(LIBDWARF_CONST_NAME 1)
	set(LIBDWARF_USE_INIT_C 1)
  endif()
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
