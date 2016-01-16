#ifndef incl_HHVM_VERSION_H
#define incl_HHVM_VERSION_H

#include <boost/preprocessor/stringize.hpp>

/* cmake -DHHVM_VERSION_OVERRIDE=3.12.0-dev .
 * Allows packaging scripts to update the reported
 * version without amending a commit to change this file
 *
 * See: CMake/HHVMVersion.cmake
 */
#ifndef HHVM_VERSION_OVERRIDE
# define HHVM_VERSION_MAJOR 3
# define HHVM_VERSION_MINOR 12
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

/* Human readable version string (e.g. "3.5.0-dev") */
#define HHVM_VERSION \
  (BOOST_PP_STRINGIZE(HHVM_VERSION_MAJOR) "." \
   BOOST_PP_STRINGIZE(HHVM_VERSION_MINOR) "." \
   BOOST_PP_STRINGIZE(HHVM_VERSION_PATCH) HHVM_VERSION_SUFFIX)

#endif // incl_HHVM_VERSION_H
