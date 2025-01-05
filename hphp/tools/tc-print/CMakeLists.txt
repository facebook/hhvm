set(
  TC_PRINT_CXX_SOURCES
  annotation-cache.cpp
  mappers.cpp
  offline-arm-code.cpp
  offline-code.cpp
  offline-trans-data.cpp
  perf-events.cpp
  printir-annotation.cpp
  ../../hhvm/process-init.cpp
  repo-wrapper.cpp
  std-logger.cpp
  tc-print.cpp
)

if(IS_X64)
  if (ENABLE_XED)
    find_package(LibXed)
    if (LibXed_FOUND)
      include_directories(${LibXed_INCLUDE_DIR})
    endif()
    list(APPEND TC_PRINT_CXX_SOURCES offline-x86-code.cpp)
  else()
    message(STATUS "XED disabled, tc-print will not be built")
    return()
  endif()
else()
  list(APPEND TC_PRINT_CXX_SOURCES offline-arm-code.cpp)
endif()

add_executable(tc-print ${TC_PRINT_CXX_SOURCES})
target_link_libraries(tc-print ${HHVM_LINK_LIBRARIES})
link_object_libraries(tc-print ${HHVM_WHOLE_ARCHIVE_LIBRARIES})
embed_all_systemlibs(tc-print "${CMAKE_CURRENT_BINARY_DIR}/../.."
                              "${CMAKE_CURRENT_BINARY_DIR}/tc-print")
