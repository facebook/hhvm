# Generated files from folly/build/generate_*.py
auto_sources(genfiles "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/gen")

# Main folly library files
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/folly")

# Subprocess requires flatmap from boost 1.48
# For now, that's not actually a requirement, so skip it
find_path(flat_map boost/container/flat_map.hpp)
if (NOT flat_map)
	list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/Subprocess.cpp)
endif ()
add_library(folly STATIC ${files} ${genfiles})
