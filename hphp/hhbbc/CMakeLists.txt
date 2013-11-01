set(CXX_SOURCES)
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

# remove anything in a test folder
foreach (file ${CXX_SOURCES})
	if (${file} MATCHES "/test/")
		list(REMOVE_ITEM CXX_SOURCES ${file})
	endif()
endforeach(file ${CXX_SOURCES})

add_library(hphp_hhbbc STATIC ${CXX_SOURCES})
add_dependencies(hphp_hhbbc hphp_system)
