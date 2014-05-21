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
	set(CMAKE_C_FLAGS_RELEASE "-fast")
	set(CMAKE_CXX_FLAGS_RELEASE "-fast")
# using GCC
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
	  message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
	endif()

	# GCC 4.8+
	set(GNUCC_48_OPT "")
	if (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
		set(GNUCC_48_OPT "-Wno-unused-local-typedefs -fno-canonical-system-headers -Wno-deprecated-declarations")
	else()
		# Workaround for GCC bug
		add_definitions(-D_GLIBCXX_USE_NANOSLEEP)
	endif()

	# ARM64
	set(GNUCC_PLAT_OPT "")
	if(NOT IS_AARCH64)
		# TODO: This should really only be set on X86/X64
		set(GNUCC_PLAT_OPT "-mcrc32")
	endif()

	# Generic GCC flags and Optional flags
	set(CMAKE_C_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_C_FLAGS "-w")
	set(CMAKE_CXX_FLAGS "-Wall -std=gnu++11 -fno-gcse -fno-omit-frame-pointer -ftemplate-depth-180 -Woverloaded-virtual -Wno-deprecated -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names -Wno-error=array-bounds -Wno-error=switch -Werror=format-security -Wno-unused-result -Wno-sign-compare -Wno-attributes -Wno-maybe-uninitialized ${GNUCC_PLAT_OPT} ${GNUCC_48_OPT}")
# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	set(CMAKE_C_FLAGS "-no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -w")
	set(CMAKE_CXX_FLAGS "-no-ipo -fp-model precise -wd584 -wd1418 -wd1918 -wd383 -wd869 -wd981 -wd424 -wd1419 -wd444 -wd271 -wd2259 -wd1572 -wd1599 -wd82 -wd177 -wd593 -fno-omit-frame-pointer -ftemplate-depth-180 -Wall -Woverloaded-virtual -Wno-deprecated -w1 -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof -fno-operator-names")
# using Visual Studio C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# TODO
endif()
