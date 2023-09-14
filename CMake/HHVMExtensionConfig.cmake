# This file holds the configuration mechanism for extensions.
#
# Now, the structure of the globals this uses.
#
# HHVM_EXTENSION_COUNT: <int>
# An integer representing the number of extensions that have
# been defined.
#
# HHVM_EXTENSIONS_REQUIRED_LIBRARIES: <list of paths>
# A list of the additional libraries that need to be linked
# against for the enabled extensions.
#
# HHVM_EXTENSIONS_REQUIRED_INCLUDE_DIRS: <list of paths>
# A list of the additional include paths that need to be used
# in order to compile the enabled extensions.
#
# HHVM_EXTENSIONS_REQUIRED_DEFINES: <list of strings>
# A list of the additional defines that need to be used in order
# to compile the enabled extensions.
#
#
# The extensions' internal info is stored in globals, prefixed by
# HHVM_EXTENSION_#_ where # represents a number between 0 and
# HHVM_EXTENSION_COUNT.
#
# HHVM_EXTENSION_#_NAME: <string>
# The name of extension.
#
# HHVM_EXTENSION_#_PRETTY_NAME: <string>
# The name of the extension to use in messages.
#
# HHVM_EXTENSION_#_REQUIRED: <ON/OFF>
# If ON, then the extension is integral to the function
# of HHVM itself, so failing to build it is not an option,
# and a FATAL_ERROR should be triggered if dependencies
# fail.
#
# HHVM_EXTENSION_#_ROOT_DIR: <string>
# The root directory to which all file paths
# referenced by the extension are relative to.
#
# HHVM_EXTENSION_#_ENABLED_STATE: <int {0, 1, 2, 3, 4}>
# The state of an extension's enabling. If this is 0, then the extension
# may be enabled once dependency calculation is performed. If this is 1,
# then the extension is enabled, and if it is 2, then it is disabled.
# If this is 3, then the extension has been forcefully enabled, and its
# dependencies should be checked. If this is 4, then the extension is a
# 'wanted' extension, and we should error if dependencies for it can't
# be resolved, unless the dependency that fails is an os* or var* dependency,
# in which case, we don't error, but just disable the extension.
#
# HHVM_EXTENSION_#_SOURCE_FILES: <list>
# The list of files to compile for the extension.
#
# HHVM_EXTENSION_#_HEADER_FILES: <list>
# The list of header files that make up this extension.
#
# HHVM_EXTENSION_#_SYSTEMLIB: <list>
# The list of php files that make up this extension's own systemlib.
#
# HHVM_EXTENSION_#_DEPENDENCIES: <list>
# The list of dependencies of this extension. For details on the specifics
# of values in this list, see the documentation of the DEPENDS parameter
# of HHVM_DEFINE_EXTENSION.
#
# HHVM_EXTENSION_#_DEPENDENCIES_OPTIONAL: <list>
# A list of ON/OFF values mapping to the values in HHVM_EXTENSION_#_DEPENDENCIES.
# If the value is ON, then the dependency is optional, and the build should
# not fail if the dependency can't be resolved.

include(CheckFunctionExists)
include(HPHPFunctions)
include(Options)

