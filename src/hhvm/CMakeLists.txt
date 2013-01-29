
if (USE_HHVM)
	set(HIPHOP_TARGET hhvm)
else()
	set(HIPHOP_TARGET hphp)
endif()

set(CXX_SOURCES)
auto_sources(files "*.cpp" "")
list(APPEND CXX_SOURCES ${files})

add_executable(${HIPHOP_TARGET} ${CXX_SOURCES})
target_link_libraries(${HIPHOP_TARGET} hphp_analysis hphp_runtime_static)

if (USE_HHVM)
	target_link_libraries(hhvm ext_hhvm_static)
endif()
