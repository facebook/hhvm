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

#ifndef incl_HPHP_VM_UNIT_UTIL_H_
#define incl_HPHP_VM_UNIT_UTIL_H_

#include <folly/Range.h>

#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-structure.h"

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
 * Returns true if the string is not of the form "Class::Method".
 */
inline bool notClassMethodPair(const StringData* name) {
  return strstr(name->data(), "::") == nullptr;
}

const char kInOutSuffix[] = "$inout";
inline bool needsStripInOut(const StringData* name) {
  auto const sz = name->size();
  return sz > 6 &&
    !memcmp(name->data() + sz - 6, kInOutSuffix, 6) &&
    folly::qfind(name->slice().subpiece(0, sz - 6), folly::StringPiece("$")) !=
      std::string::npos;
}

/*
 * Normalizes a given class or function name removing the leading '\'.
 * Leaves the name unchanged if more than one '\' is leading.
 * So '\name' becomes 'name' but '\\name' stays '\\name'.
 */
inline const StringData* normalizeNS(const StringData* name) {
  if (needsNSNormalization(name)) {
    assertx(name->size() != 0);
    auto const size  = static_cast<size_t>(name->size() - 1);
    auto const piece = folly::StringPiece{name->data() + 1, size};
    return makeStaticString(piece);
  }
  return name;
}

inline String normalizeNS(const String& name) {
  if (needsNSNormalization(name.get())) {
    return String(name.data() + 1, name.size() - 1, CopyString);
  }
  return name;
}

inline const StringData* stripInOutSuffix(const StringData* name) {
  if (UNLIKELY(needsStripInOut(name))) {
    assertx(name->size() > sizeof(kInOutSuffix));
    auto const s = name->data();
    size_t len = name->size() - sizeof(kInOutSuffix);
    for (; s[len] != '$'; --len) assertx(len != 0);
    return makeStaticString(folly::StringPiece(name->data(), len));
  }
  return name;
}

inline StringData* stripInOutSuffix(StringData* name) {
  return const_cast<StringData*>(
    stripInOutSuffix((const StringData*)name)
  );
}

inline String stripInOutSuffix(String& s) {
  return String(stripInOutSuffix(s.get()));
}

inline std::string mangleInOutFuncName(const char* name,
                                       std::vector<uint32_t> params) {
  return folly::sformat("{}${}$inout", name, folly::join(";", params));
}

inline std::string mangleInOutFuncName(const std::string& name,
                                       std::vector<uint32_t> params) {
  return mangleInOutFuncName(name.data(), std::move(params));
}

inline String mangleInOutFuncName(const StringData* name,
                                  std::vector<uint32_t> params) {
  return String(makeStaticString(
    mangleInOutFuncName(name->data(), std::move(params))
  ));
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
#endif