# function HHVM_DEFINE_EXTENSION:
# This is the function that each individual extension will call. It
# defines everything about the extension.
#
# Note that HRE_CURRENT_EXT_PATH should have been defined before calling this,
# and it should be set to the root directory to which all paths passed to this
# function are relative to.
#
# Parameters:
#
# NAME
# The name of the extension. This name will be used in the variable names,
# so spaces are not allowed.
#
# [REQUIRED]
# This extension is integral to the functioning of HHVM, and
# can not be disabled via `-DENABLE_EXTENSION_FOO=Off`.
# A FATAL_ERROR will be triggered if dependencies fail to resolve.
#
# [IMPLICIT]
# If the library dependencies for this extension fail to resolve,
# and it has not be explicitly enabled via `-DENABLE_EXTENSION_FOO=On`,
# then it will be implicitly disabled by the build system.
#
# [WANTED] (default)
# If the library dependencies for this extension fail to resolve,
# and it has not been explicitly disabled with `-DENABLE_EXTENSION_FOO=Off`
# a FATAL_ERROR will be triggered, unless the dependency that fails is
# an os* or var* dependency, in which case the extension will be implicitly
# disabled by the build system.
#
# Note that it does not make sense to specify more than one of the above
# three settings as the behavior they imply is mutually exclusive.
# Using more than one will result in undefined behavior.
#
# [PRETTY_NAME string]
# If passed, use this name when naming the extension in messages. If this is
# not passed, default to NAME.
#
# [IS_ENABLED VARNAME]
# If the parameter passed is defined, and has a trueish value,
# then the extension will be enabled. This is only used to maintain
# backwards compatibility with existing options. All other
# extensions can be enabled or disabled with ENABLE_EXTENSION_*.
# The ENABLE_EXTENSION_* variables will also be defined for the source
# code so that fallbacks may be used where needed.
#
# [SOURCES ...]
# The source files of the extension
#
# [HEADERS ...]
# The header files of the extension
#
# [SYSTEMLIB ...]
# The PHP API of the extension.
#
# [HACK_SYSTEMLIB_DIR ...]
# A directory containing a Hack library that should be part of the
# extension.
#
# [DEPENDS ...]
# The dependencies of the extension. Extensions are prefixed
# with "ext_", and external libraries with "lib".
# "systemlib" is a special dependency that represents the
# systemlib header.
#
# A dependency may optionally be followed by "OPTIONAL", which
# means that the build won't fail if the dependency is not found.
#
# Dependencies prefixed with "os" represent the OS required to
# build the extension. The only valid value for this currently
# is osPosix, which represents everything with a valid posix
# API, which is most everything except for Windows.
#
# Dependencies prefixed with "var" represent a CMake variable
# which must evaluate to a trueish value for the extension to
# be enabled. If the value isn't defined, it is assumed to be
# false.
#
# If there is a space in an argument with a string in it, and
# the argument is a library, the exact version of the library
# required is expected to be the second part of the string.
# For example, "libFribidi 0.19.6" would require the Fribidi
# package to be exactly version 0.19.6.
#
# For libBoost, a single component is expected to be specified
# by appending a -componentName to the value, for example
# libBoost-variant would require the variant component of libBoost.
# This is only required if a library needs to be linked against.
# If a boost component is a headers-only library, libBoost is
# enough of a dependency.
#
# Note that libFolly is currently a dependency of everything
# for the sanity of the Windows port.
function(HHVM_DEFINE_EXTENSION extNameIn)
  if (NOT DEFINED HHVM_EXTENSION_COUNT)
    set(HHVM_EXTENSION_COUNT 0)
  endif()

  set(extensionName "")
  set(extensionPrettyName "")
  set(extensionRequired OFF)
  # If WANTED is specified, then the extension must be explicitly disabled
  # If IMPLICIT is specified, then the extension will be implicitly disabled
  # when the dependencies are not found
  # If neither is specified, we default to WANTED anyway
  set(extensionEnabledState 4)
  set(extensionSources)
  set(extensionHeaders)
  set(extensionLibrary)
  set(extensionDependencies)
  set(extensionDependenciesOptional)

  # Make sure there are no spaces.
  string(FIND ${extNameIn} " " extNameSpace)
  if (NOT ${extNameSpace} EQUAL -1)
    message(FATAL_ERROR "An extension name cannot have a space in it! Got name '${extNameIn}'.")
  endif()

  # Make sure another extension with the same hasn't already
  # been defined.
  set(i 0)
  while (i LESS HHVM_EXTENSION_COUNT)
    if (${HHVM_EXTENSION_${i}_NAME} STREQUAL ${extNameIn})
      message(FATAL_ERROR "An extension with the name '${extNameIn}' has already been defined!")
    endif()
    math(EXPR i "${i} + 1")
  endwhile()
  set(extensionName ${extNameIn})
  set(extensionPrettyName ${extensionName})

  set(argumentState 0)
  foreach (arg ${ARGN})
    if ("x${arg}" STREQUAL "xPRETTY_NAME")
      set(argumentState 1)
    elseif ("x${arg}" STREQUAL "xIS_ENABLED")
      set(argumentState 2)
    elseif ("x${arg}" STREQUAL "xSOURCES")
      set(argumentState 3)
    elseif ("x${arg}" STREQUAL "xHEADERS")
      set(argumentState 4)
    elseif ("x${arg}" STREQUAL "xSYSTEMLIB")
      set(argumentState 5)
    elseif ("x${arg}" STREQUAL "xDEPENDS")
      set(argumentState 7)
    elseif ("x${arg}" STREQUAL "xHACK_SYSTEMLIB_DIR")
      set(argumentState 8)
    elseif ("x${arg}" STREQUAL "xREQUIRED")
      if (NOT ${argumentState} EQUAL 0)
        message(FATAL_ERROR "The REQUIRED modifier should only be placed immediately after the extension's name! (while processing the '${extensionPrettyName}' extension)")
      endif()
      set(extensionRequired ON)
    elseif ("x${arg}" STREQUAL "xIMPLICIT")
      if (NOT ${argumentState} EQUAL 0)
        message(FATAL_ERROR "The IMPLICIT modifier should only be placed immediately after the extension's name! (while processing the '${extensionPrettyName}' extension)")
      endif()
      set(extensionEnabledState 0)
    elseif ("x${arg}" STREQUAL "xWANTED")
      if (NOT ${argumentState} EQUAL 0)
        message(FATAL_ERROR "The WANTED modifier should only be placed immediately after the extension's name! (while processing the '${extensionPrettyName}' extension)")
      endif()
      set(extensionEnabledState 4)
    elseif ("x${arg}" STREQUAL "xOPTIONAL")
      if (${argumentState} EQUAL 7)
        list(LENGTH extensionDependenciesOptional optDepLen)
        math(EXPR optDepLen "${optDepLen} - 1")
        list(REMOVE_AT extensionDependenciesOptional ${optDepLen})
        list(APPEND extensionDependenciesOptional ON)
      else()
        message(FATAL_ERROR "The OPTIONAL modifier is only currently valid in the DEPENDS section of extension '${extensionPrettyName}'!")
      endif()
    elseif (${argumentState} EQUAL 0)
      message(FATAL_ERROR "Unknown argument '${arg}' while processing extension '${extensionPrettyName}'!")
    elseif (${argumentState} EQUAL 1)
      # PRETTY_NAME
      set(extensionPrettyName ${arg})
      set(argumentState 0)
    elseif (${argumentState} EQUAL 2)
      # IS_ENABLED
      if (DEFINED ${arg})
        if (${${arg}})
          set(extensionEnabledState 3)
        else()
          set(extensionEnabledState 2)
        endif()
      endif()
      set(argumentState 0)
    elseif (${argumentState} EQUAL 3)
      # SOURCES
      list(FIND extensionSources ${arg} listIDX)
      if (NOT ${listIDX} EQUAL -1)
        message(FATAL_ERROR "The file '${arg}' was already specified as a source of '${extensionPrettyName}'!")
      endif()
      list(APPEND extensionSources ${arg})
    elseif (${argumentState} EQUAL 4)
      # HEADERS
      list(FIND extensionHeaders ${arg} listIDX)
      if (NOT ${listIDX} EQUAL -1)
        message(FATAL_ERROR "The file '${arg}' was already specified as a header of '${extensionPrettyName}'!")
      endif()
      list(APPEND extensionHeaders ${arg})
    elseif (${argumentState} EQUAL 5)
      # SYSTEMLIB
      list(FIND extensionLibrary ${arg} listIDX)
      if (NOT ${listIDX} EQUAL -1)
        message(FATAL_ERROR "The file '${arg}' was already specified as part of the library of '${extensionPrettyName}'!")
      endif()
      list(APPEND extensionLibrary ${arg})
    elseif (${argumentState} EQUAL 7)
      # DEPENDS
      list(FIND extensionDependencies ${arg} listIDX)
      if (NOT ${listIDX} EQUAL -1)
        message(FATAL_ERROR "'${arg}' was already specified as a dependency of '${extensionPrettyName}'!")
      endif()
      list(APPEND extensionDependencies ${arg})
      list(APPEND extensionDependenciesOptional OFF)
    elseif (${argumentState} EQUAL 8)
      # HACK_SYSTEMLIB_DIR
      set(singleFilePath "${CMAKE_CURRENT_BINARY_DIR}/ext_${extensionName}.hack")
      if(IS_ABSOLUTE ${arg})
        set(sourceDir ${arg})
      else()
        set(sourceDir "${HRE_CURRENT_EXT_PATH}/${arg}")
      endif()
      add_custom_command(
        OUTPUT "${singleFilePath}"
        COMMAND
        "${CMAKE_SOURCE_DIR}/hphp/hack/scripts/concatenate_all.sh"
        "--install_dir=${CMAKE_CURRENT_BINARY_DIR}"
        "--root=${sourceDir}"
        "--output_file=${singleFilePath}"
        VERBATIM
      )
      add_custom_target(
        "ext_${extensionName}_generated_systemlib"
        DEPENDS
        "${singleFilePath}"
      )
      add_dependencies(generated_systemlib "ext_${extensionName}_generated_systemlib")
      list(APPEND extensionLibrary "${singleFilePath}")
    else()
      message(FATAL_ERROR "An error occurred while processing the arguments of the '${extensionPrettyName}' extension!")
    endif()
  endforeach()

  # Check if the extension has been explicitly enabled or disabled.
  string(TOUPPER ${extensionName} upperExtName)
  if (DEFINED ENABLE_EXTENSION_${upperExtName})
    if (${ENABLE_EXTENSION_${upperExtName}})
      set(extensionEnabledState 3)
    elseif (${extensionRequired})
      message(WARNING "Attempt to explicitly disable the required extension '${extensionPrettyName}' by setting 'ENABLE_EXTENSION_${upperExtName}' was ignored.")
    else()
      set(extensionEnabledState 2)
    endif()
  endif()

  # Increment the extension count.
  set(extensionID ${HHVM_EXTENSION_COUNT})
  math(EXPR newCount "${HHVM_EXTENSION_COUNT} + 1")
  set(HHVM_EXTENSION_COUNT ${newCount} PARENT_SCOPE)

  # And lastly, export the globals.
  # We put these in the cache to make debugging easier.
  # The only one that absolutely has to be in the cache is
  # the ENABLED_STATE, due to it's modification from fairly
  # arbitrary scope depths. HHVM_EXTENSION_COUNT must NEVER
  # be in the cache, otherwise this will break.
  set(HHVM_EXTENSION_${extensionID}_NAME ${extensionName} CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSION_${extensionID}_PRETTY_NAME ${extensionPrettyName} CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSION_${extensionID}_REQUIRED ${extensionRequired} CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSION_${extensionID}_ROOT_DIR ${HRE_CURRENT_EXT_PATH} CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSION_${extensionID}_ENABLED_STATE ${extensionEnabledState} CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSION_${extensionID}_SOURCE_FILES ${extensionSources} CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSION_${extensionID}_HEADER_FILES ${extensionHeaders} CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSION_${extensionID}_SYSTEMLIB ${extensionLibrary} CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSION_${extensionID}_DEPENDENCIES ${extensionDependencies} CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSION_${extensionID}_DEPENDENCIES_OPTIONAL ${extensionDependenciesOptional} CACHE INTERNAL "" FORCE)
