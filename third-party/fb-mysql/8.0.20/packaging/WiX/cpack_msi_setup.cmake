# Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

SET(ENV{VS_UNICODE_OUTPUT})

INCLUDE(${CMAKE_BINARY_DIR}/CPackConfig.cmake)

IF(CPACK_WIX_CONFIG)
  INCLUDE(${CPACK_WIX_CONFIG})
ENDIF()

IF(NOT CPACK_WIX_UI)
  SET(CPACK_WIX_UI "WixUI_Mondo_Custom")
ENDIF()

SET(WIX_FEATURES)
FOREACH(comp ${CPACK_COMPONENTS_ALL})
 STRING(TOUPPER "${comp}" comp_upper)
 IF(NOT CPACK_COMPONENT_${comp_upper}_GROUP)
   SET(WIX_FEATURE_${comp_upper}_COMPONENTS "${comp}")
   SET(CPACK_COMPONENT_${comp_upper}_HIDDEN 1)
   SET(CPACK_COMPONENT_GROUP_${comp_upper}_DISPLAY_NAME ${CPACK_COMPONENT_${comp_upper}_DISPLAY_NAME})
   SET(CPACK_COMPONENT_GROUP_${comp_upper}_DESCRIPTION ${CPACK_COMPONENT_${comp_upper}_DESCRIPTION})
   SET(CPACK_COMPONENT_GROUP_${comp_upper}_WIX_LEVEL ${CPACK_COMPONENT_${comp_upper}_WIX_LEVEL})
   SET(WIX_FEATURES ${WIX_FEATURES} WIX_FEATURE_${comp_upper})
 ELSE()
   SET(FEATURE_NAME WIX_FEATURE_${CPACK_COMPONENT_${comp_upper}_GROUP})
   SET(WIX_FEATURES ${WIX_FEATURES} ${FEATURE_NAME})
   LIST(APPEND ${FEATURE_NAME}_COMPONENTS ${comp})
 ENDIF()
ENDFOREACH()

LIST(REMOVE_DUPLICATES WIX_FEATURES)

SET(CPACK_WIX_FEATURES)

