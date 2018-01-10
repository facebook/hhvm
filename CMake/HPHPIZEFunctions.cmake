# Functions for use when building extensions dynamically

# These functions also exist in CMake/EXTFunctions.cmake
# Their signatures should be kept consistent, though their behavior
# will differ slightly.

function(HHVM_LINK_LIBRARIES EXTNAME)
  target_link_libraries(${EXTNAME} ${ARGN})
endfunction()

function(HHVM_ADD_INCLUDES EXTNAME)
  include_directories(${ARGN})
endfunction()

# Add an extension
function(HHVM_EXTENSION EXTNAME)
  set(EXTENSION_NAME ${EXTNAME} CACHE INTERNAL "")
  add_library(${EXTNAME} SHARED ${ARGN})
  set_target_properties(${EXTNAME} PROPERTIES PREFIX "")
  set_target_properties(${EXTNAME} PROPERTIES SUFFIX ".so")

  get_target_property(LOC ${EXTNAME} LOCATION)
  get_target_property(TY ${EXTNAME} TYPE)
  # Don't install via target, because it triggers a re-link that doesn't
  # run the POST_BUILD custom command that embeds the systemlib on Linux.
  install(CODE "FILE(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/hhvm/extensions/${HHVM_VERSION_BRANCH}\" TYPE ${TY} FILES \"${LOC}\")")
endfunction()

function(embed_systemlibs TARGET DEST)
  if (APPLE)
    target_link_libraries(${TARGET} ${${TARGET}_SLIBS})
  elseif (MSVC)
    message(FATAL_ERROR "Shared extensions are not supported on Windows")
  else()
    add_custom_command(TARGET ${TARGET} POST_BUILD
      COMMAND "objcopy"
      ARGS ${${TARGET}_SLIBS} ${DEST}
      COMMENT "Embedding php in ${TARGET}")
  endif()
endfunction(embed_systemlibs)

function(HHVM_SYSTEMLIB EXTNAME)
  foreach (SLIB ${ARGN})
    embed_systemlib_byname(${EXTNAME} ${SLIB})
  endforeach()
  embed_systemlibs(${EXTNAME} "${EXTNAME}.so")
endfunction()

function(HHVM_DEFINE EXTNAME)
  add_definitions(${ARGN})
endfunction()
