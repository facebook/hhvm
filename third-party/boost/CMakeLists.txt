add_library(boost INTERFACE)

# Every supported platform now includes a new enough boost.
# Keeping this in third-party though as it doesn't seem unlikely that that
# will change in the future, and to provide a consistent way to reference
# dependencies
find_package(
  Boost 1.62.0
  REQUIRED
  COMPONENTS
  context
  fiber
  filesystem
  iostreams
  program_options
  regex
  system
  thread
)
target_include_directories(boost INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries(boost INTERFACE ${Boost_LIBRARIES})
