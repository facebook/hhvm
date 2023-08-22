#pragma once

// This file needs to be valid C, not just C++

/* cmake -DHHVM_VERSION_OVERRIDE=3.12.0-dev .
 * Allows packaging scripts to update the reported
 * version without amending a commit to change this file
 *
 * See: CMake/HHVMVersion.cmake
 */
#ifndef HHVM_VERSION_OVERRIDE
# define HHVM_VERSION_MAJOR 6
# define HHVM_VERSION_MINOR 79
# define HHVM_VERSION_PATCH 0
# define HHVM_VERSION_SUFFIX "-dev"
#endif

/* HHVM_VERSION_ID minus the patch number
 * APIs should remain stable while this number is constant
 */
#define HHVM_VERSION_BRANCH ((HHVM_VERSION_MAJOR << 16) | \
                             (HHVM_VERSION_MINOR <<  8))

/* Specific HHVM release */
#define HHVM_VERSION_ID (HHVM_VERSION_BRANCH | HHVM_VERSION_PATCH)

#define HHVM_VERSION_STRINGIFY_HELPER(x) #x
#define HHVM_VERSION_STRINGIFY(x) HHVM_VERSION_STRINGIFY_HELPER(x)

/* Human readable version string (e.g. "3.5.0-dev") */
#define HHVM_VERSION_C_STRING_LITERALS \
  HHVM_VERSION_STRINGIFY(HHVM_VERSION_MAJOR) "." \
  HHVM_VERSION_STRINGIFY(HHVM_VERSION_MINOR) "." \
  HHVM_VERSION_STRINGIFY(HHVM_VERSION_PATCH) HHVM_VERSION_SUFFIX
#define HHVM_VERSION (HHVM_VERSION_C_STRING_LITERALS)