endfunction()

# Call after all of the calls to HHVM_DEFINE_EXTENSION are complete.
#
# This will also add the appropriate libraries, include directories, and
# defines for the enabled extensions' dependencies.
function(HHVM_EXTENSION_RESOLVE_DEPENDENCIES)
  set(HHVM_EXTENSIONS_REQUIRED_LIBRARIES "" CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSIONS_REQUIRED_INCLUDE_DIRS "" CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSIONS_REQUIRED_DEFINES "" CACHE INTERNAL "" FORCE)
  set(i 0)
  while (i LESS HHVM_EXTENSION_COUNT)
    HHVM_EXTENSION_INTERNAL_RESOLVE_DEPENDENCIES_OF_EXTENSION(wasResolved ${i} " ")
    string(TOUPPER ${HHVM_EXTENSION_${i}_NAME} upperExtName)
    if (${wasResolved} EQUAL 1)
      message(STATUS "Building the ${HHVM_EXTENSION_${i}_PRETTY_NAME} extension.")

      # Now we need to make sure the dependencies are included and linked in
      # correctly.
      set(i2 0)
      list(LENGTH HHVM_EXTENSION_${i}_DEPENDENCIES depCount)
      while (i2 LESS depCount)
        list(GET HHVM_EXTENSION_${i}_DEPENDENCIES ${i2} currentDependency)
        string(FIND ${currentDependency} "lib" libIdx)
        if (${libIdx} EQUAL 0)
          HHVM_EXTENSION_INTERNAL_HANDLE_LIBRARY_DEPENDENCY(${i} ${currentDependency} ON)
        endif()
        math(EXPR i2 "${i2} + 1")
      endwhile()

      if (HHVM_EXTENSION_${i}_REQUIRED)
        set(ENABLE_EXTENSION_${upperExtName} ON CACHE INTERNAL "Enable the ${HHVM_EXTENSION_${i}_PRETTY_NAME} extension.")
      else()
        set(ENABLE_EXTENSION_${upperExtName} ON CACHE BOOL "Enable the ${HHVM_EXTENSION_${i}_PRETTY_NAME} extension.")
      endif()
    else()
      if (HHVM_EXTENSION_${i}_REQUIRED)
        message(FATAL_ERROR "Failed to resolve a dependency of the ${HHVM_EXTENSION_${i}_PRETTY_NAME} extension, which is a required extension!")
      endif()
      message("Not building the ${HHVM_EXTENSION_${i}_PRETTY_NAME} extension.")
      set(ENABLE_EXTENSION_${upperExtName} OFF CACHE BOOL "Enable the ${HHVM_EXTENSION_${i}_PRETTY_NAME} extension.")
    endif()
    math(EXPR i "${i} + 1")
  endwhile()
