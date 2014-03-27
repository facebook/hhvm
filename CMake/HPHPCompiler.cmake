if(CMAKE_COMPILER_IS_GNUCC)
	INCLUDE(CheckCSourceCompiles)

	CHECK_C_SOURCE_COMPILES("#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40700
#error Not GCC 4.7.0+
#endif
int main() { return 0; }" HAVE_GCC_47)

	if (NOT HAVE_GCC_47)
		message(FATAL_ERROR "Need at least GCC 4.7")
	endif()

	CHECK_C_SOURCE_COMPILES("#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40800
#error Not GCC 4.8.0+
#endif
int main() { return 0; }" HAVE_GCC_48)

endif()

set(FREEBSD FALSE)
set(LINUX FALSE)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
	set(FREEBSD TRUE)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
	set(LINUX TRUE)
endif()

if($ENV{CXX} MATCHES "icpc")
	set(CMAKE_C_FLAGS "-no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -w")
	set(CMAKE_CXX_FLAGS "-no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -fno-omit-frame-pointer -ftemplate-depth-180 -Wall -Woverloaded-virtual -Wno-deprecated -w1 -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names")
else()
	set(GNUCC_48_OPT "")
	if(HAVE_GCC_48)
		set(GNUCC_48_OPT "-Wno-unused-local-typedefs -fno-canonical-system-headers -Wno-deprecated-declarations")
	else()
		# Workaround for GCC bug
		add_definitions(-D_GLIBCXX_USE_NANOSLEEP)
	endif()
	set(GNUCC_PLAT_OPT "")
	if(NOT IS_AARCH64)
		# TODO: This should really only be set on X86/X64
		set(GNUCC_PLAT_OPT "-mcrc32")
	endif()
	set(CMAKE_C_FLAGS "-w")
	set(CMAKE_CXX_FLAGS "-fno-gcse -fno-omit-frame-pointer -ftemplate-depth-180 -Wall -Woverloaded-virtual -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names -Wno-error=array-bounds -Wno-error=switch -std=gnu++11 -Werror=format-security -Wno-unused-result -Wno-sign-compare -Wno-attributes -Wno-maybe-uninitialized ${GNUCC_PLAT_OPT} ${GNUCC_48_OPT}")
endif()

if(${CMAKE_CXX_COMPILER} MATCHES ".*clang.*")
	set(CMAKE_CXX_FLAGS "-fno-gcse -fno-omit-frame-pointer -ftemplate-depth-180 -Wall -Woverloaded-virtual -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names -Wno-error=array-bounds -Wno-error=switch -std=gnu++11 -Werror=format-security -Wno-unused-result -Wno-sign-compare -Wno-attributes -Wno-maybe-uninitialized -Wno-mismatched-tags -Wno-unknown-warning-option -Wno-return-type-c-linkage -Qunused-arguments")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS_RELEASE "-O3")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()
