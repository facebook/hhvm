/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#ifndef incl_HPHP_COMPILATION_FLAGS_H_
#define incl_HPHP_COMPILATION_FLAGS_H_

#ifndef INSTALL_PREFIX
# if defined(__APPLE__) || defined(__FreeBSD__) || \
     defined(__OpenBSD__) || defined(__NetBSD__)
#  define INSTALL_PREFIX "/usr/local"
# else
#  define INSTALL_PREFIX ""
# endif
#endif

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * This header converts some common preprocessor symbols into
 * compile-time constant booleans.
 *
 * This allows writing code that is checked for being able to compile
 * regardless of whether the preprocessor flag is on.  (Then we rely
 * on DCE to remove the cases that can't happen.)
 */

//////////////////////////////////////////////////////////////////////

const bool debug =
#ifdef DEBUG
  true
#else
  false
#endif
  ;

const bool packed_tv =
#ifdef PACKED_TV
  true
#else
  false
#endif
  ;

const bool memory_profiling =
#ifdef MEMORY_PROFILING
  true
#else
  false
#endif
  ;

//////////////////////////////////////////////////////////////////////

}

#endif
