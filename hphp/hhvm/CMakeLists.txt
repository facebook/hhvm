include(HHVMExtensionConfig)

set(CXX_SOURCES)
auto_sources(files "*.cpp" "")
list(APPEND CXX_SOURCES ${files})

# Windows targets use a generated rc file for embedding libraries
if(MSVC)
  list(APPEND CXX_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/embed.rc)
endif()

option(ENABLE_LD_GOLD "Enable Hot Linker script using ld-gold" On)
set(SECTION_ORDERING_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../tools/oss_hot_section_ordering CACHE FILEPATH "File used by ld-gold for the relocation of sections")

if (MAP_TEXT_HUGE_PAGES)
    file(STRINGS "${SECTION_ORDERING_FILE}" SECTION_ORDERING_CONTENTS)
    list(GET SECTION_ORDERING_CONTENTS 0 HOT_START)
    list(GET SECTION_ORDERING_CONTENTS -1 HOT_END)
    string(REGEX REPLACE "\\.text\\.(.*)\\*" "\\1" HOT_START ${HOT_START})
    string(REGEX REPLACE "\\.text\\.(.*)\\*" "\\1" HOT_END ${HOT_END})
    set(DEFSYM_HOT_START "--defsym=__hot_start=${HOT_START},")
    set(DEFSYM_HOT_END "--defsym=__hot_end=${HOT_END},")
endif()

if (ENABLE_LD_GOLD)
  find_package(Gold)
endif()

get_object_libraries_objects(additionalObjects ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
add_executable(hhvm ${CXX_SOURCES} ${additionalObjects})
target_link_libraries(hhvm ${HHVM_LINK_LIBRARIES} ${EZC_LINK_LIBRARIES} ${HRE_LINK_LIBRARIES})
link_object_libraries(hhvm ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
if (GOLD_FOUND AND ENABLE_LD_GOLD)
  if (IS_AARCH64)
    set(EXEC_STACK "-Wl,-z,noexecstack,--warn-execstack")
  else()
    set(EXEC_STACK "-Wl,-z,execstack")
  endif()
  if (CMAKE_CONFIGURATION_TYPES)
    SET(LINKER_SCRIPT -fuse-ld=gold;-Wl,--icf=all;${EXEC_STACK};-Wl,--gc-sections;release;-Wl,${DEFSYM_HOT_START}${DEFSYM_HOT_END}--section-ordering-file,${SECTION_ORDERING_FILE};default)
  elseif(NOT CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(LINKER_SCRIPT -fuse-ld=gold -Wl,${DEFSYM_HOT_START}${DEFSYM_HOT_END}--section-ordering-file,${SECTION_ORDERING_FILE},--icf=all;${EXEC_STACK};-Wl,--gc-sections)
  else()
    SET(LINKER_SCRIPT -fuse-ld=gold -Wl,--icf=all;${EXEC_STACK};-Wl,--gc-sections)
  endif()
  target_link_libraries(hhvm ${LINKER_SCRIPT})
endif()
hphp_link(hhvm)
HHVM_CONFIGURE_TARGET_FOR_EXTENSION_DEPENDENCIES(hhvm)

# These mocks need to be added for the rust/ocaml FFI to work properly
target_sources(hhvm PUBLIC ../hack/src/utils/ocaml_ffi_mock/ocaml.c)

if (ENABLE_SPLIT_DWARF)
  if (NOT GOLD_FOUND)
    message(FATAL_ERROR "ENABLE_SPLIT_DWARF was specified but the gold linker was not found")
  endif()
  message(STATUS "Enabled split DWARF files")
  target_link_libraries(hhvm debug "-Wl,--gdb-index")
endif()

embed_all_systemlibs(hhvm "${CMAKE_CURRENT_BINARY_DIR}/.." "${CMAKE_CURRENT_BINARY_DIR}/hhvm")
add_dependencies(hhvm systemlib)

if (CMAKE_HOST_UNIX)
  add_custom_command(TARGET hhvm POST_BUILD
    COMMAND INSTALL_DIR=${CMAKE_CURRENT_BINARY_DIR} sh ${CMAKE_CURRENT_SOURCE_DIR}/symlinks.sh ${CMAKE_CURRENT_BINARY_DIR}/hhvm
    COMMENT "Creating symlinks for hhvm")
endif()

if (ENABLE_COTIRE)
  cotire(hhvm)
endif()

HHVM_INSTALL(hhvm bin)

auto_sources(files "*.h" "")
HHVM_PUBLIC_HEADERS(hhvm ${files})
