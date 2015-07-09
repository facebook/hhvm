set(FREEBSD FALSE)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
  set(FREEBSD TRUE)
endif()
set(LINUX FALSE)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(LINUX TRUE)
endif()
set(DARWIN FALSE)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(DARWIN TRUE)
endif()
set(WINDOWS FALSE)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  set(WINDOWS TRUE)
endif()

# using Clang
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
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
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
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
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -w")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -fno-omit-frame-pointer -Wall -Woverloaded-virtual -Wno-deprecated -w1 -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names")
# using Visual Studio C++
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  message(WARNING "MSVC support is VERY experimental. It will likely not compile, and is intended for the utterly insane.")
  set(MSVC_GENERAL_OPTIONS)
  set(MSVC_DISABLED_WARNINGS)
  set(MSVC_WARNINGS_AS_ERRORS)
  set(MSVC_ADDITIONAL_DEFINES)
  
  # The general options passed:
  list(APPEND MSVC_GENERAL_OPTIONS
    "fp:precise" # Precise floating point model used in every other build, use it here as well.
    "Oy-" # Disable elimination of stack frames.
    "Zp4" # Set the default packing to 4 to be the same as GCC.
  )
  
  # The warnings that are disabled.
  list(APPEND MSVC_DISABLED_WARNINGS
    "4068" # Unknown pragma.
    "4091" # 'typedef' ignored on left of '' when no variable is declared.
    "4099" # Mixed use of struct and class on same type names. This is absolutely everywhere.
    "4101" # Unused variables
    "4103" # Alignment changed after including header. This is needed because boost includes an ABI header that does some #pragma pack push/pop stuff, and we've passed our own packing
    "4146" # Unary minus applied to unsigned type, result still unsigned.
    "4800" # Values being forced to bool, this happens many places, and is a "performance warning".
  )
  
  # Warnings disabled to keep it quiet for now,
  # most of these should be reviewed and re-enabled:
  list(APPEND MSVC_DISABLED_WARNINGS
    "4018" # Signed/unsigned mismatch.
    "4200" # Non-standard extension, zero sized array.
    "4244" # Implicit truncation of data.
    "4267" # Implicit truncation of data. This really shouldn't be disabled.
    "4291" # No matching destructor found.
    "4624" # Destructor was implicitly undefined.
    "4804" # Unsafe use of type 'bool' in operation. (comparing if bool is <=> scalar)
    "4805" # Unsafe mix of scalar type and type 'bool' in operation. (comparing if bool is == scalar)
  )
  
  # Warnings to treat as errors:
  list(APPEND MSVC_WARNINGS_AS_ERRORS
    "4129" # Unknown escape sequence. This is usually caused by incorrect escaping.
    "4566" # Character cannot be represented in current charset. This is remidied by prefixing string with "u8".
  )
  
  # And the extra defines:
  list(APPEND MSVC_ADDITIONAL_DEFINES
    "NOMINMAX" # This is needed because, for some absurd reason, one of the windows headers tries to define "min" and "max" as macros, which messes up most uses of std::numeric_limits.
    "_CRT_NONSTDC_NO_WARNINGS" # Don't deprecate posix names of functions.
    "_CRT_SECURE_NO_WARNINGS" # Don't deprecate the non _s versions of various standard library functions, because safety is for chumps.
    "_SCL_SECURE_NO_WARNINGS" # Don't deprecate the non _s versions of various standard library functions, because safety is for chumps.
    "YY_NO_UNISTD_H" # Because MSVC doesn't have unistd.h, which is requested by the YACC generated code.
  )
  
  foreach(opt ${MSVC_GENERAL_OPTIONS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /${opt}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /${opt}")
  endforeach()
  
  foreach(opt ${MSVC_DISABLED_WARNINGS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd${opt}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd${opt}")
  endforeach()
  
  foreach(opt ${MSVC_WARNINGS_AS_ERRORS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /we${opt}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /we${opt}")
  endforeach()
  
  foreach(opt ${MSVC_ADDITIONAL_DEFINES})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D ${opt}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D ${opt}")
  endforeach()
else()
  message("Warning: unknown/unsupported compiler, things may go wrong")
endif()
