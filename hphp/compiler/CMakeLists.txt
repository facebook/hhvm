set(SOURCE_SUBDIRS expression statement analysis system util parser)
set(files)
set(CXX_SOURCES)
set(C_SOURCES)

auto_sources(files "*.cpp" "${SOURCE_SUBDIRS}") 
list(APPEND CXX_SOURCES ${files})

auto_sources(files "*.c" "${SOURCE_SUBDIRS}")
list(APPEND C_SOURCES ${files})	


#FLEX_TARGET(HphpAnalysisScanner ${CMAKE_CURRENT_SOURCE_DIR}/parser/hphp.x ${CMAKE_CURRENT_SOURCE_DIR}/parser/lex.yy.cpp COMPILE_FLAGS " -w -i")
#BISON_TARGET(HphpAnalysisParser ${CMAKE_CURRENT_SOURCE_DIR}/parser/hphp.y ${CMAKE_CURRENT_SOURCE_DIR}/parser/hphp.tab.cpp  COMPILE_FLAGS " -d")
#ADD_FLEX_BISON_DEPENDENCY(HphpAnalysisScanner HphpAnalysisParser)

#list(APPEND CXX_SOURCES ${FLEX_HphpAnalysisScanner_OUTPUTS} ${BISON_HphpAnalysisParser_OUTPUTS} ${CMAKE_CURRENT_SOURCE_DIR}/parser/scanner.cpp ${CMAKE_CURRENT_SOURCE_DIR}/parser/parser.cpp)


set(ERROR_INC "analysis/core_code_error.inc")

set(ERROR_INC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/analysis/code_error.inc")

file(REMOVE "${ERROR_INC_FILE}")

file(WRITE "${ERROR_INC_FILE}" "/* This file is auto generated */")

foreach (error_file ${ERROR_INC})
	file(READ ${error_file} file_contents)

	file(APPEND "${ERROR_INC_FILE}" "${file_contents}")
endforeach (error_file ${ERROR_INC})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../")

set(CMAKE_CURRENT_BINARY_DIR "${CMAKE_SOURCE_DIR}/bin")

foreach (CXX_FILE ${CXX_SOURCES})
	if(${CXX_FILE} MATCHES ".no.cpp$")
		SET_SOURCE_FILES_PROPERTIES(
			${CXX_FILE}
			PROPERTIES
			COMPILE_FLAGS -O0
		)
	endif()
endforeach()

add_definitions(-DALWAYS_ASSERT=1)

ADD_LIBRARY(hphp_analysis STATIC ${CXX_SOURCES} ${C_SOURCES})
target_link_libraries(hphp_analysis ${Boost_LIBRARIES})
add_dependencies(hphp_analysis hphp_parser)
