# Functions for use when building extensions dynamically

# These functions also exist in CMake/EXTFunctions.cmake
# Their signatures should be kept consistent, though their behavior
# will differ slightly.

function(HHVM_LINK_LIBRARIES EXTNAME)
	list(REMOVE_AT ARGV 0)
	target_link_libraries(${EXTNAME} ${ARGV})
endfunction()

function(HHVM_ADD_INCLUDES EXTNAME)
	list(REMOVE_AT ARGV 0)
	include_directories(${ARGV})
endfunction()

function(HHVM_EXTENSION EXTNAME)
	list(REMOVE_AT ARGV 0)
	add_library(${EXTNAME} SHARED ${ARGV})
	set_target_properties(${EXTNAME} PROPERTIES PREFIX "")
	set_target_properties(${EXTNAME} PROPERTIES SUFFIX ".so")
endfunction()

function(HHVM_SYSTEMLIB EXTNAME)
	list(REMOVE_AT ARGV 0)
	foreach (SLIB ${ARGV})
		embed_systemlib_byname(${EXTNAME} ${SLIB})
	endforeach()
	embed_systemlibs(${EXTNAME} "${EXTNAME}.so")
endfunction()

