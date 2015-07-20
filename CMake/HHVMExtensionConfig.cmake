# This file holds the configuration mechanism for extensions.
#
# Now, the structure of the globals this uses.
#
# HHVM_EXTENSION_COUNT: <int>
# An integer representing the number of extensions that have
# been defined.
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
# HHVM_EXTENSION_#_ENABLED_STATE: <int {0, 1, 2, 3}>
# The state of an extension's enabling. If this is 0, then the extension
# may be enabled once dependency calculation is performed. If this is 1,
# then the extension is enabled, and if it is 2, then it is disabled.
# If this is 3, then the extension has been forcefully enabled, and its
# dependencies should be checked.
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
# HHVM_EXTENSION_#_IDL_FILES: <list>
# The list of .idl.json files that make up this extension's IDL interface.
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
# [EXPLICIT] (default)
# If the library dependencies for this extension fail to resolve,
# and it has not been explicitly disabled with `-DENABLE_EXTENSION_FOO=Off`
# a FATAL_ERROR will be triggered.
#
# Note that it does not make sense to specify more than one of the above
# three settings as the behavior they imply is mutually exclusive.
# Using more than one will result in undefined behavior.
#
# [PRETTY_NAME string]
# If passed, use this name when naming the extension in mesages. If this is
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
# [IDL ...]
# The IDL files of the extension.
#
# [DEPENDS ...]
# The dependencies of the extension. Extensions are prefixed
# with "ext_", and external libaries with "lib".
# "systemlib" is a special dependency that represents the
# systemlib header.
#
# A dependency may optionally be followed by "OPTIONAL", which
# means that the build won't fail if the dependency is not found.
#
# Dependencies prefixed with "os" represent the OS required to
# build the extension. The only valid value for this currently
# is osPosix, which represents everything with a valid posix
# API, which is most everything except for Windows. Cygwin and
# MinGW are both included in osPosix.
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
  # If EXPLICIT is specified, then the extension must be explicitly disabled
  # If IMPLICIT is specified, then the extension will be implicitly disabled
  #   when the dependencies are not found
  # If neither is specified, we default to EXPLICIT anyway
  set(extensionEnabledState 3)
  set(extensionSources)
  set(extensionHeaders)
  set(extensionLibrary)
  set(extensionIDL)
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
    elseif ("x${arg}" STREQUAL "xIDL")
      set(argumentState 6)
    elseif ("x${arg}" STREQUAL "xDEPENDS")
      set(argumentState 7)
    elseif ("x${arg}" STREQUAL "xREQUIRED")
      if (NOT ${argumentState} EQUAL 0)
        message(FATAL_ERROR "The REQUIRED modifier should only be placed immediately after the extension's name! (while processing the '${extensionPrettyName}' extension)")
      endif()
      set(extensionRequired ON)
    elseif ("x${arg}" STREQUAL "xIMPLICIT")
      if (${argumentState} EQUAL 0)
        set(extensionEnabledState 0)
      else()
        message(FATAL_ERROR "The IMPLICIT modifier is only currently valid following the extension name")
      endif()
    elseif ("x${arg}" STREQUAL "xEXPLICIT")
      if (${argumentState} EQUAL 0)
        set(extensionEnabledState 3)
      else()
        message(FATAL_ERROR "The EXPLICIT modifier is only currently valid following the extension name")
      endif()
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
    elseif (${argumentState} EQUAL 6)
      # IDL
      list(FIND extensionIDL ${arg} listIDX)
      if (NOT ${listIDX} EQUAL -1)
        message(FATAL_ERROR "The file '${arg}' was already specified as an IDL file of '${extensionPrettyName}'!")
      endif()
      list(APPEND extensionIDL ${arg})
    elseif (${argumentState} EQUAL 7)
      # DEPENDS
      list(FIND extensionDependencies ${arg} listIDX)
      if (NOT ${listIDX} EQUAL -1)
        message(FATAL_ERROR "'${arg}' was already specified as a dependency of '${extensionPrettyName}'!")
      endif()
      list(APPEND extensionDependencies ${arg})
      list(APPEND extensionDependenciesOptional OFF)
    else()
      message(FATAL_ERROR "An error occurred while processing the arguments of the '${extensionPrettyName}' extension!")
    endif()
  endforeach()

  # Check if the extension has been explicitly enabled or disabled.
  string(TOUPPER ${extensionName} upperExtName)
  if (DEFINED ENABLE_EXTENSION_${upperExtName})
    if (${ENABLE_EXTENSION_${upperExtName}})
      set(extensionEnabledState 3)
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
  set(HHVM_EXTENSION_${extensionID}_IDL_FILES ${extensionIDL} CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSION_${extensionID}_DEPENDENCIES ${extensionDependencies} CACHE INTERNAL "" FORCE)
  set(HHVM_EXTENSION_${extensionID}_DEPENDENCIES_OPTIONAL ${extensionDependenciesOptional} CACHE INTERNAL "" FORCE)