endfunction()

# This will append the files of the enabled extensions to the following variables:
# C_SOURCES: C Source Files
# CXX_SOURCES: C++ Source Files
# HEADER_SOURCES: C/C++ Header Files
# ASM_SOURCES: asm source files appropriate for the current compiler.
# PHP_SOURCES: PHP files representing the various extensions' systemlib.
function (HHVM_EXTENSION_BUILD_SOURCE_LISTS)
  set(i 0)
  while (i LESS HHVM_EXTENSION_COUNT)
    if (${HHVM_EXTENSION_${i}_ENABLED_STATE} EQUAL 1)
      HHVM_EXTENSION_INTERNAL_SORT_OUT_SOURCES(${HHVM_EXTENSION_${i}_ROOT_DIR}
        ${HHVM_EXTENSION_${i}_SOURCE_FILES}
        ${HHVM_EXTENSION_${i}_HEADER_FILES}
        ${HHVM_EXTENSION_${i}_SYSTEMLIB}
      )
    endif()
    math(EXPR i "${i} + 1")
  endwhile()

  # Propagate the extra files to the parent scope.
  set(C_SOURCES ${C_SOURCES} PARENT_SCOPE)
  set(CXX_SOURCES ${CXX_SOURCES} PARENT_SCOPE)
  set(HEADER_SOURCES ${HEADER_SOURCES} PARENT_SCOPE)
  set(ASM_SOURCES ${ASM_SOURCES} PARENT_SCOPE)
  set(PHP_SOURCES ${PHP_SOURCES} PARENT_SCOPE)
endfunction()

# Sort out all the files into their appropriate variable, as well as transform the paths
# to their fully-resolved forms.
function (HHVM_EXTENSION_INTERNAL_SORT_OUT_SOURCES rootDir)
  foreach (fileName ${ARGN})
    string(LENGTH ${fileName} fileNameLength)
    string(FIND ${fileName} "." dotPos REVERSE)
    if (${dotPos} EQUAL -1)
      message(FATAL_ERROR "No extension on file '${fileName}'!")
    endif()
    math(EXPR endPos "${fileNameLength} - ${dotPos}")
    string(SUBSTRING ${fileName} ${dotPos} ${endPos} fileExtension)
    string(TOLOWER ${fileExtension} fileExtension)
    if (${fileExtension} STREQUAL ".c")
      list(APPEND C_SOURCES "${rootDir}/${fileName}")
    elseif (${fileExtension} STREQUAL ".cpp" OR ${fileExtension} STREQUAL ".cxx" OR ${fileExtension} STREQUAL ".cc")
      list(APPEND CXX_SOURCES "${rootDir}/${fileName}")
    elseif (${fileExtension} STREQUAL ".h" OR ${fileExtension} STREQUAL ".hpp")
      list(APPEND HEADER_SOURCES "${rootDir}/${fileName}")
    elseif (${fileExtension} STREQUAL ".s")
      # AT&T syntax, MSVC doesn't like.
      if (NOT MSVC)
        list(APPEND ASM_SOURCES "${rootDir}/${fileName}")
      endif()
    elseif (${fileExtension} STREQUAL ".asm")
      # MASM syntax. MSVC only.
      if (MSVC)
        list(APPEND ASM_SOURCES "${rootDir}/${fileName}")
      endif()
    elseif (${fileExtension} STREQUAL ".php")
      list(APPEND PHP_SOURCES "${rootDir}/${fileName}")
    elseif (${fileExtension} STREQUAL ".hack")
      # .hack files are used by typechecked systemlib; there's a directory,
      # and the actual .hack file is generated. As such, it's in the build
      # directory instead, so we have an absolute path
      list(APPEND PHP_SOURCES "${fileName}")
    else()
      message(FATAL_ERROR "Unknown file extension '${fileExtension}'!")
    endif()
  endforeach()
  set(C_SOURCES ${C_SOURCES} PARENT_SCOPE)
  set(CXX_SOURCES ${CXX_SOURCES} PARENT_SCOPE)
  set(HEADER_SOURCES ${HEADER_SOURCES} PARENT_SCOPE)
  set(ASM_SOURCES ${ASM_SOURCES} PARENT_SCOPE)
  set(PHP_SOURCES ${PHP_SOURCES} PARENT_SCOPE)
endfunction()

