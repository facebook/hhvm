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

#include <vector>

namespace HPHP {

/*
 * Struct that contains information regarding reified generics of a function
 * or class
 */
struct ReifiedGenericsInfo {
  // Number of generics, both erased and reified
  size_t m_numGenerics;
  // Type parameter indices of reified generics on the function or class
  // Includes the soft reified ones
  std::vector<size_t> m_reifiedGenericPositions;
  // Type parameter indices of soft reified generics on the function or class
  std::vector<size_t> m_softReifiedGenericPositions;
};

}

#endif
