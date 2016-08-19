find_package(LibXed REQUIRED)
if (LibXed_INCLUDE_DIR AND LibXed_LIBRARY)
  include_directories(${LibXed_INCLUDE_DIR})
  add_definitions("-DHAVE_LIBXED")
  add_executable(tc-print "mappers.cpp" "offline-trans-data.cpp"
                          "offline-x86-code.cpp"  "perf-events.cpp"
                          "repo-wrapper.cpp"  "tc-print.cpp"
                          "../../hhvm/process-init.cpp")
  link_object_libraries(tc-print ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
  target_link_libraries(tc-print ${HHVM_LINK_LIBRARIES})
  embed_all_systemlibs(tc-print "${CMAKE_CURRENT_BINARY_DIR}/../.."
                                "${CMAKE_CURRENT_BINARY_DIR}/tc-print")
endif()
