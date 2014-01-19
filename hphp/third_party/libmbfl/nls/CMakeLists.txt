SET(_mbfl_nls_files nls_ja.c nls_de.c nls_en.c nls_hy.c nls_tr.c nls_kr.c
	nls_ru.c nls_ua.c nls_zh.c nls_uni.c nls_neutral.c)
	
foreach(mbfl_nls_file ${_mbfl_nls_files})
	list(APPEND mbfl_nls_files "${CMAKE_CURRENT_SOURCE_DIR}/${mbfl_nls_file}")
endforeach()

SET(mbfl_nls_files ${mbfl_nls_files} PARENT_SCOPE)