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

#ifndef incl_HPHP_VM_REIFIEDGENERICSINFO_H_
#define incl_HPHP_VM_REIFIEDGENERICSINFO_H_

#include <stdint.h>
#include <vector>

namespace HPHP {

struct TypeParamInfo {
  // Is the type parameter reified
  bool m_isReified : 1;
  // Does the type parameter contain a soft annotation
  bool m_isSoft    : 1;
  // Does the type parameter contain a warn annotation
  bool m_isWarn    : 1;
};

/*
 * Struct that contains information regarding reified generics of a function
 * or class
 */
struct ReifiedGenericsInfo {
  // Number of reified generics
  size_t m_numReifiedGenerics;
  // Whether it has any soft generics
  bool m_hasSoftGenerics;
  // Bitmap used to compare whether generics match in terms of parity in the
  // fast path of CheckFunReifiedGenericMismatch
  uint32_t m_bitmap;
  // Information regarding each type parameter
  std::vector<TypeParamInfo> m_typeParamInfo;
};

}

#endif
