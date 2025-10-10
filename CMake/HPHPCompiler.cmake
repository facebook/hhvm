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

# Do this until cmake has a define for ARMv8
INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
#ifndef __x86_64__
#error Not x64
#endif
int main() { return 0; }" IS_X64)

CHECK_CXX_SOURCE_COMPILES("
#ifndef __AARCH64EL__
#error Not ARMv8
#endif
int main() { return 0; }" IS_AARCH64)

# using Clang or GCC
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  # Warnings to disable by name, -Wno-${name}
  set(DISABLED_NAMED_WARNINGS)
  list(APPEND DISABLED_NAMED_WARNINGS
    "error=array-bounds"
    "error=switch"
    "attributes"
    "deprecated"
    "invalid-offsetof"
    "register"
    "sign-compare"
    "strict-aliasing"
    "unused-function"
    "unused-local-typedefs"
    "unused-result"
    "write-strings"
  )

  # Warnings to disable by name when building C code.
  set(DISABLED_C_NAMED_WARNINGS)
  list(APPEND DISABLED_C_NAMED_WARNINGS
    "missing-field-initializers"
    "sign-compare"
  )

  # General options to pass to both C & C++ compilers
  set(GENERAL_OPTIONS)

  # General options to pass to the C++ compiler
  set(GENERAL_CXX_OPTIONS)
  list(APPEND GENERAL_CXX_OPTIONS
    "std=gnu++1z"
    "fno-omit-frame-pointer"
    "Wall"
    "Werror=format-security"
    "Wno-unused-variable"
    "Wno-unused-value"
    "Wno-comment"
    "Wno-class-memaccess"
    "Wno-adress"
    "Wno-error=stringop-overflow"
  )

  # Options to pass for debug mode to the C++ compiler
  set(DEBUG_CXX_OPTIONS)

  # Options to pass for release mode to the C++ compiler
  set(RELEASE_CXX_OPTIONS)

  # Suboption of -g in debug mode
  set(GDB_SUBOPTION)

  # Enable GCC/LLVM stack-smashing protection
  if(ENABLE_HARDENING)
    list(APPEND GENERAL_OPTIONS
      # Enable stack protection and stack-clash protection.
      # This needs two dashes in the name, so put one here.
      "-param=ssp-buffer-size=4"
      "fstack-protector-strong"
      "fstack-clash-protection"

      # Use hardened equivalents of various glibc functions
      # to guard against buffer overflows.
      "D_FORTIFY_SOURCE=3"

      # https://isisblogs.poly.edu/2011/06/01/relro-relocation-read-only/
      "Wl,-z,relro,-z,now"
      # Mark stack as non-executable.
      "Wl,-z,noexecstack"
      # Separate ELF code into its own segment.
      "Wl,-z,separate-code"
    )

    # Enable control-flow / branch protection.
    if (IS_X64)
      list(APPEND GENERAL_OPTIONS "fcf-protection")
    elseif (IS_AARCH64)
      list(APPEND GENERAL_OPTIONS "mbranch-protection=standard")
    endif()

    # Enable C++ standard library assertions.
    if (CLANG_FORCE_LIBCPP)
      list(APPEND GENERAL_CXX_OPTIONS "D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE")
    else()
      list(APPEND GENERAL_CXX_OPTIONS "D_GLIBCXX_ASSERTIONS")
    endif()
  endif()

  if (ENABLE_PIE)
    list(APPEND GENERAL_OPTIONS "pie" "fPIC")
  else()
    list(APPEND GENERAL_OPTIONS "no-pie")
  endif()

  if (IS_X64)
    list(APPEND GENERAL_CXX_OPTIONS "march=x86-64-v3")
    set(CMAKE_ASM_FLAGS  "${CMAKE_ASM_FLAGS} -march=x86-64-v3")
  endif()

  if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang") # using Clang
    list(APPEND GENERAL_CXX_OPTIONS
      "Qunused-arguments"
    )
    list(APPEND DISABLED_C_NAMED_WARNINGS
      "unused-command-line-argument"
    )
    list(APPEND DISABLED_NAMED_WARNINGS
      "return-type-c-linkage"
      "unknown-warning-option"
      "unused-command-line-argument"
    )

    if(CLANG_FORCE_LIBCPP)
      list(APPEND GENERAL_CXX_OPTIONS "stdlib=libc++")
    endif()
  else() # using GCC
    list(APPEND DISABLED_NAMED_WARNINGS
      "deprecated-declarations"
      "maybe-uninitialized"
      "bool-compare"
    )
    list(APPEND DISABLED_C_NAMED_WARNINGS
      "maybe-uninitialized"
      "old-style-declaration"
    )
    list(APPEND GENERAL_OPTIONS
      "ffunction-sections"
      "fno-delete-null-pointer-checks"
      "DFOLLY_HAVE_MALLOC_H"
    )
    list(APPEND GENERAL_CXX_OPTIONS
      "fdata-sections"
      "fno-gcse"
      "fno-canonical-system-headers"
      "Wvla"
      "Wno-misleading-indentation"
    )
    list(APPEND RELEASE_CXX_OPTIONS
      "-param max-inline-insns-auto=100"
      "-param early-inlining-insns=200"
      "-param max-early-inliner-iterations=50"
      "-param=inline-unit-growth=200"
      "-param=large-unit-insns=10000"
    )

    # X64
    if(IS_X64)
      list(APPEND GENERAL_CXX_OPTIONS "mcrc32")
    endif()

    # ARM64
    if(IS_AARCH64)
      # Force char type to be signed, which is not the case on aarch64.
      list(APPEND GENERAL_OPTIONS "fsigned-char")

      # If a CPU was specified, build a -mcpu option for the compiler.
      set(AARCH64_TARGET_CPU "" CACHE STRING "CPU to tell gcc to optimize for (-mcpu)")
      if(AARCH64_TARGET_CPU)
        list(APPEND GENERAL_OPTIONS "mcpu=${AARCH64_TARGET_CPU}")
        set(CMAKE_ASM_FLAGS  "${CMAKE_ASM_FLAGS} -mcpu=${AARCH64_TARGET_CPU}")

        # Make sure GCC is not using the fix for errata 843419. This change
        # interferes with the gold linker. Note that GCC applies this fix
        # even if you specify an mcpu other than cortex-a53, which is why
        # it's explicitly being disabled here for any cpu other than
        # cortex-a53. If you're running a newer pass of the cortex-a53, then
        # you can likely disable this fix with the following flag too. YMMV
        if(NOT ${AARCH64_TARGET_CPU} STREQUAL "cortex-a53")
          list(APPEND GENERAL_OPTIONS "mno-fix-cortex-a53-843419")
        endif()
      endif()
    endif()

    if(STATIC_CXX_LIB)
      set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
    endif()

    if (ENABLE_SPLIT_DWARF)
      set(GDB_SUBOPTION "split-dwarf")
    endif()
  endif()

  # No optimizations for debug builds.
  # -Og enables some optimizations, but makes debugging harder by optimizing
  # away some functions and locals. -O0 is more debuggable.
  # -O0-ggdb was reputed to cause gdb to crash (github #4450)
  set(CMAKE_C_FLAGS_DEBUG            "-O0 -g${GDB_SUBOPTION}")
  set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g${GDB_SUBOPTION}")
  set(CMAKE_C_FLAGS_DEBUGOPT         "-O2 -g${GDB_SUBOPTION}")
  set(CMAKE_CXX_FLAGS_DEBUGOPT       "-O2 -g${GDB_SUBOPTION}")
  set(CMAKE_C_FLAGS_MINSIZEREL       "-Os -DNDEBUG")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
  set(CMAKE_C_FLAGS_RELEASE          "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g${GDB_SUBOPTION} -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g${GDB_SUBOPTION} -DNDEBUG")
  set(CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS} -W -Werror=implicit-function-declaration")

  mark_as_advanced(CMAKE_C_FLAGS_DEBUGOPT CMAKE_CXX_FLAGS_DEBUGOPT)

  foreach(opt ${DISABLED_NAMED_WARNINGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-${opt}")
  endforeach()

  foreach(opt ${DISABLED_C_NAMED_WARNINGS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-${opt}")
  endforeach()

  foreach(opt ${GENERAL_OPTIONS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -${opt}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -${opt}")
  endforeach()

  foreach(opt ${GENERAL_CXX_OPTIONS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -${opt}")
  endforeach()

  foreach(opt ${DEBUG_CXX_OPTIONS})
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -${opt}")
  endforeach()

  foreach(opt ${RELEASE_CXX_OPTIONS})
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -${opt}")
  endforeach()
else()
  message("Warning: unknown/unsupported compiler, things may go wrong")
endif()

include(ThinArchives)