endfunction()

# Call after all of the calls to HHVM_DEFINE_EXTENSION are complete.
#
# This will append the files of the enabled extensions to the following variables:
# C_SOURCES: C Source Files
# CXX_SOURCES: C++ Source Files
# H_SOURCES: C/C++ Header Files
# ASM_SOURCES: asm source files appropriate for the current compiler.
# PHP_SOURCES: PHP files representing the various extensions' systemlib.
# IDL_SOURCES: The .idl.json files for the various extensions.
#
# IDL_DEFINES: Defines necessary for IDL-based extensions
#   Super hacky way to ensure that class_map.cpp and ext_hhvm stubs have access
#   to IDL based extensions via runtime/ext.h
#   "Die IDL-based extensions, die." -sgolemon
#
# This will also add the appropriate libraries, include directories, and
# defines for the enabled extensions' dependencies.
function(HHVM_EXTENSION_RESOLVE_DEPENDENCIES)
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

      HHVM_EXTENSION_INTERNAL_SORT_OUT_SOURCES(${HHVM_EXTENSION_${i}_ROOT_DIR}
        ${HHVM_EXTENSION_${i}_SOURCE_FILES}
        ${HHVM_EXTENSION_${i}_HEADER_FILES}
        ${HHVM_EXTENSION_${i}_SYSTEMLIB}
        ${HHVM_EXTENSION_${i}_IDL_FILES}
      )
      add_definitions("-DENABLE_EXTENSION_${upperExtName}")
      if (HHVM_EXTENSION_${i}_IDL_FILES)
        list(APPEND IDL_DEFINES "-DENABLE_EXTENSION_${upperExtName}")
      endif()
      set(ENABLE_EXTENSION_${upperExtName} ON CACHE BOOL "Enable the ${HHVM_EXTENSION_${i}_PRETTY_NAME} extension.")
    else()
      if (HHVM_EXTENSION_${i}_REQUIRED)
        message(FATAL_ERROR "Failed to resolve a dependency of the ${HHVM_EXTENSION_${i}_PRETTY_NAME} extension, which is a required extension!")
      endif()
      message("Not building the ${HHVM_EXTENSION_${i}_PRETTY_NAME} extension.")
      set(ENABLE_EXTENSION_${upperExtName} OFF CACHE BOOL "Enable the ${HHVM_EXTENSION_${i}_PRETTY_NAME} extension.")
    endif()
    math(EXPR i "${i} + 1")
  endwhile()

  # Propagate the extra files to the parent scope.
  set(C_SOURCES ${C_SOURCES} PARENT_SCOPE)
  set(CXX_SOURCES ${CXX_SOURCES} PARENT_SCOPE)
  set(HEADER_SOURCES ${HEADER_SOURCES} PARENT_SCOPE)
  set(ASM_SOURCES ${ASM_SOURCES} PARENT_SCOPE)
  set(PHP_SOURCES ${PHP_SOURCES} PARENT_SCOPE)
  set(IDL_SOURCES ${IDL_SOURCES} PARENT_SCOPE)
  set(IDL_DEFINES ${IDL_DEFINES} PARENT_SCOPE)
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
    elseif (${fileExtension} STREQUAL ".json")
      string(FIND ${fileName} ".idl.json" idlJsonIdx REVERSE)
      math(EXPR expectedPos "${fileNameLength} - 9")
      if (NOT ${idlJsonIdx} EQUAL ${expectedPos})
        message(FATAL_ERROR "Unknown random .json file in the file list!")
      endif()
      list(APPEND IDL_SOURCES "${rootDir}/${fileName}")
    else()
      message(FATAL_ERROR "Unknown file extension '${fileExtension}'!")
    endif()
  endforeach()
  set(C_SOURCES ${C_SOURCES} PARENT_SCOPE)
  set(CXX_SOURCES ${CXX_SOURCES} PARENT_SCOPE)
  set(HEADER_SOURCES ${HEADER_SOURCES} PARENT_SCOPE)
  set(ASM_SOURCES ${ASM_SOURCES} PARENT_SCOPE)
  set(PHP_SOURCES ${PHP_SOURCES} PARENT_SCOPE)
  set(IDL_SOURCES ${IDL_SOURCES} PARENT_SCOPE)
