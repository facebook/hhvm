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

#pragma once

#include <ostream>

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/user-attributes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * This is the runtime representation of a module.
 */
struct Module {
  LowStringPtr name;
  int line0; // start line number on the src file
  int line1; // end line number on the src file
  Attr attrs;
  UserAttributeMap userAttributes;

  template<class SerDe>
  void serde(SerDe& sd) {
    sd(line0)
      (line1)
      (attrs)
      (userAttributes)
      ;
  }

  void prettyPrint(std::ostream& out) const;

  /*
   * Look up the defined Module in this request with name `name'.
   * Return nullptr if the module is not yet defined in this request.
   */
  static Module* lookup(const StringData* name);

  /*
   * Look up, or autoload and define, the Module in this request with name `name'.
   */
  static Module* load(const StringData* name);

  /*
   * Define module m for this request.
   */
  static void def(Module* m);
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Will the use of symbol raise a module boundary violation?
 */
template <typename Sym, typename Ctx>
bool will_symbol_raise_module_boundary_violation(
  const Sym* symbol,
  const Ctx* context
);

///////////////////////////////////////////////////////////////////////////////
}

