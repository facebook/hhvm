set(unicode_files 8859-1.TXT 8859-2.TXT 8859-3.TXT 8859-4.TXT 8859-5.TXT 8859-6.TXT
	8859-7.TXT 8859-8.TXT 8859-9.TXT 8859-10.TXT 8859-11.TXT
	8859-13.TXT 8859-14.TXT 8859-15.TXT 8859-16.TXT)

FIND_PROGRAM(AWK_EXECUTABLE NAMES gawk awk)

foreach(unicode_file ${unicode_files})
	string(REGEX REPLACE "([0-9]+)-([0-9]+).TXT" "iso\\1_\\2"
	         variable_name "${unicode_file}")
	string(TOUPPER "${variable_name}" variable_name_upper)

	if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${unicode_file}")
		message(STATUS "Downloading ${unicode_file}")
		file(DOWNLOAD "ftp://ftp.unicode.org/Public/MAPPINGS/ISO8859/${unicode_file}" "${CMAKE_CURRENT_SOURCE_DIR}/${unicode_file}")
	endif()

	set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${unicode_file}")
	set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/unicode_table_${variable_name}.h")

	execute_process(
						COMMAND ${AWK_EXECUTABLE} -v TABLE_NAME=${variable_name}_ucs_table -v IFNDEF_NAME=UNICODE_TABLE_${variable_name_upper}_H -f mk_sb_tbl.awk ${unicode_file}
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
						OUTPUT_FILE unicode_table_${variable_name}.h
					)
endforeach()

SET(_mbfl_filters_files mbfilter_cp936.c mbfilter_hz.c mbfilter_euc_tw.c mbfilter_big5.c
mbfilter_euc_jp.c mbfilter_jis.c mbfilter_iso8859_1.c mbfilter_iso8859_2.c
mbfilter_cp1254.c mbfilter_cp1252.c mbfilter_cp1251.c mbfilter_ascii.c
mbfilter_iso8859_3.c mbfilter_iso8859_4.c mbfilter_iso8859_5.c mbfilter_iso8859_6.c
mbfilter_iso8859_7.c mbfilter_iso8859_8.c mbfilter_iso8859_9.c mbfilter_iso8859_10.c
mbfilter_iso8859_13.c mbfilter_iso8859_14.c mbfilter_iso8859_15.c mbfilter_iso8859_16.c
mbfilter_htmlent.c mbfilter_byte2.c mbfilter_byte4.c mbfilter_uuencode.c
mbfilter_base64.c mbfilter_sjis.c mbfilter_7bit.c mbfilter_qprint.c mbfilter_ucs4.c
mbfilter_ucs2.c mbfilter_utf32.c mbfilter_utf16.c mbfilter_utf8.c mbfilter_utf7.c
mbfilter_utf7imap.c mbfilter_euc_jp_win.c mbfilter_cp932.c mbfilter_cp51932.c
mbfilter_euc_cn.c mbfilter_euc_kr.c mbfilter_uhc.c mbfilter_iso2022_jp_ms.c
mbfilter_iso2022_kr.c mbfilter_cp866.c mbfilter_koi8r.c mbfilter_koi8u.c
mbfilter_armscii8.c mbfilter_cp850.c html_entities.c)

foreach(mbfl_filter_file ${_mbfl_filters_files})
	list(APPEND mbfl_filters_files "${CMAKE_CURRENT_SOURCE_DIR}/${mbfl_filter_file}")
endforeach()

SET(mbfl_filters_files ${mbfl_filters_files} PARENT_SCOPE)
