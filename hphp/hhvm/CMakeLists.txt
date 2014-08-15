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

add_executable(hhvm ${CXX_SOURCES})
target_link_libraries(hhvm ${HHVM_LINK_LIBRARIES})

# cygwin has an issue with linking to crypt and intl
# this is the only way the redirection stuff works
if(CYGWIN)
  target_link_libraries(hhvm crypt)
  target_link_libraries(hhvm intl)
  target_link_libraries(hhvm dbghelp)
endif()

embed_all_systemlibs(hhvm "${CMAKE_CURRENT_BINARY_DIR}/.." "${CMAKE_CURRENT_BINARY_DIR}/hhvm")
if(CYGWIN OR MSVC OR MINGW)
  add_dependencies(hhvm generate_rc)
else()
  add_dependencies(hhvm systemlib)
endif()

if (ENABLE_COTIRE)
  cotire(hhvm)
endif()

HHVM_INSTALL(hhvm bin)

auto_sources(files "*.h" "")
HHVM_PUBLIC_HEADERS(hhvm ${files})
