add_executable(gen-ext-hhvm "gen-ext-hhvm.cpp" "idl.cpp")
target_link_libraries(gen-ext-hhvm folly ${LIBGLOG_LIBRARY} double-conversion)

add_executable(gen-infotabs "gen-infotabs.cpp" "idl.cpp")
target_link_libraries(gen-infotabs folly ${LIBGLOG_LIBRARY} double-conversion)
