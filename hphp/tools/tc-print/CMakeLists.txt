set(
  TC_PRINT_CXX_SOURCES
  annotation-cache.cpp
  mappers.cpp
  offline-arm-code.cpp
  offline-code.cpp
  offline-trans-data.cpp
  perf-events.cpp
  printir-annotation.cpp
  repo-wrapper.cpp
  std-logger.cpp
  tc-print.cpp
)

if(HHVM_ENABLE_XED)
  if (LibXed_FOUND)
    set(TC_PRINT_XED_LIBRARY ${LibXed_LIBRARY})
  else()
    set(TC_PRINT_XED_LIBRARY xed)
  endif()
  list(APPEND TC_PRINT_CXX_SOURCES offline-x86-code.cpp)
else()
  message(STATUS "XED disabled, tc-print will not be built")
  return()
endif()

add_executable(tc-print ${TC_PRINT_CXX_SOURCES})
add_dependencies(tc-print hhvm)

link_object_libraries(tc-print ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
target_link_libraries(tc-print ${HHVM_LINK_LIBRARIES})
if(NOT HHVM_REQUIRE_XED)
  target_link_libraries(tc-print ${TC_PRINT_XED_LIBRARY})
endif()
hphp_link(tc-print)
if (NOT WINDOWS)
  target_link_libraries(
    tc-print ${LIBELF_LIBRARIES} ${LIBDWARF_LIBRARIES} ${ZLIB_LIBRARIES}
  )
endif()

embed_all_systemlibs(tc-print "${CMAKE_CURRENT_BINARY_DIR}/../.."
                              "${CMAKE_CURRENT_BINARY_DIR}/tc-print")

install(TARGETS tc-print)
