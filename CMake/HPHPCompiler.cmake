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
  # For unclear reasons, our detection for what crc32 intrinsics you have will
  # cause clang to ICE. Specifying a baseline here works around the issue.
  # (SSE4.2 has been available on processors for quite some time now.)
  set(LLVM_OPT "-msse4.2")
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

  if(CLANG_FORCE_LIBSTDCXX)
    set(CLANG_STDLIB "libstdc++")
  else()
    set(CLANG_STDLIB "libc++")
  endif()

  set(CMAKE_C_FLAGS_DEBUG            "-g")
  set(CMAKE_CXX_FLAGS_DEBUG          "-g")
  set(CMAKE_C_FLAGS_MINSIZEREL       "-Os -DNDEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE          "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${LLVM_OPT} -w")
  set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS} -std=gnu++11 -stdlib=${CLANG_STDLIB} -fno-omit-frame-pointer -Woverloaded-virtual -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names -Wno-error=array-bounds -Wno-error=switch -Werror=format-security -Wno-unused-result -Wno-sign-compare -Wno-attributes -Wno-maybe-uninitialized -Wno-mismatched-tags -Wno-unknown-warning-option -Wno-return-type-c-linkage -Qunused-arguments ${LLVM_OPT}")
# using GCC
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  set(GNUCC_OPT "")
  if(GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
    # FIXME: GCC 4.8+ regressions http://git.io/4r7VCQ
    set(GNUCC_OPT "${GNUCC_OPT} -ftrack-macro-expansion=0 -fno-builtin-memcmp")
  else()
     message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.8 or greater.")
  endif()

  # Fix problem with GCC 4.9, https://kb.isc.org/article/AA-01167
  if(GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
    set(GNUCC_OPT "${GNUCC_OPT} -fno-delete-null-pointer-checks")
  endif()

  if(GCC_VERSION VERSION_GREATER 5.0 OR GCC_VERSION VERSION_EQUAL 5.0)
    message(WARNING "HHVM is primarily tested on GCC 4.8 and GCC 4.9. Using other versions may produce unexpected results, or may not even build at all.")
  endif()

  if(GCC_VERSION VERSION_GREATER 5.1 OR GCC_VERSION VERSION_EQUAL 5.1)
    set(GNUCC_OPT "${GNUCC_OPT} -Wno-bool-compare -DFOLLY_HAVE_MALLOC_H")
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
  # -Og enables some optimizations, but makes debugging harder by optimizing
  # away some functions and locals. -O0 is more debuggable.
  # -O0-ggdb was reputed to cause gdb to crash (github #4450)
  set(CMAKE_C_FLAGS_DEBUG    "-O0 -g")
  set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")

  # Generic GCC flags and Optional flags
  set(CMAKE_C_FLAGS_MINSIZEREL       "-Os -DNDEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE          "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG -fno-gcse ${CMAKE_CXX_OMIT_LEAF_FRAME_POINTER} --param max-inline-insns-auto=100 --param early-inlining-insns=200 --param max-early-inliner-iterations=50 --param=inline-unit-growth=200 --param=large-unit-insns=10000")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
  set(CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS} ${GNUCC_OPT} -w")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++11 -ffunction-sections -fdata-sections -fno-gcse -fno-omit-frame-pointer -Woverloaded-virtual -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names -Wno-error=array-bounds -Wno-error=switch -Werror=format-security -Wno-unused-result -Wno-sign-compare -Wno-attributes -Wno-maybe-uninitialized -Wno-unused-local-typedefs -fno-canonical-system-headers -Wno-deprecated-declarations -Wno-unused-function -Wvla ${GNUCC_OPT} ${GNUCC_PLAT_OPT}")
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

  ############################################################
  # First we setup and account for the option sets.
  ############################################################

  set(MSVC_GENERAL_OPTIONS)
  set(MSVC_DISABLED_WARNINGS)
  set(MSVC_WARNINGS_AS_ERRORS)
  set(MSVC_ADDITIONAL_DEFINES)
  set(MSVC_EXE_LINKER_OPTIONS)
  set(MSVC_DEBUG_OPTIONS)
  set(MSVC_RELEASE_OPTIONS)
  set(MSVC_RELEASE_LINKER_OPTIONS)
  set(MSVC_DEBUG_EXE_LINKER_OPTIONS)
  set(MSVC_RELEASE_EXE_LINKER_OPTIONS)

  # Some addional configuration options.
  set(MSVC_ENABLE_ALL_WARNINGS ON CACHE BOOL "If enabled, pass /Wall to the compiler.")
  set(MSVC_ENABLE_DEBUG_INLINING ON CACHE BOOL "If enabled, enable inlining in the debug configuration. This allows /Zc:inline to be far more effective, resulting in hphp_runtime_static being ~450mb smaller.")
  set(MSVC_ENABLE_LTCG OFF CACHE BOOL "If enabled, use Link Time Code Generation for Release builds.")
  set(MSVC_ENABLE_PARALLEL_BUILD ON CACHE BOOL "If enabled, build multiple source files in parallel.")
  set(MSVC_ENABLE_PCH ON CACHE BOOL "If enabled, use precompiled headers to speed up the build.")
  set(MSVC_ENABLE_STATIC_ANALYSIS OFF CACHE BOOL "If enabled, do more complex static analysis and generate warnings appropriately.")
  set(MSVC_FAVORED_ARCHITECTURE "blend" CACHE STRING "One of 'blend', 'AMD64', 'INTEL64', or 'ATOM'. This tells the compiler to generate code optimized to run best on the specified architecture.")
  set(MSVC_NO_ASSERT_IN_DEBUG OFF CACHE BOOL "If enabled, don't do asserts in debug mode. The reduces the size of hphp_runtime_static by ~300mb.")

  # The general options passed:
  list(APPEND MSVC_GENERAL_OPTIONS
    "bigobj" # Support objects with > 65k sections. Needed for folly due to templates.
    "fp:precise" # Precise floating point model used in every other build, use it here as well.
    "EHa" # Enable both SEH and C++ Exceptions.
    "Oy-" # Disable elimination of stack frames.
    "Zc:inline" # Have the compiler eliminate unreferenced COMDAT functions and data before emitting the object file. This produces significantly less input to the linker, resulting in MUCH faster linking.
    "Zo" # Enable enhanced optimized debugging. Produces slightly larger pdb files, but the resulting optimized code is much much easier to debug.
  )

  # Enable all warnings if requested.
  if (MSVC_ENABLE_ALL_WARNINGS)
    list(APPEND MSVC_GENERAL_OPTIONS "Wall")
  endif()

  # Enable static analysis if requested.
  if (MSVC_ENABLE_STATIC_ANALYSIS)
    list(APPEND MSVC_GENERAL_OPTIONS "analyze")
  endif()

  # Enable multi-processor compilation if requested.
  if (MSVC_ENABLE_PARALLEL_BUILD)
    list(APPEND MSVC_GENERAL_OPTIONS "MP")
  endif()

  # Enable AVX2 codegen if available and requested.
  if (ENABLE_AVX2)
    list(APPEND MSVC_GENERAL_OPTIONS "arch:AVX2")
  endif()

  # Validate, and then add the favored architecture.
  if (NOT MSVC_FAVORED_ARCHITECTURE STREQUAL "blend" AND NOT MSVC_FAVORED_ARCHITECTURE STREQUAL "AMD64" AND NOT MSVC_FAVORED_ARCHITECTURE STREQUAL "INTEL64" AND NOT MSVC_FAVORED_ARCHITECTURE STREQUAL "ATOM")
    message(FATAL_ERROR "MSVC_FAVORED_ARCHITECTURE must be set to one of exactly, 'blend', 'AMD64', 'INTEL64', or 'ATOM'! Got '${MSVC_FAVORED_ARCHITECTURE}' instead!")
  endif()
  list(APPEND MSVC_GENERAL_OPTIONS "favor:${MSVC_FAVORED_ARCHITECTURE}")

  # The warnings that are disabled:
  list(APPEND MSVC_DISABLED_WARNINGS
    "4068" # Unknown pragma.
    "4091" # 'typedef' ignored on left of '' when no variable is declared.
    "4101" # Unused variables
    "4103" # Alignment changed after including header. This is needed because boost includes an ABI header that does some #pragma pack push/pop stuff, and we've passed our own packing
    "4146" # Unary minus applied to unsigned type, result still unsigned.
    "4250" # Function was inherited via dominance.
    "4800" # Values being forced to bool, this happens many places, and is a "performance warning".
  )

  if (MSVC_ENABLE_ALL_WARNINGS)
    # These warnings are disabled because we've
    # enabled all warnings. If all warnings are
    # not enabled, then these don't need to be
    # disabled.
    list(APPEND MSVC_DISABLED_WARNINGS
      "4061" # Enum value not handled by a case in a switch on an enum. This isn't very helpful because it is produced even if a default statement is present.
      "4100" # Unreferenced formal parameter.
      "4127" # Conditional expression is constant.
      "4131" # Old style declarator used. This is triggered by ext_bc's backend code.
      "4189" # Local variable is initialized but not referenced.
      "4191" # Unsafe type cast.
      "4200" # Non-standard extension, zero sized array.
      "4201" # Non-standard extension used: nameless struct/union.
      "4232" # Non-standard extension used: 'pCurrent': address of dllimport.
      "4245" # Implicit change from signed/unsigned when initializing.
      "4255" # Implicitly converting fucntion prototype from `()` to `(void)`.
      "4265" # Class has virtual functions, but destructor is not virtual.
      "4287" # Unsigned/negative constant mismatch.
      "4296" # '<' Expression is always false.
      "4315" # 'this' pointer for member may not be aligned to 8 bytes as expected by the constructor.
      "4324" # Structure was padded due to alignment specifier.
      "4355" # 'this' used in base member initializer list.
      "4365" # Signed/unsigned mismatch.
      "4371" # Layout of class may have changed due to fixes in packing.
      "4388" # Signed/unsigned mismatch on relative comparison operator.
      "4389" # Signed/unsigned mismatch on equality comparison operator.
      "4435" # Object layout under /vd2 will change due to virtual base.
      "4456" # Declaration of local hides previous definition of local by the same name.
      "4457" # Declaration of local hides function parameter.
      "4458" # Declaration of parameter hides class member.
      "4459" # Declaration of parameter hides global declaration.
      "4464" # Relative include path contains "..". This is triggered by the TBB headers.
      "4505" # Unreferenced local function has been removed. This is mostly the result of things not being needed under MSVC.
      "4514" # Unreferenced inline function has been removed. (caused by /Zc:inline)
      "4548" # Expression before comma has no effect. I wouldn't disable this normally, but malloc.h triggers this warning.
      "4555" # Expression has no effect; expected expression with side-effect. This is triggered by boost/variant.hpp.
      "4574" # ifdef'd macro was defined to 0.
      "4582" # Constructor is not implicitly called.
      "4583" # Destructor is not implicitly called.
      "4608" # Member has already been initialized by another union member initializer.
      "4619" # Invalid warning number used in #pragma warning.
      "4623" # Default constructor was implicitly defined as deleted.
      "4625" # Copy constructor was implicitly defined as deleted.
      "4626" # Assignment operator was implicitly defined as deleted.
      "4647" # __is_pod() has a different value in pervious versions of MSVC.
      "4668" # Macro was not defined, replacing with 0.
      "4701" # Potentially uninitialized local variable used.
      "4702" # Unreachable code.
      "4706" # Assignment within conditional expression.
      "4709" # Comma operator within array index expression. This currently just produces false-positives.
      "4710" # Function was not inlined.
      "4711" # Function was selected for automated inlining. This produces tens of thousands of warnings in release mode if you leave it enabled, which will completely break Visual Studio, so don't enable it.
      "4714" # Function marked as __forceinline not inlined.
      "4774" # Format string expected in argument is not a string literal.
      "4820" # Padding added after data member.
      "4917" # A GUID can only be associated with a class. This is triggered by some standard windows headers.
      "4946" # reinterpret_cast used between related types.
      "5026" # Move constructor was implicitly defined as deleted.
      "5027" # Move assignment operator was implicitly defined as deleted.
      "5031" # #pragma warning(pop): likely mismatch, popping warning state pushed in different file. This is needed because of how boost does things.
    )
  endif()

  if (MSVC_ENABLE_STATIC_ANALYSIS)
    # Warnings disabled for /analyze
    list(APPEND MSVC_DISABLED_WARNINGS
      "6001" # Using uninitialized memory. This is disabled because it is wrong 99% of the time.
      "6011" # Dereferencing potentially NULL pointer.
      "6031" # Return value ignored.
      "6235" # (<non-zero constant> || <expression>) is always a non-zero constant.
      "6237" # (<zero> && <expression>) is always zero. <expression> is never evaluated and may have side effects.
      "6239" # (<non-zero constant> && <expression>) always evaluates to the result of <expression>.
      "6240" # (<expression> && <non-zero constant>) always evaluates to the result of <expression>.
      "6246" # Local declaration hides declaration of same name in outer scope.
      "6248" # Setting a SECURITY_DESCRIPTOR's DACL to NULL will result in an unprotected object. This is done by one of the boost headers.
      "6255" # _alloca indicates failure by raising a stack overflow exception.
      "6262" # Function uses more than x bytes of stack space.
      "6271" # Extra parameter passed to format function. The analysis pass doesn't recognize %j or %z, even though the runtime does.
      "6285" # (<non-zero constant> || <non-zero constant>) is always true.
      "6297" # 32-bit value is shifted then cast to 64-bits. The places this occurs never use more than 32 bits.
      "6308" # Realloc might return null pointer: assigning null pointer to '<name>', which is passed as an argument to 'realloc', will cause the original memory to leak.
      "6326" # Potential comparison of a constant with another constant.
      "6330" # Unsigned/signed mismatch when passed as a parameter.
      "6340" # Mismatch on sign when passed as format string value.
      "6387" # '<value>' could be '0': This does not adhere to the specification for a function.
      "28182" # Dereferencing NULL pointer. '<value>' contains the same NULL value as '<expression>'.
      "28251" # Inconsistent annotation for function. This is because we only annotate the declaration and not the definition.
      "28278" # Function appears with no prototype in scope.
    )
  endif()

  # Warnings disabled to keep it quiet for now,
  # most of these should be reviewed and re-enabled:
  list(APPEND MSVC_DISABLED_WARNINGS
    "4005" # Macro redefinition
    "4018" # Signed/unsigned mismatch.
    "4242" # Possible loss of data when returning a value.
    "4244" # Implicit truncation of data.
    "4267" # Implicit truncation of data. This really shouldn't be disabled.
    "4291" # No matching destructor found.
    "4302" # Pointer casting size difference
    "4311" # Pointer casting size difference
    "4312" # Pointer casting size difference
    "4477" # Parameter to a formatting function isn't the same type as was passed in the format string.
    "4624" # Destructor was implicitly undefined.
    "4804" # Unsafe use of type 'bool' in operation. (comparing if bool is <=> scalar)
    "4805" # Unsafe mix of scalar type and type 'bool' in operation. (comparing if bool is == scalar)
  )

  # Warnings to treat as errors:
  list(APPEND MSVC_WARNINGS_AS_ERRORS
    "4099" # Mixed use of struct and class on same type names. This was absolutely everywhere, and can cause errors at link-time if not fixed.
    "4129" # Unknown escape sequence. This is usually caused by incorrect escaping.
    "4566" # Character cannot be represented in current charset. This is remidied by prefixing string with "u8".
  )

  # And the extra defines:
  list(APPEND MSVC_ADDITIONAL_DEFINES
    "NOMINMAX" # This is needed because, for some absurd reason, one of the windows headers tries to define "min" and "max" as macros, which messes up most uses of std::numeric_limits.
    "_CRT_NONSTDC_NO_WARNINGS" # Don't deprecate posix names of functions.
    "_CRT_SECURE_NO_WARNINGS" # Don't deprecate the non _s versions of various standard library functions, because safety is for chumps.
    "_SCL_SECURE_NO_WARNINGS" # Don't deprecate the non _s versions of various standard library functions, because safety is for chumps.
    "_WINSOCK_DEPRECATED_NO_WARNINGS" # Don't deprecate pieces of winsock
    "YY_NO_UNISTD_H" # Because MSVC doesn't have unistd.h, which is requested by the YACC generated code.
  )

  # The options passed to the linker for EXE targets:
  list(APPEND MSVC_EXE_LINKER_OPTIONS
    "BASE:0x10000" # Base the program at just over 64k in memory, to play nice with the JIT.
    "DYNAMICBASE:NO" # Don't randomize the base address.
    "FIXED" # The program can only be loaded at its preferred base address.
    "STACK:8388608,8388608" # Set the stack reserve,commit to 8mb. Reserve should probably be higher.
    "time" # Output some timing information about the link.
  )

  # The options to pass to the compiler for debug builds:
  list(APPEND MSVC_DEBUG_OPTIONS
    "Gy-" # Disable function level linking.
    "GF-" # Disable string pooling.
  )

  # Add /Ob2 if allowing inlining in debug mode:
  if (MSVC_ENABLE_DEBUG_INLINING)
    list(APPEND MSVC_DEBUG_OPTIONS "Ob2")
  endif()

  # The options to pass to the compiler for release builds:
  list(APPEND MSVC_RELEASE_OPTIONS
    "GF" # Enable string pooling. (this is enabled by default by the optimization level, but we enable it here for clarity)
    "Gw" # Optimize global data. (-fdata-sections)
    "Gy" # Enable function level linking. (-ffunction-sections)
    "Qpar" # Enable parallel code generation. HHVM itself doesn't currently use this, but it's dependencies, TBB for instance, might, so enable it.
    "Oi" # Enable intrinsic functions.
    "Ot" # Favor fast code.
  )

  # Add /GL to the compiler, and /LTCG to the linker
  # if link time code generation is enabled.
  if (MSVC_ENABLE_LTCG)
    list(APPEND MSVC_RELEASE_OPTIONS "GL")
    list(APPEND MSVC_RELEASE_LINKER_OPTIONS "LTCG")
  endif()

  # The options to pass to the linker for debug builds for EXE targets:
  list(APPEND MSVC_DEBUG_EXE_LINKER_OPTIONS
    "OPT:NOREF" # No unreferenced data elimination. (well, mostly)
    "OPT:NOICF" # No Identical COMDAT folding.
  )

  # The options to pass to the linker for release builds for EXE targets:
  list(APPEND MSVC_RELEASE_EXE_LINKER_OPTIONS
    "OPT:REF" # Remove unreferenced functions and data.
    "OPT:ICF" # Identical COMDAT folding.
  )

  ############################################################
  # Now we need to adjust a couple of the default option sets.
  ############################################################

  # We need the static runtime.
  foreach(flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if (${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()

  # In order for /Zc:inline, which speeds up the build significantly, to work
  # we need to remove the /Ob0 parameter that CMake adds by default, because that
  # would normally disable all inlining.
  foreach(flag_var CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG)
    if (${flag_var} MATCHES "/Ob0")
      string(REGEX REPLACE "/Ob0" "" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()

  # Ignore a warning about an object file not defining any symbols,
  # these are known, and we don't care.
  set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4221")

  ############################################################
  # And finally, we can set all the flags we've built up.
  ############################################################

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

  foreach(opt ${MSVC_EXE_LINKER_OPTIONS})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /${opt}")
  endforeach()

  foreach(opt ${MSVC_RELEASE_LINKER_OPTIONS})
    foreach(flag_var
        CMAKE_EXE_LINKER_FLAGS_RELEASE CMAKE_EXE_LINKER_FLAGS_MINSIZEREL CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO
        CMAKE_SHARED_LINKER_FLAGS_RELEASE CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO
        CMAKE_STATIC_LINKER_FLAGS_RELEASE CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO)
      set(${flag_var} "${${flag_var}} /${opt}")
    endforeach()
  endforeach()

  foreach(opt ${MSVC_DEBUG_OPTIONS})
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /${opt}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /${opt}")
  endforeach()

  foreach(opt ${MSVC_RELEASE_OPTIONS})
    foreach(flag_var
        CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      set(${flag_var} "${${flag_var}} /${opt}")
    endforeach()
  endforeach()

  foreach(opt ${MSVC_DEBUG_EXE_LINKER_OPTIONS})
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /${opt}")
  endforeach()

  foreach(opt ${MSVC_RELEASE_EXE_LINKER_OPTIONS})
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /${opt}")
    set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} /${opt}")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /${opt}")
  endforeach()
else()
  message("Warning: unknown/unsupported compiler, things may go wrong")
endif()
