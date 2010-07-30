
FIND_PROGRAM(AWK_EXECUTABLE NAMES gawk awk)
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/EastAsianWidth.txt")
	message(STATUS "Downloading EastAsianWidth.txt")
	file(DOWNLOAD "http://ftp.unicode.org/Public/UNIDATA/EastAsianWidth.txt" "${CMAKE_CURRENT_SOURCE_DIR}/EastAsianWidth.txt")

  execute_process(
	  COMMAND ${AWK_EXECUTABLE} -v TABLE_NAME=mbfl_eaw_table -f mk_eaw_tbl.awk EastAsianWidth.txt
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	  OUTPUT_FILE eaw_table.h
  )
endif()

SET(mbfl_files mbfilter.c mbfl_string.c mbfl_language.c mbfl_encoding.c mbfl_convert.c
mbfl_ident.c mbfl_memory_device.c mbfl_allocators.c mbfl_filter_output.c
mbfilter_pass.c mbfilter_wchar.c mbfilter_8bit.c)

add_library(mbfl STATIC ${mbfl_files} ${mbfl_filters_files} ${mbfl_nls_files})
