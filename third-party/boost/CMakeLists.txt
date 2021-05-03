# boost checks
option(FORCE_BUNDLED_BOOST "Always build boost, instead of using system version" OFF)
set(
  BOOST_COMPONENTS
  context
  fiber
  filesystem
  headers
  iostreams
  program_options
  regex
  system
  thread
)
if(FORCE_BUNDLED_BOOST)
  set(Boost_FOUND false)
else()
  find_package(Boost 1.62.0 COMPONENTS ${BOOST_COMPONENTS})
  if("${Boost_VERSION}" EQUAL "107000")
    # https://github.com/boostorg/variant/issues/69
    message(WARNING "System boost is blacklisted version")

    set(Boost_FOUND false)
  endif()
endif()

add_library(boost INTERFACE)

if(Boost_FOUND)
  message(STATUS "Using system boost")
  target_include_directories(boost BEFORE INTERFACE ${Boost_INCLUDE_DIRS})
  # Not just using ${Boost_LIBRARIES} as this includes imported targets, and
  # third-party dependents (e.g folly) may need the actual path
  foreach(COMPONENT ${BOOST_COMPONENTS})
    string(TOUPPER ${COMPONENT} COMPONENT)
    if(EXISTS "${Boost_${COMPONENT}_LIBRARY_RELEASE}")
      target_link_libraries(boost INTERFACE ${Boost_${COMPONENT}_LIBRARY_RELEASE})
    elseif(EXISTS "${Boost_${COMPONENT}_LIBRARY_DEBUG}")
      target_link_libraries(boost INTERFACE ${Boost_${COMPONENT}_LIBRARY_DEBUG})
    endif()
  endforeach()
else()
  message(STATUS "Using third-party bundled boost")
  include(bundled_boost.cmake)
endif()
