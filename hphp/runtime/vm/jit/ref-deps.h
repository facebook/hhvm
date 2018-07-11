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
#ifndef incl_HPHP_TRACELET_H_
#define incl_HPHP_TRACELET_H_

#include <climits>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>

namespace HPHP {
struct Func;

namespace jit {

struct NormalizedInstruction;
struct RegionContext;

struct ActRecState {
  // State for tracking known Func pointers on pre-live ActRecs. The top entry
  // represents activation record that is closest to the top of the stack.
  // Contains Func pointer if it is known, nullptr otherwise.
  std::vector<const Func*> m_arStack;

  ActRecState() {}
  void pushFunc(const NormalizedInstruction& ni);
  void pop();
  const Func* knownFunc();
};

} } // HPHP::jit
#endif