# Configure the specified target so that it can compile when
# linked against the enabled extensions.
function(HHVM_CONFIGURE_TARGET_FOR_EXTENSION_DEPENDENCIES targetName)
  target_link_libraries(${targetName} ${HHVM_EXTENSIONS_REQUIRED_LIBRARIES})
  target_include_directories(${targetName} PUBLIC ${HHVM_EXTENSIONS_REQUIRED_INCLUDE_DIRS})
  target_compile_definitions(${targetName} PUBLIC ${HHVM_EXTENSIONS_REQUIRED_DEFINES})
endfunction()


# Resolve the dependencies of the specified extension, and update it's enabled state.
function(HHVM_EXTENSION_INTERNAL_RESOLVE_DEPENDENCIES_OF_EXTENSION resolvedDestVar extensionID)
  # If already resolved, return that state.
  if (NOT HHVM_EXTENSION_${extensionID}_ENABLED_STATE EQUAL 0 AND
      NOT HHVM_EXTENSION_${extensionID}_ENABLED_STATE EQUAL 3 AND
      NOT HHVM_EXTENSION_${extensionID}_ENABLED_STATE EQUAL 4)
    set(${resolvedDestVar} ${HHVM_EXTENSION_${extensionID}_ENABLED_STATE} PARENT_SCOPE)
    return()
  endif()

  # If already in resolution stack, it's a circular dependency,
  # assume for now that it's enabled.
  list(FIND HHVM_EXTENSION_RESOLUTION_STACK ${HHVM_EXTENSION_${extensionID}_NAME} resIDX)
  if (NOT ${resIDX} EQUAL -1)
    set(${resolvedDestVar} 1 PARENT_SCOPE)
    return()
  endif()

  # Go through the dependencies, checking each one recursively in turn.
  list(LENGTH HHVM_EXTENSION_${extensionID}_DEPENDENCIES depCount)
  set(i 0)
  while (i LESS depCount)
    list(GET HHVM_EXTENSION_${extensionID}_DEPENDENCIES ${i} currentDependency)

    string(FIND ${currentDependency} "lib" listIDX)
    if (${listIDX} EQUAL 0)
      # Library Dependency
      HHVM_EXTENSION_INTERNAL_HANDLE_LIBRARY_DEPENDENCY(${extensionID} ${currentDependency} OFF)
      if (HHVM_EXTENSION_${extensionID}_ENABLED_STATE EQUAL 2)
        break()
      endif()
    else()
      string(FIND ${currentDependency} "var" listIDX)
      if (${listIDX} EQUAL 0)
        # CMake Variable Dependency
        string(LENGTH ${currentDependency} depLength)
        math(EXPR depLength "${depLength} - 3")
        string(SUBSTRING ${currentDependency} 3 ${depLength} varName)
        if (DEFINED ${varName})
          if (NOT ${${varName}})
            HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${currentDependency} ON)
            break()
          endif()
        else()
          HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${currentDependency} ON)
          break()
        endif()
      else()
        string(FIND ${currentDependency} "os" listIDX)
        if (${listIDX} EQUAL 0)
          # OS Dependency
          if (${currentDependency} STREQUAL "osPosix")
            if (MSVC)
              HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${currentDependency} ON)
              break()
            endif()
          else()
            message(FATAL_ERROR "The only OS restriction that is currently valid is 'osPosix', got '${currentDependency}'!")
          endif()
        elseif (${currentDependency} STREQUAL "systemlib")
          # TODO: Mark this somehow?
        else()
          message(FATAL_ERROR "Unknown dependency '${currentDependency}' for extension '${HHVM_EXTENSION_${extensionID}_PRETTY_NAME}'!")
        endif()
      endif()
    endif()

    math(EXPR i "${i} + 1")
  endwhile()

  if (HHVM_EXTENSION_${extensionID}_ENABLED_STATE EQUAL 0 OR
      HHVM_EXTENSION_${extensionID}_ENABLED_STATE EQUAL 3 OR
      HHVM_EXTENSION_${extensionID}_ENABLED_STATE EQUAL 4)
    set(HHVM_EXTENSION_${extensionID}_ENABLED_STATE 1 CACHE INTERNAL "" FORCE)
  endif()
  set(${resolvedDestVar} ${HHVM_EXTENSION_${extensionID}_ENABLED_STATE} PARENT_SCOPE)
endfunction()

# Set that an extension was disabled because of the specified dependency not being
# possible to resolve.
# This optionally takes a third BOOL parameter that should be set to ON only if the
# dependency that failed to resolve is an os* or var* dependency.
function(HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY extensionID failedDependency)
  list(FIND HHVM_EXTENSION_${extensionID}_DEPENDENCIES ${failedDependency} depIdx)
  if (depIdx EQUAL -1)
    message(FATAL_ERROR "An issue occurred while processing the '${failedDependency}' dependency of the ${HHVM_EXTENSION_${extensionID}_PRETTY_NAME} extension!")
  endif()
  list(GET HHVM_EXTENSION_${extensionID}_DEPENDENCIES_OPTIONAL ${depIdx} isOptional)

  if (NOT ${isOptional})
    if (${HHVM_EXTENSION_${extensionID}_ENABLED_STATE} EQUAL 4 AND (NOT ${ARGC} EQUAL 3 OR NOT "${ARGV2}" STREQUAL "ON"))
      message(FATAL_ERROR "The ${HHVM_EXTENSION_${extensionID}_PRETTY_NAME} extension is an extension you probably want, but resolving the dependency '${failedDependency}' failed!")
    elseif (${HHVM_EXTENSION_${extensionID}_ENABLED_STATE} EQUAL 3)
      message(FATAL_ERROR "The ${HHVM_EXTENSION_${extensionID}_PRETTY_NAME} extension was forcefully enabled, but resolving the dependency '${failedDependency}' failed!")
    elseif (${HHVM_EXTENSION_${extensionID}_ENABLED_STATE} EQUAL 1)
      # Currently only triggers for issues with find_package when applying the library dependencies.
      message(FATAL_ERROR "An error occurred while applying the '${failedDependency}' dependency of the ${HHVM_EXTENSION_${extensionID}_PRETTY_NAME} extension!")
    endif()
    message(STATUS "The ${HHVM_EXTENSION_${extensionID}_PRETTY_NAME} extension was disabled because resolving the dependency '${failedDependency}' failed.")
    set(HHVM_EXTENSION_${extensionID}_ENABLED_STATE 2 CACHE INTERNAL "" FORCE)
  endif()
