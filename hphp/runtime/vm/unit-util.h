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

#include <string_view>
#include <folly/Range.h>

#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-structure.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Returns true if the class or function name is normalized wrt namespaces.
 */
inline bool isNSNormalized(std::string_view name) {
  return name.data()[0] != '\\';
}

inline bool isNSNormalized(const StringData* name) {
  return isNSNormalized(name->slice());
}

/*
 * Returns true if the class or function name needs to be normalized.
 */
inline bool needsNSNormalization(std::string_view name) {
  auto const data = name.data();
  return name.size() >= 2 && data[0] == '\\' && data[1] != '\\';
}

inline bool needsNSNormalization(const StringData* name) {
  return needsNSNormalization(name->slice());
}

/*
 * Returns true if the string is not of the form "Class::Method".
 */
inline bool notClassMethodPair(const StringData* name) {
  return strstr(name->data(), "::") == nullptr;
}

/*
 * Normalizes a given class or function name removing the leading '\'.
 * Leaves the name unchanged if more than one '\' is leading.
 * So '\name' becomes 'name' but '\\name' stays '\\name'.
 */

inline std::string_view normalizeNS(std::string_view name) {
  if (needsNSNormalization(name)) {
    return {name.data() + 1, name.size() - 1};
  }
  return name;
}

inline const StringData* normalizeNS(const StringData* name) {
  if (needsNSNormalization(name)) {
    assertx(!name->empty());
    return makeStaticString(normalizeNS(name->slice()));
  }
  return name;
}

inline String normalizeNS(const String& name) {
  if (needsNSNormalization(name.get())) {
    std::string_view normalized = normalizeNS(std::string_view{name.slice()});
    return String(normalized);
  }
  return name;
}

std::string mangleReifiedGenericsName(const ArrayData* tsList);

inline bool isMangledReifiedGenericInClosure(const StringData* name) {
  // mangled name is of the form
  // __captured$reifiedgeneric$class$ or __captured$reifiedgeneric$function$
  // so it must be longer than 32 characters
  return name->size() > 32 &&
         folly::qfind(name->slice(),
                      folly::StringPiece("__captured$reifiedgeneric$"))
          != std::string::npos;
}

//////////////////////////////////////////////////////////////////////

}
