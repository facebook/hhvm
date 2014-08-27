set(FREEBSD FALSE)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
  set(FREEBSD TRUE)
endif()
set(LINUX FALSE)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(LINUX TRUE)
endif()

# using Clang
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # TODO: Fix Folly ad change to -std=c++11 (ISO C++11), GNU_GCC version enable flags: -ffast-math
  set(CMAKE_CXX_FLAGS " -Wall -std=gnu++11 -stdlib=libc++ -fno-gcse -fno-omit-frame-pointer -ftemplate-depth-180 -Woverloaded-virtual -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names -Wno-error=array-bounds -Wno-error=switch -Werror=format-security -Wno-unused-result -Wno-sign-compare -Wno-attributes -Wno-maybe-uninitialized -Wno-mismatched-tags -Wno-unknown-warning-option -Wno-return-type-c-linkage -Qunused-arguments")
  # CMAKE_BUILD_TYPE: http://www.cmake.org/Wiki/CMake_Useful_Variables#Compilers_and_Tools
  set(CMAKE_C_FLAGS_DEBUG            "-g")
  set(CMAKE_CXX_FLAGS_DEBUG          "-g")
  set(CMAKE_C_FLAGS_MINSIZEREL       "-Os -DNDEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE          "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
# using GCC
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  set(GNUCC49_OPT "")
  if (NOT (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8))
    message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.8 or greater.")
  elseif(GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
    # fix problem with GCC 4.9, Changes in gcc Code Optimization Can Cause a Crash
    # https://kb.isc.org/article/AA-01167
    set(GNUCC49_OPT "-fno-delete-null-pointer-checks")
  endif()

  # ARM64
  set(GNUCC_PLAT_OPT "")
  if(NOT IS_AARCH64)
    # TODO: This should really only be set on X86/X64
    set(GNUCC_PLAT_OPT "-mcrc32")
  endif()

  # No optimizations for debug builds.
  set(CMAKE_C_FLAGS_DEBUG    "-O0 -ggdb")
  set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -ggdb")

  # Generic GCC flags and Optional flags
  set(CMAKE_C_FLAGS_MINSIZEREL       "-Os -DNDEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE          "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
  set(CMAKE_C_FLAGS                  "${GNUCC49_OPT} -w")
  set(CMAKE_CXX_FLAGS "-Wall -std=gnu++11 -fno-gcse -fno-omit-frame-pointer -ftemplate-depth-180 -Woverloaded-virtual -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names -Wno-error=array-bounds -Wno-error=switch -Werror=format-security -Wno-unused-result -Wno-sign-compare -Wno-attributes -Wno-maybe-uninitialized -Wno-unused-local-typedefs -fno-canonical-system-headers -Wno-deprecated-declarations ${GNUCC49_OPT} ${GNUCC_PLAT_OPT}")
  if(STATIC_CXX_LIB)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
  endif()
  if(CYGWIN)
  # in debug mode large files can overflow pe/coff sections
  # this switches binutils to use the pe+ format
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wa,-mbig-obj")
    # stack limit is set at compile time on windows
    # code expects a minimum of 8 * 1024 * 1024 + 8 for a buffer
    # the default is 2 mb
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--stack,8388616")
  endif()
# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_C_FLAGS   "-no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -w")
  set(CMAKE_CXX_FLAGS "-no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -fno-omit-frame-pointer -ftemplate-depth-180 -Wall -Woverloaded-virtual -Wno-deprecated -w1 -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names")
# using Visual Studio C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # TODO
endif()