endfunction()

# Add a set of libraries to link against.
function(HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES)
  set(reqLibs ${HHVM_EXTENSIONS_REQUIRED_LIBRARIES})
  foreach (lib ${ARGN})
    list(APPEND reqLibs ${lib})
  endforeach()
  set(HHVM_EXTENSIONS_REQUIRED_LIBRARIES ${reqLibs} CACHE INTERNAL "" FORCE)
endfunction()

# Add a set of include directories to use.
function(HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS)
  set(incDirs ${HHVM_EXTENSIONS_REQUIRED_INCLUDE_DIRS})
  foreach (inc ${ARGN})
    list(APPEND incDirs ${inc})
  endforeach()
  set(HHVM_EXTENSIONS_REQUIRED_INCLUDE_DIRS ${incDirs} CACHE INTERNAL "" FORCE)
endfunction()

# Add a set of defines to use when compiling.
function(HHVM_EXTENSION_INTERNAL_ADD_DEFINES)
  set(defs ${HHVM_EXTENSIONS_REQUIRED_DEFINES})
  foreach (def ${ARGN})
    list(APPEND defs ${def})
  endforeach()
  set(HHVM_EXTENSIONS_REQUIRED_DEFINES ${defs} CACHE INTERNAL "" FORCE)
endfunction()