endfunction()


# Resolve the dependencies of the specified extension, and update it's enabled state.
function(HHVM_EXTENSION_INTERNAL_RESOLVE_DEPENDENCIES_OF_EXTENSION resolvedDestVar extensionID)
  # If already resolved, return that state.
  if (NOT HHVM_EXTENSION_${extensionID}_ENABLED_STATE EQUAL 0 AND
      NOT HHVM_EXTENSION_${extensionID}_ENABLED_STATE EQUAL 3)
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
            HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${currentDependency})
            break()
          endif()
        else()
          HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${currentDependency})
          break()
        endif()
      else()
        string(FIND ${currentDependency} "os" listIDX)
        if (${listIDX} EQUAL 0)
          # OS Dependency
          if (${currentDependency} STREQUAL "osPosix")
            if (MSVC)
              HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${currentDependency})
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
      HHVM_EXTENSION_${extensionID}_ENABLED_STATE EQUAL 3)
    set(HHVM_EXTENSION_${extensionID}_ENABLED_STATE 1 CACHE INTERNAL "" FORCE)
  endif()
  set(${resolvedDestVar} ${HHVM_EXTENSION_${extensionID}_ENABLED_STATE} PARENT_SCOPE)
endfunction()

# Set that an extension was disabled because of the specified dependency not being
# possible to resolve.
function(HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY extensionID failedDependency)
  list(FIND HHVM_EXTENSION_${extensionID}_DEPENDENCIES ${failedDependency} depIdx)
  if (depIdx EQUAL -1)
    message(FATAL_ERROR "An issue occured while processing the '${failedDependency}' dependency of the ${HHVM_EXTENSION_${extensionID}_PRETTY_NAME} extension!")
  endif()
  list(GET HHVM_EXTENSION_${extensionID}_DEPENDENCIES_OPTIONAL ${depIdx} isOptional)

  if (NOT ${isOptional})
    if (${HHVM_EXTENSION_${extensionID}_ENABLED_STATE} EQUAL 3)
      message(FATAL_ERROR "The ${HHVM_EXTENSION_${extensionID}_PRETTY_NAME} extension was forcefully enabled, but resolving the dependency '${failedDependency}' failed!")
    elseif (${HHVM_EXTENSION_${extensionID}_ENABLED_STATE} EQUAL 1)
      # Currently only triggers for issues with find_package when applying the library dependencies.
      message(FATAL_ERROR "An error occurred while applying the '${failedDependency}' dependency of the ${HHVM_EXTENSION_${extensionID}_PRETTY_NAME} extension!")
    endif()
    message(STATUS "The ${HHVM_EXTENSION_${extensionID}_PRETTY_NAME} extension was disabled because resolving the dependency '${failedDependency}' failed.")
    set(HHVM_EXTENSION_${extensionID}_ENABLED_STATE 2 CACHE INTERNAL "" FORCE)
  endif()
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

  # MySQL has to be handled specially
  if (${libraryName} STREQUAL "mysql")
    # mysql checks - if we're using async mysql, we use webscalesqlclient from
    # third-party/ instead
    if (ENABLE_ASYNC_MYSQL)
      set(MYSQL_CLIENT_LIB_DIR ${TP_DIR}/webscalesqlclient/src/)
      # Unlike the .so, the static library intentionally does not link against
      # yassl, despite building it :/
      set(MYSQL_CLIENT_LIBS
        ${MYSQL_CLIENT_LIB_DIR}/libmysql/libwebscalesqlclient_r.a
        ${MYSQL_CLIENT_LIB_DIR}/extra/yassl/libyassl.a
        ${MYSQL_CLIENT_LIB_DIR}/extra/yassl/taocrypt/libtaocrypt.a
      )

      if (${addPaths})
        include_directories(
          ${TP_DIR}/re2/src/
          ${TP_DIR}/squangle/src/
          ${TP_DIR}/webscalesqlclient/src/include/
        )
        add_definitions("-DENABLE_ASYNC_MYSQL=1")
      endif()
    else()
      find_package(MySQL ${requiredVersion})
      if (NOT MYSQL_LIB_DIR OR NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_CLIENT_LIBS)
        HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
        return()
      endif()

      if (${addPaths})
        link_directories(${MYSQL_LIB_DIR})
        include_directories(${MYSQL_INCLUDE_DIR})
      endif()
    endif()

    MYSQL_SOCKET_SEARCH()
    if (MYSQL_UNIX_SOCK_ADDR)
      if (${addPaths})
        add_definitions("-DPHP_MYSQL_UNIX_SOCK_ADDR=\"${MYSQL_UNIX_SOCK_ADDR}\"")
      endif()
    elseif (NOT ${addPaths})
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    else()
      message(FATAL_ERROR "Could not find MySQL socket path - if you install a MySQL server, this should be automatically detected. Alternatively, specify -DMYSQL_UNIX_SOCK_ADDR=/path/to/mysql.socket ; if you don't care about unix socket support for MySQL, specify -DMYSQL_UNIX_SOCK_ADDR=/dev/null")
    endif()

    if (${addPaths})
      link_libraries(${MYSQL_CLIENT_LIBS})
    endif()
    return()
  endif()

  # This first check is for libraries that are used by default
  # Keep these in alphabetical order.
  if (
    ${libraryName} STREQUAL "boost" OR
    ${libraryName} STREQUAL "bz2" OR
    ${libraryName} STREQUAL "editline" OR
    ${libraryName} STREQUAL "fastlz" OR
    ${libraryName} STREQUAL "folly" OR
    ${libraryName} STREQUAL "iconv" OR
    ${libraryName} STREQUAL "lz4" OR
    ${libraryName} STREQUAL "mbfl" OR
    ${libraryName} STREQUAL "mcrouter" OR
    ${libraryName} STREQUAL "oniguruma" OR
    ${libraryName} STREQUAL "openssl" OR
    ${libraryName} STREQUAL "pcre" OR
    ${libraryName} STREQUAL "readline" OR
    ${libraryName} STREQUAL "sqlite" OR
    ${libraryName} STREQUAL "squangle" OR
    ${libraryName} STREQUAL "webscalesql" OR
    ${libraryName} STREQUAL "zip" OR
    ${libraryName} STREQUAL "zlib"
  )
    # Nothing to do, they are included by default.
  elseif (${libraryName} STREQUAL "curl")
    find_package(CURL ${requiredVersion})
    if (NOT CURL_INCLUDE_DIR OR NOT CURL_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      set(CMAKE_REQUIRED_LIBRARIES "${CURL_LIBRARIES}")
      set(CMAKE_REQUIRED_INCLUDES "${CURL_INCLUDE_DIR}")
      CHECK_FUNCTION_EXISTS("curl_multi_select" HAVE_CURL_MULTI_SELECT)
      CHECK_FUNCTION_EXISTS("curl_multi_wait" HAVE_CURL_MULTI_WAIT)
      if (HAVE_CURL_MULTI_SELECT)
        add_definitions("-DHAVE_CURL_MULTI_SELECT")
      endif()
      if (HAVE_CURL_MULTI_WAIT)
        add_definitions("-DHAVE_CURL_MULTI_WAIT")
      endif()
      set(CMAKE_REQUIRED_LIBRARIES)
      set(CMAKE_REQUIRED_INCLUDES)

      include_directories(${CURL_INCLUDE_DIR})
      link_libraries(${CURL_LIBRARIES})
      add_definitions("-DHAVE_LIBCURL")
    endif()
  elseif (${libraryName} STREQUAL "expat")
    find_package(EXPAT ${requiredVersion})
    if (NOT EXPAT_INCLUDE_DIRS OR NOT EXPAT_LIBRARY)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      include_directories(${EXPAT_INCLUDE_DIRS})
      link_libraries(${EXPAT_LIBRARY})
      add_definitions("-DHAVE_LIBEXPAT")
    endif()
  elseif (${libraryName} STREQUAL "fribidi")
    find_package(fribidi ${requiredVersion})
    if (NOT FRIBIDI_INCLUDE_DIR OR NOT FRIBIDI_LIBRARY)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      include_directories(${FRIBIDI_INCLUDE_DIR})
      link_libraries(${FRIBIDI_LIBRARY})
      add_definitions("-DHAVE_LIBFRIBIDI")
    endif()
  elseif (${libraryName} STREQUAL "glib")
    find_package(GLIB ${requiredVersion})
    if (NOT GLIB_INCLUDE_DIR OR NOT GLIB_CONFIG_INCLUDE_DIR)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      include_directories(${GLIB_INCLUDE_DIR} ${GLIB_CONFIG_INCLUDE_DIR})
      add_definitions("-DHAVE_LIBGLIB")
    endif()
  elseif (${libraryName} STREQUAL "gmp")
    find_package(LibGmp ${requiredVersion})
    if (NOT GMP_INCLUDE_DIR OR NOT GMP_LIBRARY)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      include_directories(${GMP_INCLUDE_DIR})
      link_libraries(${GMP_LIBRARY})
      add_definitions("-DHAVE_LIBGMP")
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
      include_directories(${ICU_INCLUDE_DIRS})
      link_libraries(${ICU_DATA_LIBRARIES} ${ICU_I18N_LIBRARIES} ${ICU_LIBRARIES})
      add_definitions("-DHAVE_LIBICU")
    endif()
  elseif (${libraryName} STREQUAL "jsonc")
    find_package(Libjsonc ${requiredVersion})
    if (NOT LIBJSONC_INCLUDE_DIR OR NOT LIBJSONC_LIBRARY)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      include_directories(${LIBJSONC_INCLUDE_DIR})
      link_libraries(${LIBJSONC_LIBRARY})
      add_definitions("-DHAVE_LIBJSONC")
    endif()
  elseif (${libraryName} STREQUAL "ldap")
    find_package(Ldap ${requiredVersion})
    if (NOT LDAP_INCLUDE_DIR OR NOT LDAP_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      include_directories(${LDAP_INCLUDE_DIR})
      link_libraries(${LDAP_LIBRARIES})
      add_definitions("-DHAVE_LIBLDAP")
    endif()
  elseif (${libraryName} STREQUAL "magickwand")
    find_package(LibMagickWand ${requiredVersion})
    if (NOT LIBMAGICKWAND_INCLUDE_DIRS OR NOT LIBMAGICKWAND_LIBRARIES OR NOT LIBMAGICKCORE_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      include_directories(${LIBMAGICKWAND_INCLUDE_DIRS})
      link_libraries(${LIBMAGICKWAND_LIBRARIES} ${LIBMAGICKCORE_LIBRARIES})
      add_definitions("-DHAVE_LIBMAGICKWAND")
    endif()
  elseif (${libraryName} STREQUAL "mcrypt")
    find_package(Mcrypt ${requiredVersion})
    if (NOT Mcrypt_INCLUDE_DIR OR NOT Mcrypt_LIB)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      include_directories(${Mcrypt_INCLUDE_DIR})
      link_libraries(${Mcrypt_LIB})
      add_definitions("-DHAVE_LIBMCRYPT")
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
      include_directories(${LIBMEMCACHED_INCLUDE_DIR})
      link_libraries(${LIBMEMCACHED_LIBRARY})
      add_definitions("-DHAVE_LIBMEMCACHED")
    endif()
  elseif (${libraryName} STREQUAL "xml2")
    find_package(LibXml2 ${requiredVersion})
    if (NOT LIBXML2_INCLUDE_DIR OR NOT LIBXML2_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      include_directories(${LIBXML2_INCLUDE_DIR})
      add_definitions(${LIBXML2_DEFINITIONS})
      link_libraries(${LIBXML2_LIBRARIES})
      add_definitions("-DHAVE_LIBXML2")
    endif()
  elseif (${libraryName} STREQUAL "xslt")
    find_package(LibXslt ${requiredVersion})
    if (NOT LIBXSLT_INCLUDE_DIR OR NOT LIBXSLT_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      include_directories(${LIBXSLT_INCLUDE_DIR})
      add_definitions(${LIBXSLT_DEFINITIONS})
      link_libraries(${LIBXSLT_LIBRARIES} ${LIBXSLT_EXSLT_LIBRARIES})
      add_definitions("-DHAVE_LIBXSLT")
    endif()
  elseif (${libraryName} STREQUAL "uodbc")
    find_package(LibUODBC ${requiredVersion})
    if (NOT LIBUODBC_INCLUDE_DIRS)
      HHVM_EXTENSION_INTERNAL_SET_FAILED_DEPENDENCY(${extensionID} ${dependencyName})
      return()
    endif()

    if (${addPaths})
      include_directories(LIBUODBC_INCLUDE_DIRS)
      add_definitions("-DHAVE_UODBC")
      add_definitions("-DHAVE_LIBUODBC")
    endif()
  else()
    message(FATAL_ERROR "Unknown library '${originalLibraryName}' as a dependency of the '${HHVM_EXTENSION_${extensionID}_PRETTY_NAME}' extension!")
  endif()
endfunction()
