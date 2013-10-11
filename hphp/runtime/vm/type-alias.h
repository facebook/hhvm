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
#ifndef incl_HPHP_VM_TYPE_ALIAS_H_
#define incl_HPHP_VM_TYPE_ALIAS_H_

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * This is the runtime representation of a type alias.  Type aliases
 * are only allowed when HipHop extensions are enabled.
 *
 * The m_kind field is KindOfObject whenever the type alias is
 * basically just a name.  At runtime we still might resolve this name
 * to another type alias, becoming a type alias for KindOfArray or
 * something in that request.
 *
 * For the per-request struct, see TypeAliasReq below.
 */
struct TypeAlias {
  const StringData* name;
  const StringData* value;
  DataType          kind;
  bool              nullable; // Null is allowed; for ?Foo aliases

  template<class SerDe> void serde(SerDe& sd) {
    sd(name)
      (value)
      (kind)
      (nullable)
      ;
  }
};

/*
 * In a given request, a defined type alias is turned into a
 * TypeAliasReq struct.  This contains the information needed to
 * validate parameter type hints for a type alias at runtime.
 */
struct TypeAliasReq {
  DataType kind;          // may be KindOfAny for "mixed"
  bool nullable;          // for option types, like ?Foo
  Class* klass;           // nullptr if kind != KindOfObject
  const StringData* name; // needed for error messages; nullptr if not defined
};

//////////////////////////////////////////////////////////////////////

}

#endif