# This handles all the library dependencies, and determines if the libraries are present.
function (HHVM_EXTENSION_INTERNAL_HANDLE_LIBRARY_DEPENDENCY extensionID dependencyName addPaths)
  string(FIND ${dependencyName} "lib" libIdx)
  if (NOT libIdx EQUAL 0)
    message(FATAL_ERROR "Non-library dependency '${dependencyName}' passed to HHVM_EXTENSION_INTERNAL_HANDLE_LIBRARY_DEPENDENCY!")
  endif()

  set(requiredVersion)
  string(LENGTH ${dependencyName} depLength)
  math(EXPR depLength "${depLength} - 3")
  string(SUBSTRING ${dependencyName} 3 ${depLength} originalLibraryName)
  string(FIND ${originalLibraryName} " " spaceIDX)
  if (NOT ${spaceIDX} EQUAL -1)
    math(EXPR spaceIDX "${spaceIDX} + 1")
    string(LENGTH ${originalLibraryName} libNameLength)
    math(EXPR libNameLength "${libNameLength} - ${spaceIDX}")
    string(SUBSTRING ${originalLibraryName} ${spaceIDX} ${libNameLength} requiredVersion)
    math(EXPR spaceIDX "${spaceIDX} - 1")
    string(SUBSTRING ${originalLibraryName} 0 ${spaceIDX} originalLibraryName)
  endif()
  string(TOLOWER ${originalLibraryName} libraryName)

  # This first check is for libraries that are used by default
  # Keep these in alphabetical order.
  if (
    ${libraryName} STREQUAL "boost" OR
    ${libraryName} STREQUAL "editline" OR
    ${libraryName} STREQUAL "fastlz" OR
    ${libraryName} STREQUAL "folly" OR
    ${libraryName} STREQUAL "lz4" OR
    ${libraryName} STREQUAL "mbfl" OR
    ${libraryName} STREQUAL "oniguruma" OR
    ${libraryName} STREQUAL "openssl" OR
    ${libraryName} STREQUAL "pcre" OR
    ${libraryName} STREQUAL "readline" OR
    ${libraryName} STREQUAL "sqlite" OR
    ${libraryName} STREQUAL "zip" OR
    ${libraryName} STREQUAL "zlib"
  )
    # Nothing to do, they are included by default.
  elseif (${libraryName} STREQUAL "bzip2")
    find_package(BZip2 ${requiredVersion})
    find_package(EXPAT ${requiredVersion})
    if (NOT BZIP2_INCLUDE_DIR OR NOT BZIP2_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${BZIP2_INCLUDE_DIR})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES(${BZIP2_DEFINITIONS})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${BZIP2_LIBRARIES})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBBZIP2")
    endif()
  elseif (${libraryName} STREQUAL "cclient")
    find_package(CClient ${requiredVersion})
    if (NOT CCLIENT_INCLUDE_PATH OR NOT CCLIENT_LIBRARY)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    CONTAINS_STRING("${CCLIENT_INCLUDE_PATH}/utf8.h" U8T_DECOMPOSE RECENT_CCLIENT)
    if (NOT RECENT_CCLIENT)
      unset(RECENT_CCLIENT CACHE)
      if (${addPaths})
        message(FATAL_ERROR "Your version of c-client is too old, you need 2007")
      else()
        message(STATUS "Your version of c-client is too old, you need 2007")
        HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
        return()
      endif()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${CCLIENT_INCLUDE_PATH})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${CCLIENT_LIBRARY})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBCCLIENT")

      if (EXISTS "${CCLIENT_INCLUDE_PATH}/linkage.c")
        CONTAINS_STRING("${CCLIENT_INCLUDE_PATH}/linkage.c" auth_gss CCLIENT_HAS_GSS)
      elseif (EXISTS "${CCLIENT_INCLUDE_PATH}/linkage.h")
        CONTAINS_STRING("${CCLIENT_INCLUDE_PATH}/linkage.h" auth_gss CCLIENT_HAS_GSS)
      endif()
      if (NOT CCLIENT_HAS_GSS)
        HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DSKIP_IMAP_GSS=1")
      endif()

      if (EXISTS "${CCLIENT_INCLUDE_PATH}/linkage.c")
        CONTAINS_STRING("${CCLIENT_INCLUDE_PATH}/linkage.c" ssl_onceonlyinit CCLIENT_HAS_SSL)
      elseif (EXISTS "${CCLIENT_INCLUDE_PATH}/linkage.h")
        CONTAINS_STRING("${CCLIENT_INCLUDE_PATH}/linkage.h" ssl_onceonlyinit CCLIENT_HAS_SSL)
      endif()
      if (NOT CCLIENT_HAS_SSL)
        HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DSKIP_IMAP_SSL=1")
      endif()
    endif()
  elseif (${libraryName} STREQUAL "curl")
    find_package(CURL ${requiredVersion})
    if (NOT CURL_INCLUDE_DIR OR NOT CURL_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${CURL_INCLUDE_DIR})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${CURL_LIBRARIES})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBCURL")
    endif()
  elseif (${libraryName} STREQUAL "expat")
    find_package(EXPAT ${requiredVersion})
    if (NOT EXPAT_INCLUDE_DIRS OR NOT EXPAT_LIBRARY)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${EXPAT_INCLUDE_DIRS})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${EXPAT_LIBRARY})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBEXPAT")
      if (EXPAT_STATIC)
        HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DXML_STATIC")
      endif()
    endif()
  elseif (${libraryName} STREQUAL "freetype")
    find_package(Freetype ${requiredVersion})
    if (NOT FREETYPE_INCLUDE_DIRS OR NOT FREETYPE_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${FREETYPE_INCLUDE_DIRS})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${FREETYPE_LIBRARIES})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBFREETYPE")
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_GD_FREETYPE")
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DENABLE_GD_TTF")
    endif()
  elseif (${libraryName} STREQUAL "fribidi")
    find_package(fribidi ${requiredVersion})
    if (NOT FRIBIDI_INCLUDE_DIR OR NOT FRIBIDI_LIBRARY)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${FRIBIDI_INCLUDE_DIR})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${FRIBIDI_LIBRARY})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBFRIBIDI")
    endif()
  elseif (${libraryName} STREQUAL "glib")
    find_package(GLIB ${requiredVersion})
    if (NOT GLIB_INCLUDE_DIR OR NOT GLIB_CONFIG_INCLUDE_DIR)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${GLIB_INCLUDE_DIR} ${GLIB_CONFIG_INCLUDE_DIR})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBGLIB")
    endif()
  elseif (${libraryName} STREQUAL "gmp")
    find_package(LibGmp ${requiredVersion})
    if (NOT GMP_INCLUDE_DIR OR NOT GMP_LIBRARY)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${GMP_INCLUDE_DIR})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${GMP_LIBRARY})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBGMP")
    endif()
  elseif (${libraryName} STREQUAL "iconv")
    find_package(Libiconv ${requiredVersion})
    if (NOT LIBICONV_INCLUDE_DIR)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${LIBICONV_INCLUDE_DIR})
      if (LIBICONV_LIBRARY)
        HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${LIBICONV_LIBRARY})
      endif()
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_ICONV")
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBICONV")
      if (LIBICONV_CONST)
        message(STATUS "Using const for input to iconv() call")
        HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DICONV_CONST=const")
      endif()
    endif()
  elseif (${libraryName} STREQUAL "icu")
    find_package(ICU ${requiredVersion})
    if (NOT ICU_FOUND OR NOT ICU_DATA_LIBRARIES OR NOT ICU_I18N_LIBRARIES OR NOT ICU_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (ICU_VERSION VERSION_LESS "4.2")
      unset(ICU_FOUND CACHE)
      unset(ICU_INCLUDE_DIRS CACHE)
      unset(ICU_LIBRARIES CACHE)
      if (${addPaths})
        message(FATAL_ERROR "ICU is too old, found ${ICU_VERSION} and we need 4.2")
      else()
        message(STATUS "ICU is too old, found ${ICU_VERSION} and we need 4.2")
        HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
        return()
      endif()
    endif ()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${ICU_INCLUDE_DIRS})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${ICU_DATA_LIBRARIES} ${ICU_I18N_LIBRARIES} ${ICU_LIBRARIES})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBICU")
    endif()
  elseif (${libraryName} STREQUAL "intl")
    find_package(LibIntl ${requiredVersion})
    if (NOT LIBINTL_INCLUDE_DIRS)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${LIBINTL_INCLUDE_DIRS})
      if (LIBINTL_LIBRARIES)
        HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${LIBINTL_LIBRARIES})
      endif()
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBINTL")
    endif()
  elseif (${libraryName} STREQUAL "jpeg")
    find_package(LibJpeg ${requiredVersion})
    if (NOT LIBJPEG_INCLUDE_DIRS OR NOT LIBJPEG_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${LIBJPEG_INCLUDE_DIRS})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${LIBJPEG_LIBRARIES})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBJPEG")
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_GD_JPG")
    endif()
  elseif (${libraryName} STREQUAL "ldap")
    find_package(Ldap ${requiredVersion})
    if (NOT LDAP_INCLUDE_DIR OR NOT LDAP_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${LDAP_INCLUDE_DIR})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${LDAP_LIBRARIES})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBLDAP")
    endif()
  elseif (${libraryName} STREQUAL "magickwand")
    find_package(LibMagickWand ${requiredVersion})
    if (NOT LIBMAGICKWAND_INCLUDE_DIRS OR NOT LIBMAGICKWAND_LIBRARIES OR NOT LIBMAGICKCORE_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${LIBMAGICKWAND_INCLUDE_DIRS})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${LIBMAGICKWAND_LIBRARIES} ${LIBMAGICKCORE_LIBRARIES})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBMAGICKWAND")
    endif()
  elseif (${libraryName} STREQUAL "mcrypt")
    find_package(Mcrypt ${requiredVersion})
    if (NOT Mcrypt_INCLUDE_DIR OR NOT Mcrypt_LIB)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${Mcrypt_INCLUDE_DIR})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${Mcrypt_LIB})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBMCRYPT")
    endif()
  elseif (${libraryName} STREQUAL "memcached")
    find_package(Libmemcached ${requiredVersion})
    if (NOT LIBMEMCACHED_INCLUDE_DIR OR NOT LIBMEMCACHED_LIBRARY)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (LIBMEMCACHED_VERSION VERSION_LESS "0.39")
      unset(LIBMEMCACHED_INCLUDE_DIR CACHE)
      unset(LIBMEMCACHED_LIBRARY CACHE)
      unset(LIBMEMCACHED_VERSION CACHE)
      if (${addPaths})
        message(FATAL_ERROR "libmemcached is too old, found ${LIBMEMCACHED_VERSION} and we need 0.39")
      else()
        message(STATUS "libmemcached is too old, found ${LIBMEMCACHED_VERSION} and we need 0.39")
        HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
        return()
      endif()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${LIBMEMCACHED_INCLUDE_DIR})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${LIBMEMCACHED_LIBRARY})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBMEMCACHED")
    endif()
  elseif (${libraryName} STREQUAL "mysql")
    HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(fbmysqlclient)
    MYSQL_SOCKET_SEARCH()
    HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DPHP_MYSQL_UNIX_SOCK_ADDR=\"${MYSQL_UNIX_SOCK_ADDR}\"")
  elseif (${libraryName} STREQUAL "png")
    find_package(LibPng ${requiredVersion})
    if (NOT LIBPNG_INCLUDE_DIRS OR NOT LIBPNG_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${LIBPNG_INCLUDE_DIRS})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${LIBPNG_LIBRARIES})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBPNG")
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_GD_PNG")
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DPNG_SKIP_SETJMP_CHECK")
    endif()
  elseif (${libraryName} STREQUAL "snappy")
    find_package(Snappy ${requiredVersion})
    if (NOT SNAPPY_INCLUDE_DIRS OR NOT SNAPPY_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${SNAPPY_INCLUDE_DIRS})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${SNAPPY_LIBRARIES})
    endif()
  elseif (${libraryName} STREQUAL "squangle")
    HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(squangle)
  elseif (${libraryName} STREQUAL "thrift")
    HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(thrift)
  elseif (${libraryName} STREQUAL "vpx")
    find_package(LibVpx ${requiredVersion})
    if (NOT LIBVPX_INCLUDE_DIRS OR NOT LIBVPX_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${LIBVPX_INCLUDE_DIRS})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${LIBVPX_LIBRARIES})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBVPX")
    endif()
  elseif (${libraryName} STREQUAL "xml2")
    find_package(LibXml2 ${requiredVersion})
    if (NOT LIBXML2_INCLUDE_DIR OR NOT LIBXML2_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${LIBXML2_INCLUDE_DIR})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES(${LIBXML2_DEFINITIONS})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${LIBXML2_LIBRARIES})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBXML2")
    endif()
  elseif (${libraryName} STREQUAL "xslt")
    find_package(LibXslt ${requiredVersion})
    if (NOT LIBXSLT_INCLUDE_DIR OR NOT LIBXSLT_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS(${LIBXSLT_INCLUDE_DIR})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES(${LIBXSLT_DEFINITIONS})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${LIBXSLT_LIBRARIES} ${LIBXSLT_EXSLT_LIBRARIES})
      HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DHAVE_LIBXSLT")
      if (LIBXSLT_STATIC)
        HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DLIBXSLT_STATIC=1")
        HHVM_EXTENSION_INTERNAL_ADD_DEFINES("-DLIBEXSLT_STATIC=1")
      endif()
    endif()
  elseif (TARGET "${dependencyName}")
    # If we have libfoo, resolve as libfoo
    message(STATUS "Resolving extension '${HHVM_EXTENSION_${extensionID}_NAME}' dependency '${dependencyName}' as CMake target")
    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${dependencyName})
      get_target_property(DEPENDENCY_TARGET_INCLUDE_DIR ${dependencyName} INTERFACE_INCLUDE_DIRECTORIES)
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS("${DEPENDENCY_TARGET_INCLUDE_DIR}")
    endif ()
  elseif (TARGET "${originalLibraryName}")
    # If we have libfoo, resolve as 'foo'; the `lib` prefix is needed for our cmake to consider it to be a
    # library dependency, so either case is valid :(
    message(STATUS "Resolving extension '${HHVM_EXTENSION_${extensionID}_NAME}' dependency '${dependencyName}' as CMake target '${originalLibraryName}'")
    if (${addPaths})
      HHVM_EXTENSION_INTERNAL_ADD_LINK_LIBRARIES(${originalLibraryName})
      get_target_property(DEPENDENCY_TARGET_INCLUDE_DIR ${originalLibraryName} INTERFACE_INCLUDE_DIRECTORIES)
      HHVM_EXTENSION_INTERNAL_ADD_INCLUDE_DIRS("${DEPENDENCY_TARGET_INCLUDE_DIR}")
    endif ()
  else()
    message(FATAL_ERROR "Unknown library '${originalLibraryName}' as a dependency of the '${HHVM_EXTENSION_${extensionID}_PRETTY_NAME}' extension!")
  endif()
endfunction()