FOREACH(f ${WIX_FEATURES})
 STRING(TOUPPER "${f}" f_upper)
 STRING(REPLACE "WIX_FEATURE_" "" f_upper ${f_upper})
 IF (CPACK_COMPONENT_GROUP_${f_upper}_DISPLAY_NAME)
  SET(TITLE ${CPACK_COMPONENT_GROUP_${f_upper}_DISPLAY_NAME})
 ELSE()
  SET(TITLE  CPACK_COMPONENT_GROUP_${f_upper}_DISPLAY_NAME)
 ENDIF()

 IF (CPACK_COMPONENT_GROUP_${f_upper}_DESCRIPTION)
  SET(DESCRIPTION ${CPACK_COMPONENT_GROUP_${f_upper}_DESCRIPTION})
 ELSE()
  SET(DESCRIPTION CPACK_COMPONENT_GROUP_${f_upper}_DESCRIPTION)
 ENDIF()
 IF(CPACK_COMPONENT_${f_upper}_WIX_LEVEL)
   SET(Level ${CPACK_COMPONENT_${f_upper}_WIX_LEVEL})
 ELSE()
   SET(Level 1)
 ENDIF()
 IF(CPACK_COMPONENT_GROUP_${f_upper}_HIDDEN)
   SET(DISPLAY "Display='hidden'")
   SET(TITLE ${f_upper})
   SET(DESCRIPTION ${f_upper})
 ELSE()
   SET(DISPLAY)
   IF(CPACK_COMPONENT_GROUP_${f_upper}_EXPANDED)
    SET(DISPLAY "Display='expand'")
   ENDIF()
   IF (CPACK_COMPONENT_GROUP_${f_upper}_DISPLAY_NAME)
    SET(TITLE ${CPACK_COMPONENT_GROUP_${f_upper}_DISPLAY_NAME})
   ELSE()
     SET(TITLE  CPACK_COMPONENT_GROUP_${f_upper}_DISPLAY_NAME)
   ENDIF()
   IF (CPACK_COMPONENT_GROUP_${f_upper}_DESCRIPTION)
     SET(DESCRIPTION ${CPACK_COMPONENT_GROUP_${f_upper}_DESCRIPTION})
   ELSE()
     SET(DESCRIPTION CPACK_COMPONENT_GROUP_${f_upper}_DESCRIPTION)
   ENDIF()
 ENDIF()
 
 SET(CPACK_WIX_FEATURES 
 "${CPACK_WIX_FEATURES}
   <Feature  Id='${f_upper}'
     Title='${TITLE}'
     Description='${DESCRIPTION}'
     ConfigurableDirectory='INSTALLDIR'
     Level='${Level}' ${DISPLAY} >"
  )
 FOREACH(c ${${f}_COMPONENTS})
   STRING(TOUPPER "${c}" c_upper)
   IF (CPACK_COMPONENT_${c_upper}_DISPLAY_NAME)
    SET(TITLE ${CPACK_COMPONENT_${c_upper}_DISPLAY_NAME})
   ELSE()
    SET(TITLE CPACK_COMPONENT_${c_upper}_DISPLAY_NAME)
   ENDIF()

   IF (CPACK_COMPONENT_${c_upper}_DESCRIPTION)
     SET(DESCRIPTION ${CPACK_COMPONENT_${c_upper}_DESCRIPTION})
   ELSE()
     SET(DESCRIPTION CPACK_COMPONENT_${c_upper}_DESCRIPTION)
   ENDIF()
   IF(CPACK_COMPONENT_${c_upper}_WIX_LEVEL)
    SET(Level ${CPACK_COMPONENT_${c_upper}_WIX_LEVEL})
   ELSE()
    SET(Level 1)
   ENDIF()
   IF(CPACK_COMPONENT_${c_upper}_HIDDEN)
   SET(CPACK_WIX_FEATURES
   "${CPACK_WIX_FEATURES} 
     <ComponentGroupRef Id='componentgroup.${c}'/>")
   ELSE()
   SET(CPACK_WIX_FEATURES
   "${CPACK_WIX_FEATURES} 
    <Feature Id='${c}' 
       Title='${TITLE}'
       Description='${DESCRIPTION}'
       ConfigurableDirectory='INSTALLDIR'
       Level='${Level}'>
       <ComponentGroupRef Id='componentgroup.${c}'/>
    </Feature>")
  ENDIF()
  
  ENDFOREACH()
   SET(CPACK_WIX_FEATURES
   "${CPACK_WIX_FEATURES}
   </Feature>
   ") 
ENDFOREACH()


IF(CMAKE_INSTALL_CONFIG_NAME)
  STRING(REPLACE "${CMAKE_CFG_INTDIR}" "${CMAKE_INSTALL_CONFIG_NAME}" 
    WIXCA_LOCATION "${WIXCA_LOCATION}")
  SET(CONFIG_PARAM "-DCMAKE_INSTALL_CONFIG_NAME=${CMAKE_INSTALL_CONFIG_NAME}")
ENDIF()

FILE(REMOVE_RECURSE testinstall) 

FOREACH(comp ${CPACK_COMPONENTS_ALL})
 SET(ENV{DESTDIR} testinstall/${comp})
 SET(DIRS ${DIRS} testinstall/${comp})
 EXECUTE_PROCESS(
  COMMAND ${CMAKE_COMMAND} ${CONFIG_PARAM} -DCMAKE_INSTALL_COMPONENT=${comp}  
   -DCMAKE_INSTALL_PREFIX=  -P ${CMAKE_BINARY_DIR}/cmake_install.cmake
   OUTPUT_QUIET
  )
ENDFOREACH()

MACRO(GENERATE_GUID VarName)
 EXECUTE_PROCESS(COMMAND uuidgen -c 
 OUTPUT_VARIABLE ${VarName}
 OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDMACRO()

# Make sure that WIX identifier created from a path matches all the rules:
# - it is shorter than 72 characters
# - doesn't contain reserver characters ('+', '-' and '/')
# ID_SET contains a global list of all identifiers which are too long.
# Every time we use an identifier which is too long we use its index in
# ID_SET to shorten the name.
SET_PROPERTY(GLOBAL PROPERTY ID_SET)
MACRO(MAKE_WIX_IDENTIFIER str varname)
  STRING(REPLACE "/" "." ${varname} "${str}")
  STRING(REPLACE "+" "p" ${varname} "${str}")
  STRING(REPLACE "-" "m" ${varname} "${str}")
  STRING(REGEX REPLACE "[^a-zA-Z_0-9.]" "_" ${varname} "${${varname}}")
  STRING(LENGTH "${${varname}}" len)
  # FIXME: the prefix length has to be controlled better
  # Identifier should be smaller than 72 character
  # We have to cut down the length to 40 chars, since we add prefixes
  # pretty often
  IF(len GREATER 40)   
    STRING(SUBSTRING  "${${varname}}" 0 37 shortstr)
    GET_PROPERTY(LOCAL_LIST GLOBAL PROPERTY ID_SET)
    LIST(FIND LOCAL_LIST "${${varname}}" STRING_ID)
    IF(${STRING_ID} EQUAL -1)
      LIST(APPEND LOCAL_LIST "${${varname}}")
      SET_PROPERTY(GLOBAL PROPERTY ID_SET "${LOCAL_LIST}")
      LIST(LENGTH LOCAL_LIST STRING_ID)
      MATH(EXPR STRING_ID "${STRING_ID}-1" )
    ENDIF()
    SET(${varname} "${shortstr}${STRING_ID}")
  ENDIF()
ENDMACRO()

FUNCTION(TRAVERSE_FILES dir topdir file file_comp  dir_root)
  FILE(RELATIVE_PATH dir_rel ${topdir} ${dir})
  IF(dir_rel)
    LIST(FIND EXCLUDE_DIRS ${dir_rel} TO_EXCLUDE)
    IF(NOT TO_EXCLUDE EQUAL -1)
      MESSAGE(STATUS "excluding directory: ${dir_rel}")
      RETURN()
    ENDIF()
  ENDIF()
  FILE(GLOB all_files ${dir}/*)
  IF(NOT all_files)
    RETURN()
  ENDIF()
  IF(dir_rel)
   MAKE_DIRECTORY(${dir_root}/${dir_rel})
   MAKE_WIX_IDENTIFIER("${dir_rel}" id)
   SET(DirectoryRefId  "D.${id}")
  ELSE()
   SET(DirectoryRefId "INSTALLDIR")
  ENDIF()
  FILE(APPEND ${file} "<DirectoryRef Id='${DirectoryRefId}'>\n")
 
  SET(NONEXEFILES)
  FOREACH(f ${all_files})
    IF(NOT IS_DIRECTORY ${f})
      FILE(RELATIVE_PATH rel ${topdir} ${f})
      SET(TO_EXCLUDE)
      IF(rel MATCHES "\\.pdb$")
        SET(TO_EXCLUDE TRUE)
      ELSE()
        LIST(FIND EXCLUDE_FILES ${rel} RES)
        IF(NOT RES EQUAL -1)
          SET(TO_EXCLUDE TRUE)
        ENDIF()
      ENDIF()
      IF(TO_EXCLUDE)
        MESSAGE(STATUS "excluding file: ${rel}")
      ELSE()
	MAKE_WIX_IDENTIFIER("${rel}" id)
	FILE(TO_NATIVE_PATH ${f} f_native)
	GET_FILENAME_COMPONENT(f_ext "${f}" EXT)
	# According to MSDN each DLL or EXE should be in the own component
	IF(f_ext MATCHES ".exe" OR f_ext MATCHES ".dll")

	  FILE(APPEND ${file} "  <Component Id='C.${id}' Guid='*' Win64='yes'>\n")
	  FILE(APPEND ${file} "    <File Id='F.${id}' KeyPath='yes' Source='${f_native}'/>\n")
	  FILE(APPEND ${file} "  </Component>\n")
	  FILE(APPEND ${file_comp} "  <ComponentRef Id='C.${id}'/>\n")
       ELSE()
	SET(NONEXEFILES  "${NONEXEFILES}\n<File Id='F.${id}' Source='${f_native}'/>" )
	ENDIF()
      ENDIF()
    ENDIF()
  ENDFOREACH()
  FILE(APPEND ${file} "</DirectoryRef>\n")
  IF(NONEXEFILES)
    GENERATE_GUID(guid)
    SET(ComponentId "C._files_${COMP_NAME}.${DirectoryRefId}")
    FILE(APPEND ${file} 
    "<DirectoryRef Id='${DirectoryRefId}'>\n<Component Guid='${guid}' Id='${ComponentId}' Win64='yes'>${NONEXEFILES}\n</Component></DirectoryRef>\n")
	FILE(APPEND ${file_comp} "  <ComponentRef Id='${ComponentId}'/>\n")
  ENDIF()
  FOREACH(f ${all_files})
    IF(IS_DIRECTORY ${f})
      TRAVERSE_FILES(${f} ${topdir} ${file} ${file_comp}  ${dir_root})
    ENDIF()
  ENDFOREACH()
ENDFUNCTION()

FUNCTION(TRAVERSE_DIRECTORIES dir topdir file prefix)
  FILE(RELATIVE_PATH rel ${topdir} ${dir})
  IF(rel)
    MAKE_WIX_IDENTIFIER("${rel}" id)
    GET_FILENAME_COMPONENT(name ${dir} NAME)
    FILE(APPEND ${file} "${prefix}<Directory Id='D.${id}' Name='${name}'>\n")
  ENDIF()
  FILE(GLOB all_files ${dir}/*)
  FOREACH(f ${all_files})
    IF(IS_DIRECTORY ${f})
      TRAVERSE_DIRECTORIES(${f} ${topdir} ${file} "${prefix}  ")
    ENDIF()
  ENDFOREACH()
  IF(rel)
    FILE(APPEND ${file} "${prefix}</Directory>\n")
  ENDIF()
ENDFUNCTION()

SET(CPACK_WIX_COMPONENTS)
SET(CPACK_WIX_COMPONENT_GROUPS)
GET_FILENAME_COMPONENT(abs . ABSOLUTE)
FOREACH(d ${DIRS})
  GET_FILENAME_COMPONENT(d ${d} ABSOLUTE)
  GET_FILENAME_COMPONENT(d_name ${d} NAME)
  FILE(WRITE ${abs}/${d_name}_component_group.wxs  "<ComponentGroup Id='componentgroup.${d_name}'>")
  SET(COMP_NAME ${d_name})
  TRAVERSE_FILES(${d} ${d} ${abs}/${d_name}.wxs ${abs}/${d_name}_component_group.wxs "${abs}/dirs")
  FILE(APPEND  ${abs}/${d_name}_component_group.wxs   "</ComponentGroup>")
  IF(EXISTS ${d_name}.wxs)
    FILE(READ ${d_name}.wxs WIX_TMP)
    SET(CPACK_WIX_COMPONENTS "${CPACK_WIX_COMPONENTS}\n${WIX_TMP}")
    FILE(REMOVE ${d_name}.wxs)
  ENDIF()
  
  FILE(READ ${d_name}_component_group.wxs WIX_TMP)
 
  SET(CPACK_WIX_COMPONENT_GROUPS "${CPACK_WIX_COMPONENT_GROUPS}\n${WIX_TMP}")
  FILE(REMOVE ${d_name}_component_group.wxs)
ENDFOREACH()

FILE(WRITE directories.wxs "<DirectoryRef Id='INSTALLDIR'>\n")
TRAVERSE_DIRECTORIES(${abs}/dirs ${abs}/dirs directories.wxs "")
FILE(APPEND directories.wxs "</DirectoryRef>\n")

FILE(READ directories.wxs CPACK_WIX_DIRECTORIES)
FILE(REMOVE directories.wxs)


FOREACH(src ${CPACK_WIX_INCLUDE})
SET(CPACK_WIX_INCLUDES 
"${CPACK_WIX_INCLUDES}
 <?include ${src}?>"
)
ENDFOREACH()
