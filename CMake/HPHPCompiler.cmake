if(CMAKE_COMPILER_IS_GNUCC)
	INCLUDE(CheckCSourceCompiles)
	CHECK_C_SOURCE_COMPILES("#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40600
#error Need GCC 4.6.0+
#endif
int main() { return 0; }" HAVE_GCC_46)

	if(NOT HAVE_GCC_46)
	        message(FATAL_ERROR "Need at least GCC 4.6")
	endif()

	CHECK_C_SOURCE_COMPILES("#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40700
#error Not GCC 4.7.0+
#endif
int main() { return 0; }" HAVE_GCC_47)

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
	set(GNUCC_UNINIT_OPT "")
	if(HAVE_GCC_47)
		set(GNUCC_UNINIT_OPT "-Wno-maybe-uninitialized")
	endif()
	set(GNUCC_LOCAL_TYPEDEF_OPT "")
	if(HAVE_GCC_48)
		set(GNUCC_LOCAL_TYPEDEF_OPT "-Wno-unused-local-typedefs")
	endif()
	set(CMAKE_C_FLAGS "-w")
	set(CMAKE_CXX_FLAGS "-fno-gcse -fno-omit-frame-pointer -ftemplate-depth-180 -Wall -Woverloaded-virtual -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names -Wno-error=array-bounds -Wno-error=switch -std=gnu++0x -Werror=format-security -Wno-unused-result -Wno-sign-compare -Wno-attributes ${GNUCC_UNINIT_OPT} ${GNUCC_LOCAL_TYPEDEF_OPT}")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS_RELEASE "-O3")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

