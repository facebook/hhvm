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

#ifndef incl_HPHP_VM_UNIT_UTIL_H_
#define incl_HPHP_VM_UNIT_UTIL_H_

#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/type-string.h"

#include "hphp/util/slice.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Returns true if the class or function name is normalized wrt namespaces.
 */
inline bool isNSNormalized(const StringData* name) {
  return name->data()[0] != '\\';
}

/*
 * Returns true if the class or function name needs to be normalized.
 */
inline bool needsNSNormalization(const StringData* name) {
  auto const data = name->data();
  return name->size() >= 2 && data[0] == '\\' && data[1] != '\\';
}

/*
 * Normalizes a given class or function name removing the leading '\'.
 * Leaves the name unchanged if more than one '\' is leading.
 * So '\name' becomes 'name' but '\\name' stays '\\name'.
 */
inline const StringData* normalizeNS(const StringData* name) {
  if (needsNSNormalization(name)) {
    return makeStaticString(StringSlice(name->data() + 1, name->size() - 1));
  }
  return name;
}

inline String normalizeNS(const String& name) {
  if (needsNSNormalization(name.get())) {
    return String(name.data() + 1, name.size() - 1, CopyString);
  }
  return name;
}

//////////////////////////////////////////////////////////////////////

}
#endif
