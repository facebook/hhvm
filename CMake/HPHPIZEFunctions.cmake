# Functions for use when building extensions dynamically

# These functions also exist in CMake/EXTFunctions.cmake
# Their signatures should be kept consistent, though their behavior
# will differ slightly.

function(HHVM_LINK_LIBRARIES EXTNAME)
	target_link_libraries(${EXTNAME} ${ARGN})
endfunction()

function(HHVM_ADD_INCLUDES EXTNAME)
	include_directories(${ARGN})
endfunction()

function(HHVM_EXTENSION EXTNAME)
	add_library(${EXTNAME} SHARED ${ARGN})
	set_target_properties(${EXTNAME} PROPERTIES PREFIX "")
	set_target_properties(${EXTNAME} PROPERTIES SUFFIX ".so")
endfunction()

function(HHVM_SYSTEMLIB EXTNAME)
	foreach (SLIB ${ARGN})
		embed_systemlib_byname(${EXTNAME} ${SLIB})
	endforeach()
	embed_systemlibs(${EXTNAME} "${EXTNAME}.so")
endfunction()

function(HHVM_DEFINE EXTNAME)
	add_definitions(${ARGN})
endfunction()
