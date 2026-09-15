include(HHVMExtensionConfig)

set(CXX_SOURCES)
auto_sources(files "*.cpp" "")
list(APPEND CXX_SOURCES ${files})
list(REMOVE_ITEM CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/process-init.cpp)

add_library(hphp_process_init STATIC process-init.cpp)
hphp_link(hphp_process_init)

get_object_libraries_objects(additionalObjects ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
add_executable(hhvm ${CXX_SOURCES} ${additionalObjects})
target_link_libraries(hhvm ${HHVM_LINK_LIBRARIES} ${EZC_LINK_LIBRARIES} ${HRE_LINK_LIBRARIES})
link_object_libraries(hhvm ${HHVM_WHOLE_ARCHIVE_LIBRARIES})

hphp_link(hhvm)
HHVM_CONFIGURE_TARGET_FOR_EXTENSION_DEPENDENCIES(hhvm)
if (NOT WINDOWS)
  target_link_libraries(
    hhvm ${LIBELF_LIBRARIES} ${LIBDWARF_LIBRARIES} ${ZLIB_LIBRARIES}
  )
endif()

# These mocks need to be added for the rust/ocaml FFI to work properly
target_sources(hhvm PUBLIC ../hack/src/utils/ocaml_ffi_mock/ocaml.c)

if (ENABLE_SPLIT_DWARF)
  message(STATUS "Enabled split DWARF files")
  target_link_libraries(hhvm debug "-Wl,--gdb-index")
endif()

embed_all_systemlibs(
  hhvm "${CMAKE_CURRENT_BINARY_DIR}/.." "${CMAKE_CURRENT_BINARY_DIR}/hhvm"
)

if (CMAKE_HOST_UNIX)
  add_custom_command(TARGET hhvm POST_BUILD
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/symlinks.sh
      ${CMAKE_CURRENT_BINARY_DIR}/hhvm ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Creating symlinks for hhvm")
endif()

HHVM_INSTALL(hhvm bin)

auto_sources(files "*.h" "")
HHVM_PUBLIC_HEADERS(hhvm ${files})
