/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_AS_SHARED_H_
#define incl_HPHP_AS_SHARED_H_

#include <string>
#include "folly/Optional.h"
#include "hphp/runtime/base/types.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * This header contains routines shared between as.cpp and disas.cpp.
 * It shouldn't be included by anything else.
 */

//////////////////////////////////////////////////////////////////////

/*
 * Attribute bits mean different things depending on context.  This
 * just enumerates the contexts the (dis)assembler cares about.
 */
enum class AttrContext {
  Class         = 0x1,
  Func          = 0x2,
  Prop          = 0x4,
  TraitImport   = 0x8,
};

/*
 * Convert an attr to a string of space-separated attribute names, for
 * a given context.
 */
std::string attrs_to_string(AttrContext, Attr);

/*
 * Convert a string containing a single attribute name into an Attr,
 * for a given context.
 *
 * Returns folly::none if the string doesn't name a known attribute.
 */
folly::Optional<Attr> string_to_attr(AttrContext, const std::string&);

//////////////////////////////////////////////////////////////////////

}

#endif
