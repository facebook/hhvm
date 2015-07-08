include(GNUInstallDirs)

function(auto_sources RETURN_VALUE PATTERN SOURCE_SUBDIRS)

  if ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")
    SET(PATH ".")
    if (${ARGC} EQUAL 4)
      list(GET ARGV 3 PATH)
    endif ()
  endif()

  if ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")
    unset(${RETURN_VALUE})
    file(GLOB SUBDIR_FILES "${PATH}/${PATTERN}")
    list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})

    file(GLOB subdirs RELATIVE ${PATH} ${PATH}/*)

    foreach(DIR ${subdirs})
      if (IS_DIRECTORY ${PATH}/${DIR})
        if (NOT "${DIR}" STREQUAL "CMakeFiles")
          file(GLOB_RECURSE SUBDIR_FILES "${PATH}/${DIR}/${PATTERN}")
          list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})
        endif()
      endif()
    endforeach()
  else ()
    file(GLOB ${RETURN_VALUE} "${PATTERN}")

    foreach (PATH ${SOURCE_SUBDIRS})
      file(GLOB SUBDIR_FILES "${PATH}/${PATTERN}")
      list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})
    endforeach(PATH ${SOURCE_SUBDIRS})
  endif ()

  if (${FILTER_OUT})
    list(REMOVE_ITEM ${RETURN_VALUE} ${FILTER_OUT})
  endif()

  set(${RETURN_VALUE} ${${RETURN_VALUE}} PARENT_SCOPE)
endfunction(auto_sources)

macro(HHVM_SELECT_SOURCES DIR)
  auto_sources(files "*.cpp" "RECURSE" "${DIR}")
  foreach(f ${files})
    if (NOT (${f} MATCHES "(ext_hhvm|/(old-)?tests?/)"))
      list(APPEND CXX_SOURCES ${f})
    endif()
  endforeach()
  auto_sources(files "*.c" "RECURSE" "${DIR}")
  foreach(f ${files})
    if (NOT (${f} MATCHES "(ext_hhvm|/(old-)?tests?/)"))
      list(APPEND C_SOURCES ${f})
    endif()
  endforeach()
  auto_sources(files "*.S" "RECURSE" "${DIR}")
  foreach(f ${files})
    if (NOT (${f} MATCHES "(ext_hhvm|/(old-)?tests?/)"))
      list(APPEND ASM_SOURCES ${f})
    endif()
  endforeach()
  auto_sources(files "*.h" "RECURSE" "${DIR}")
  foreach(f ${files})
    if (NOT (${f} MATCHES "(/(old-)?tests?/)"))
      list(APPEND HEADER_SOURCES ${f})
    endif()
  endforeach()
endmacro(HHVM_SELECT_SOURCES)

function(CONTAINS_STRING FILE SEARCH RETURN_VALUE)
  file(STRINGS ${FILE} FILE_CONTENTS REGEX ".*${SEARCH}.*")
  if (FILE_CONTENTS)
    set(${RETURN_VALUE} True PARENT_SCOPE)
  endif()
endfunction(CONTAINS_STRING)

macro(MYSQL_SOCKET_SEARCH)
  execute_process(COMMAND mysql_config --socket OUTPUT_VARIABLE MYSQL_SOCK OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
  if (NOT MYSQL_SOCK)
    foreach (i
      /var/run/mysqld/mysqld.sock
      /var/tmp/mysql.sock
      /var/run/mysql/mysql.sock
      /var/lib/mysql/mysql.sock
      /var/mysql/mysql.sock
      /usr/local/mysql/var/mysql.sock
      /Private/tmp/mysql.sock
      /private/tmp/mysql.sock
      /tmp/mysql.sock
      )
      if (EXISTS ${i})
        set(MYSQL_SOCK ${i})
        break()
      endif()
    endforeach()
  endif()
  if (MYSQL_SOCK)
    set(MYSQL_UNIX_SOCK_ADDR ${MYSQL_SOCK} CACHE STRING "Path to MySQL Socket")
  endif()
endmacro()

function(append_systemlib TARGET SOURCE SECTNAME)
  if(CYGWIN OR MSVC OR MINGW)
    # for each library append the following line to embed.rc
    # $sectionname RCDATA "$source"
    add_custom_command(TARGET generate_rc
      COMMAND echo "${SECTNAME} RCDATA \"${SOURCE}\"" >> embed.rc
      COMMENT "Adding ${SOURCE} as ${SECTNAME} to embed.rc"
      VERBATIM)
  else()
    if (APPLE)
      set(${TARGET}_SLIBS ${${TARGET}_SLIBS} -Wl,-sectcreate,__text,${SECTNAME},${SOURCE} PARENT_SCOPE)
    else()
      set(${TARGET}_SLIBS ${${TARGET}_SLIBS} "--add-section" "${SECTNAME}=${SOURCE}" PARENT_SCOPE)
    endif()
    # Add the systemlib file to the "LINK_DEPENDS" for the systemlib, this will cause it
    # to be relinked and the systemlib re-embedded
    set_property(TARGET ${TARGET} APPEND PROPERTY LINK_DEPENDS ${SOURCE})
  endif()
endfunction(append_systemlib)

function(embed_systemlibs TARGET DEST)
  if (APPLE)
    target_link_libraries(${TARGET} ${${TARGET}_SLIBS})
  elseif(CYGWIN OR MSVC OR MINGW)
    # nothing to do
  else()
    add_custom_command(TARGET ${TARGET} POST_BUILD
      COMMAND "objcopy"
      ARGS ${${TARGET}_SLIBS} ${DEST}
      COMMENT "Embedding php in ${TARGET}")
  endif()
endfunction(embed_systemlibs)

macro(embed_systemlib_byname TARGET SLIB)
  get_filename_component(SLIB_BN ${SLIB} "NAME_WE")
  string(LENGTH ${SLIB_BN} SLIB_BN_LEN)
  math(EXPR SLIB_BN_REL_LEN "${SLIB_BN_LEN} - 4")
  string(SUBSTRING ${SLIB_BN} 4 ${SLIB_BN_REL_LEN} SLIB_EXTNAME)
  string(MD5 SLIB_HASH_NAME ${SLIB_EXTNAME})
  # Some platforms limit section names to 16 characters :(
  string(SUBSTRING ${SLIB_HASH_NAME} 0 12 SLIB_HASH_NAME_SHORT)
  append_systemlib(${TARGET} ${SLIB} "ext.${SLIB_HASH_NAME_SHORT}")
endmacro()

function(embed_all_systemlibs TARGET ROOT DEST)
  append_systemlib(${TARGET} ${ROOT}/system/systemlib.php systemlib)
  auto_sources(SYSTEMLIBS "ext_*.php" "RECURSE" "${HPHP_HOME}/hphp/runtime")
  foreach(SLIB ${SYSTEMLIBS})
    embed_systemlib_byname(${TARGET} ${SLIB})
  endforeach()
  embed_systemlibs(${TARGET} ${DEST})
endfunction(embed_all_systemlibs)

# Custom install function that doesn't relink, instead it uses chrpath to change it, if
# it's available, otherwise, it leaves the chrpath alone
function(HHVM_INSTALL TARGET DEST)
  get_target_property(LOC ${TARGET} LOCATION)
  get_target_property(TY ${TARGET} TYPE)
  if (FOUND_CHRPATH)
    get_target_property(RPATH ${TARGET} INSTALL_RPATH)
    if (NOT RPATH STREQUAL "RPATH-NOTFOUND")
      if (RPATH STREQUAL "")
        install(CODE "execute_process(COMMAND \"${CHRPATH}\" \"-d\" \"${LOC}\" ERROR_QUIET)")
      else()
        install(CODE "execute_process(COMMAND \"${CHRPATH}\" \"-r\" \"${RPATH}\" \"${LOC}\" ERROR_QUIET)")
      endif()
    endif()
  endif()
  string(TOUPPER ${DEST} DEST_UPPER)
  install(CODE "FILE(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_${DEST_UPPER}DIR}\" TYPE ${TY} FILES \"${LOC}\")")
endfunction(HHVM_INSTALL)

function(HHVM_PUBLIC_HEADERS TARGET)
  install(
    CODE "INCLUDE(\"${HPHP_HOME}/CMake/HPHPFunctions.cmake\")
      HHVM_INSTALL_HEADERS(${TARGET} ${HPHP_HOME}
      \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}\" ${ARGN})"
    COMPONENT dev)
endfunction()

function(HHVM_INSTALL_HEADERS TARGET SRCROOT DEST)
  message(STATUS "Installing header files for ${TARGET}")
  foreach(src_rel ${ARGN})
    # Determine the relative directory name so that we can mirror the
    # directory structure in the output
    file(RELATIVE_PATH dest_rel ${SRCROOT} ${src_rel})
    if (IS_ABSOLUTE ${dest_rel})
      message(WARNING "${TARGET}: Header file ${dest_rel} is not inside ${SRCROOT}")
    else()
      string(FIND ${dest_rel} / slash_pos REVERSE)
      if(slash_pos EQUAL -1)
        set(dest_rel)
      else()
        string(SUBSTRING ${dest_rel} 0 ${slash_pos} dest_rel)
      endif()
      file(COPY ${src_rel}
        DESTINATION "$ENV{DESTDIR}${DEST}/${dest_rel}"
        NO_SOURCE_PERMISSIONS)
    endif()
  endforeach()
endfunction()

macro(HHVM_EXT_OPTION EXTNAME PACKAGENAME)
  if (NOT DEFINED EXT_${EXTNAME})
    # Implicit check
    find_package(${PACKAGENAME})
  elseif (EXT_${EXTNAME} STREQUAL "ON")
    # Explicit check
    find_package(${PACKAGENAME} REQUIRED)
  endif()
endmacro()

# Remove all files matching a set of patterns, and,
# optionally, not matching a second set of patterns,
# from a set of lists.
#
# Example:
# This will remove all files in the CPP_SOURCES list
# matching "/test/" or "Test.cpp$", but not matching
# "BobTest.cpp$".
# HHVM_REMOVE_MATCHES_FROM_LISTS(CPP_SOURCES MATCHES "/test/" "Test.cpp$" IGNORE_MATCHES "BobTest.cpp$")
# 
# Parameters:
# 
# [...]:
# The names of the lists to remove matches from.
#
# [MATCHES ...]:
# The matches to remove from the lists.
#
# [IGNORE_MATCHES ...]:
# The matches not to remove, even if they match
# the main set of matches to remove.
function(HHVM_REMOVE_MATCHES_FROM_LISTS)
  set(LISTS_TO_SEARCH)
  set(MATCHES_TO_REMOVE)
  set(MATCHES_TO_IGNORE)
  set(argumentState 0)
  foreach (arg ${ARGN})
    if ("x${arg}" STREQUAL "xMATCHES")
      set(argumentState 1)
    elseif ("x${arg}" STREQUAL "xIGNORE_MATCHES")
      set(argumentState 2)
    elseif (argumentState EQUAL 0)
      list(APPEND LISTS_TO_SEARCH ${arg})
    elseif (argumentState EQUAL 1)
      list(APPEND MATCHES_TO_REMOVE ${arg})
    elseif (argumentState EQUAL 2)
      list(APPEND MATCHES_TO_IGNORE ${arg})
    else()
      message(FATAL_ERROR "Unknown argument state!")
    endif()
  endforeach()
  
  foreach (theList ${LISTS_TO_SEARCH})
    foreach (entry ${${theList}})
      foreach (match ${MATCHES_TO_REMOVE})
        if (${entry} MATCHES ${match})
          set(SHOULD_IGNORE OFF)
          foreach (ign ${MATCHES_TO_IGNORE})
            if (${entry} MATCHES ${ign})
              set(SHOULD_IGNORE ON)
              break()
            endif()
          endforeach()
          
          if (NOT SHOULD_IGNORE)
            list(REMOVE_ITEM ${theList} ${entry})
          endif()
        endif()
      endforeach()
    endforeach()
    set(${theList} ${${theList}} PARENT_SCOPE)
  endforeach()
endfunction()
