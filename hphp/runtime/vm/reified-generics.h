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

#ifndef incl_HPHP_VM_REIFIEDGENERICS_H_
#define incl_HPHP_VM_REIFIEDGENERICS_H_

#include "hphp/runtime/base/tv-val.h"

#include "hphp/runtime/vm/unit-util.h"

#include "hphp/util/hash-map.h"

namespace HPHP {

struct ActRec;
struct ArrayData;
struct StringData;

///////////////////////////////////////////////////////////////////////////////

namespace {
/*
 * Global reified types map for classes
 */
using ReifiedGenericsTable = hphp_string_map<ArrayData*>;

extern ReifiedGenericsTable g_reified_generics_table;

} // namespace

void addToReifiedGenericsTable(const std::string& mangledName,
                               ArrayData* tsList);
ArrayData* getReifiedTypeList(const std::string& name);

///////////////////////////////////////////////////////////////////////////////

// Returns the value on the property that holds reified generics
// If the cls does not have any reified generics, then returns nullptr
ArrayData* getClsReifiedGenericsProp(Class* cls, ActRec* ar);

///////////////////////////////////////////////////////////////////////////////

// Pulls the reified generics associated with the name named 'name'
// Throws if no such reified generics exist
inline ArrayData* getReifiedGenerics(StringData* name) {
  assertx(isReifiedName(name));
  auto const tstr = stripClsOrFnNameFromReifiedName(name->toCppString());
  return getReifiedTypeList(tstr);
}

// Does the same operation as getReifiedGenerics but returns nullptr
// if no such generics exist
inline ArrayData* getReifiedGenericsOpt(Cell cell) {
  if (!isStringType(cell.m_type) || !isReifiedName(cell.m_data.pstr)) {
    return nullptr;
  }
  return getReifiedGenerics(cell.m_data.pstr);
}

}

#endif
