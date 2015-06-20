set(CXX_SOURCES)
auto_sources(files "*.cpp" "")
list(APPEND CXX_SOURCES ${files})

# Windows targets use a generate rc file for embedding libraries
if(CYGWIN OR MSVC OR MING)
  add_custom_target(generate_embed
    COMMAND echo \"// THIS IS A GENERATED FILE \" > ${CMAKE_CURRENT_SOURCE_DIR}/embed.rc
    COMMENT "generating embed.rc")

  # this is a cmake trick to allow items added later to depend properly
  add_custom_target(generate_rc
    DEPENDS generate_embed
    COMMENT "Generating RC file")

    list(APPEND CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/embed.rc)
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/embed.rc)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/embed.rc
      PROPERTIES GENERATED TRUE)
endif()

find_package(Gold)

if (GOLD_FOUND)
  SET(LINKER_SCRIPT -fuse-ld=gold -Wl,--section-ordering-file,${CMAKE_CURRENT_SOURCE_DIR}/../tools/oss_hot_section_ordering,--icf=all,--gc-sections)
  add_executable(hhvm ${CXX_SOURCES})
  target_link_libraries(hhvm ${HHVM_LINK_LIBRARIES} ${LINKER_SCRIPT})
else()
  add_executable(hhvm ${CXX_SOURCES})
  target_link_libraries(hhvm ${HHVM_LINK_LIBRARIES})
endif()

# cygwin has an issue with linking to crypt and intl
# this is the only way the redirection stuff works
if(CYGWIN)
  target_link_libraries(hhvm crypt)
  target_link_libraries(hhvm intl)
  target_link_libraries(hhvm dbghelp)
endif()

# Check whether atomic operations require -latomic or not
# See https://github.com/facebook/hhvm/issues/5217
INCLUDE(CheckCXXSourceCompiles)
set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_FLAGS "-std=c++11")
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
if(CYGWIN OR MSVC OR MINGW)
  add_dependencies(hhvm generate_rc)
else()
  add_dependencies(hhvm systemlib)
endif()

if (CMAKE_HOST_UNIX)
  add_custom_command(TARGET hhvm POST_BUILD
    COMMAND INSTALL_DIR=${CMAKE_CURRENT_BINARY_DIR} sh "${CMAKE_CURRENT_SOURCE_DIR}/symlinks.sh"
    COMMENT "Creating symlinks for hhvm")
endif()

if (ENABLE_COTIRE)
  cotire(hhvm)
endif()

HHVM_INSTALL(hhvm bin)

auto_sources(files "*.h" "")
HHVM_PUBLIC_HEADERS(hhvm ${files})
