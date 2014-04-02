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

function(HHVM_SYSTEMLIB EXTNAME SOURCE_FILE)
	# Ignore it, embed_all_systemlibs will pick this up
	# TODO: Make this cleaner so that we don't embed systemlibs
	# which aren't going to be used
endfunction()

