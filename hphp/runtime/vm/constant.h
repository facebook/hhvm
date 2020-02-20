/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_CONSTANT_H_
#define incl_HPHP_CONSTANT_H_

#include <ostream>

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * This is the runtime representation of a constant.
 */
struct Constant {
  LowStringPtr name;
  TypedValue val;
  Attr attrs;

  template<class SerDe>
  void serde(SerDe& sd) {
    sd(val)
      (attrs);
  }

  void prettyPrint(std::ostream& out) const;

  static const StringData* nameFromFuncName(const StringData* func_name);
  static const StringData* funcNameFromName(const StringData* name);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
