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
  if (CMAKE_CONFIGURATION_TYPES)
    SET(LINKER_SCRIPT -fuse-ld=gold;-Wl,--icf=all,--gc-sections;release;-Wl,${DEFSYM_HOT_START}${DEFSYM_HOT_END}--section-ordering-file,${SECTION_ORDERING_FILE};default)
  elseif(NOT CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(LINKER_SCRIPT -fuse-ld=gold -Wl,${DEFSYM_HOT_START}${DEFSYM_HOT_END}--section-ordering-file,${SECTION_ORDERING_FILE},--icf=all,--gc-sections)
  else()
    SET(LINKER_SCRIPT -fuse-ld=gold -Wl,--icf=all,--gc-sections)
  endif()
  target_link_libraries(hhvm ${LINKER_SCRIPT})
endif()
hphp_link(hhvm)
HHVM_CONFIGURE_TARGET_FOR_EXTENSION_DEPENDENCIES(hhvm)

if (ENABLE_SPLIT_DWARF)
  if (NOT GOLD_FOUND)
    message(FATAL_ERROR "ENABLE_SPLIT_DWARF was specified but the gold linker was not found")
  endif()
  message(STATUS "Enabled split DWARF files")
  target_link_libraries(hhvm debug "-Wl,--gdb-index")
endif()

# Check whether atomic operations require -latomic or not
# See https://github.com/facebook/hhvm/issues/5217
INCLUDE(CheckCXXSourceCompiles)
set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_FLAGS "-std=c++1y")
CHECK_CXX_SOURCE_COMPILES("
#include <atomic>
int main() {
    struct Test { int val; };
    std::atomic<Test> s;
    s.is_lock_free();
}
" NOT_REQUIRE_ATOMIC_LINKER_FLAG)

if(NOT "${NOT_REQUIRE_ATOMIC_LINKER_FLAG}")
    message(STATUS "-latomic is required to link hhvm")
    target_link_libraries(hhvm atomic)
endif()
set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})

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
