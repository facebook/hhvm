/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_DECLARE_BOOST_TYPES_H_
#define incl_HPHP_DECLARE_BOOST_TYPES_H_

#include <memory>
#include <vector>
#include <set>
#include <list>

#include "hphp/util/hphp-raw-ptr.h"
#include "hphp/util/hash-map-typedefs.h"

//////////////////////////////////////////////////////////////////////

/*
 * We want to avoid this macro in new code.
 */

//////////////////////////////////////////////////////////////////////

#ifdef DECLARE_BOOST_TYPES
# undef DECLARE_BOOST_TYPES
#endif

#define DECLARE_BOOST_TYPES(classname)                                  \
  class classname;                                                      \
                                                                        \
  using classname ## Ptr      = std::shared_ptr<classname>;             \
  using classname ## RawPtr   = hphp_raw_ptr<classname>;                \
  using classname ## ConstPtr = std::shared_ptr<const classname>;       \

#define DECLARE_EXTENDED_BOOST_TYPES(classname)                     \
  DECLARE_BOOST_TYPES(classname)                                    \
  using StringTo ## classname ## PtrMap    = hphp_string_hash_map<  \
    classname ## Ptr,                                               \
    classname                                                       \
  >;                                                                \
  using StringTo ## classname ## PtrVecMap = hphp_string_hash_map<  \
    std::vector<classname ## Ptr>,                                  \
    classname                                                       \
  >;                                                                \

//////////////////////////////////////////////////////////////////////

#endif
