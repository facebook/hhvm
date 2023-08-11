if(IS_X64)
  if (ENABLE_XED)
    find_package(LibXed)
    if (LibXed_FOUND)
      include_directories(${LibXed_INCLUDE_DIR})
    endif()
    add_executable(tc-print "mappers.cpp" "offline-trans-data.cpp"
                            "offline-code.cpp"
                            "offline-x86-code.cpp"  "perf-events.cpp"
                            "repo-wrapper.cpp"  "tc-print.cpp"
                            "../../hhvm/process-init.cpp" "std-logger.cpp")
    link_object_libraries(tc-print ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
    target_link_libraries(tc-print ${HHVM_LINK_LIBRARIES})
    embed_all_systemlibs(tc-print "${CMAKE_CURRENT_BINARY_DIR}/../.."
                                  "${CMAKE_CURRENT_BINARY_DIR}/tc-print")
  else()
    message(STATUS "XED disabled, tc-print will not be built")
  endif()
else()
  add_executable(tc-print "mappers.cpp" "offline-trans-data.cpp"
                          "offline-code.cpp"  "perf-events.cpp"
                          "offline-arm-code.cpp"
                          "repo-wrapper.cpp"  "tc-print.cpp"
                          "../../hhvm/process-init.cpp" "std-logger.cpp")
  link_object_libraries(tc-print ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
  target_link_libraries(tc-print ${HHVM_LINK_LIBRARIES})
  embed_all_systemlibs(tc-print "${CMAKE_CURRENT_BINARY_DIR}/../.."
                                "${CMAKE_CURRENT_BINARY_DIR}/tc-print")
endif()
