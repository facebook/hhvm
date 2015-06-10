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
  # march=native is a rather ham-fisted approach to try to work around a clang
  # ICE where it can't figure out what to do when we request to use a crc32
  # intrinsic. This might affect portability of binaries, and should probably be
  # revisited.
  set(LLVM_OPT "-march=native")
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} --version COMMAND head -1
    OUTPUT_VARIABLE _clang_version_info)
  string(
    REGEX MATCH "(clang version|based on LLVM) ([0-9]\\.[0-9]\\.?[0-9]?)"
    CLANG_VERSION "${_clang_version_info}")
  # Enabled GCC/LLVM stack-smashing protection
  if(ENABLE_SSP)
    if(CLANG_VERSION VERSION_GREATER 3.6 OR CLANG_VERSION VERSION_EQUAL 3.6)
      set(LLVM_OPT "${LLVM_OPT} -fstack-protector-strong")
    else()
      set(LLVM_OPT "${LLVM_OPT} -fstack-protector")
    endif()
    set(LLVM_OPT "${LLVM_OPT} --param=ssp-buffer-size=4 -pie -fPIC")
  endif()
  set(CMAKE_C_FLAGS_DEBUG            "-g")
  set(CMAKE_CXX_FLAGS_DEBUG          "-g")
  set(CMAKE_C_FLAGS_MINSIZEREL       "-Os -DNDEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE          "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${LLVM_OPT} -w")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++11 -stdlib=libc++ -fno-omit-frame-pointer -Woverloaded-virtual -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names -Wno-error=array-bounds -Wno-error=switch -Werror=format-security -Wno-unused-result -Wno-sign-compare -Wno-attributes -Wno-maybe-uninitialized -Wno-mismatched-tags -Wno-unknown-warning-option -Wno-return-type-c-linkage -Qunused-arguments ${LLVM_OPT}")
# using GCC
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  set(GNUCC_OPT "")
  if(GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
    # FIXME: GCC 4.8+ regressions http://git.io/4r7VCQ
    set(GNUCC_OPT "${GNUCC_OPT} -ftrack-macro-expansion=0 -fno-builtin-memcmp")
    # Fix problem with GCC 4.9, https://kb.isc.org/article/AA-01167
    if(GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
      set(GNUCC_OPT "${GNUCC_OPT} -fno-delete-null-pointer-checks")
    endif()
  else()
     message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.8 or greater.")
  endif()

  # Enabled GCC/LLVM stack-smashing protection
  if(ENABLE_SSP)
    if(GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
      if(LINUX)
        # https://isisblogs.poly.edu/2011/06/01/relro-relocation-read-only/
        set(GNUCC_OPT "${GNUCC_OPT} -Wl,-z,relro,-z,now")
      endif()
      if(GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
        set(GNUCC_OPT "${GNUCC_OPT} -fstack-protector-strong")
      endif()
    else()
      set(GNUCC_OPT "${GNUCC_OPT} -fstack-protector")
    endif()
    set(GNUCC_OPT "${GNUCC_OPT} -pie -fPIC --param=ssp-buffer-size=4")
  endif()

  # X64
  set(GNUCC_PLAT_OPT "")
  if(IS_X64)
    set(GNUCC_PLAT_OPT "-mcrc32")
  endif()

  # PPC64
  if(NOT IS_PPC64)
    set(CMAKE_CXX_OMIT_LEAF_FRAME_POINTER "-momit-leaf-frame-pointer")
  endif()

  # No optimizations for debug builds.
  set(CMAKE_C_FLAGS_DEBUG    "-Og -g")
  set(CMAKE_CXX_FLAGS_DEBUG  "-Og -g")

  # Generic GCC flags and Optional flags
  set(CMAKE_C_FLAGS_MINSIZEREL       "-Os -DNDEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE          "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG -fno-gcse ${CMAKE_CXX_OMIT_LEAF_FRAME_POINTER} --param max-inline-insns-auto=100 --param early-inlining-insns=200 --param max-early-inliner-iterations=50")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
  set(CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS} ${GNUCC_OPT} -w")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++11 -ffunction-sections -fdata-sections -fno-gcse -fno-omit-frame-pointer -Woverloaded-virtual -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names -Wno-error=array-bounds -Wno-error=switch -Werror=format-security -Wno-unused-result -Wno-sign-compare -Wno-attributes -Wno-maybe-uninitialized -Wno-unused-local-typedefs -fno-canonical-system-headers -Wno-deprecated-declarations -Wno-unused-function ${GNUCC_OPT} ${GNUCC_PLAT_OPT}")
  if(STATIC_CXX_LIB)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
  endif()
  if(ENABLE_AVX2)
    set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -mavx2 -march=core-avx2")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mavx2 -march=core-avx2")
    set(CMAKE_ASM_FLAGS  "${CMAKE_ASM_FLAGS} -mavx2 -march=core-avx2")
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
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -w")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -fno-omit-frame-pointer -Wall -Woverloaded-virtual -Wno-deprecated -w1 -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names")
# using Visual Studio C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  message(WARNING "MSVC support is VERY experimental. It will likely not compile, and is intended for the utterly insane.")
  # Accounting for each of the warnings disabled:
  # 4068: Unknown pragma.
  # 4099: Mixed use of struct and class on same type names. This is absolutely everywhere.
  # 4101: Unused variables
  # 4146: Unary minus applied to unsigned type, result still unsigned.
  # 4267: Implicit truncation of data. This really shouldn't be disabled. (and isn't at the moment)
  # 4800: Values being forced to bool, this happens many places, and is a "performance warning".
  #
  # And the extra defines:
  # NOMINMAX: This is needed because, for some absurd reason, one of the windows headers tries to define "min" and "max" as macros, which messes up most uses of std::numeric_limits.
  # _CRT_SECURE_NO_WARNINGS: Don't deprecate the non _s versions of various standard library functions, because safety is for chumps.
  #
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /fp:precise /wd4068 /wd4099 /wd4101 /wd4146 /wd4800 /D NOMINMAX /D _CRT_SECURE_NO_WARNINGS")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:precise /wd4068 /wd4099 /wd4101 /wd4146 /wd4800 /D NOMINMAX /D _CRT_SECURE_NO_WARNINGS")
else()
  message("Warning: unknown/unsupported compiler, things may go wrong")
endif()
