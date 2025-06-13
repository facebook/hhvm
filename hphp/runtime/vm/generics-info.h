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

#include <algorithm>

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/containers.h"

namespace HPHP {

struct TypeParamInfo {
  // Is the type parameter reified
  bool m_isReified : 1;
  // Does the type parameter contain a soft annotation
  bool m_isSoft    : 1;
  // Does the type parameter contain a warn annotation
  bool m_isWarn    : 1;
  // Name of the template param
  LowStringPtr m_typeName;
};

/*
 * Struct that contains information regarding reified generics of a function
 * or class
 */
struct GenericsInfo {
  using iterator = VMFixedVector<TypeParamInfo>::iterator;
  using const_iterator = VMFixedVector<TypeParamInfo>::const_iterator;

  GenericsInfo() : m_typeParamInfo() {}
  GenericsInfo(std::vector<TypeParamInfo>&& info)
    : m_typeParamInfo(std::move(info)) {}
  VMFixedVector<TypeParamInfo> m_typeParamInfo;

  bool hasSoft() const {
    return std::any_of(
      m_typeParamInfo.begin(),
      m_typeParamInfo.end(),
      [](TypeParamInfo t) {
        return t.m_isSoft;
      }
    );
  }

  bool allGenericsSoft() const {
    return !std::any_of(m_typeParamInfo.begin(), m_typeParamInfo.end(),
                        [](TypeParamInfo t) {
                          return !t.m_isSoft;
                        });
  }

  bool allGenericsFullyReified() const {
    return std::all_of(m_typeParamInfo.begin(), m_typeParamInfo.end(),
                       [](TypeParamInfo t) {
                         return t.m_isReified && !t.m_isSoft;
                       });
  }

  const_iterator begin() const { return m_typeParamInfo.begin(); }
  const_iterator end()   const { return m_typeParamInfo.end(); }
  iterator begin()             { return m_typeParamInfo.begin(); }
  iterator end()               { return m_typeParamInfo.end(); }
  bool empty() const {return m_typeParamInfo.empty(); }

};

}

