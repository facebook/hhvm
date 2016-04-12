/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/hash-map-typedefs.h"

namespace HPHP {
struct Func;

namespace jit {

struct NormalizedInstruction;

struct RefDeps {
  struct Record {
    std::vector<bool> m_mask;
    std::vector<bool> m_vals;

    std::string pretty() const;
  };
  typedef hphp_hash_map<int64_t, Record, int64_hash> ArMap;
  ArMap m_arMap;

  RefDeps() {}

  void addDep(int entryArDelta, unsigned argNum, bool isRef);

  size_t size() const { return m_arMap.size(); }
};

struct ActRecState {
  // State for tracking function param reffiness. m_topFunc is the function
  // for the activation record that is closest to the top of the stack, or
  // NULL if it is currently unknown. A tracelet can be in one of three
  // epistemological states: GUESSABLE, KNOWN, and UNKNOWABLE. We start out in
  // GUESSABLE, with m_topFunc == NULL (not yet guessed); when it's time to
  // guess, we will use the ActRec seen on the top of stack at compilation
  // time as a hint for refs going forward.
  //
  // The KNOWN state is a very strong guarantee. It means that no matter when
  // this tracelet is executed, no matter what else has happened, the ActRec
  // closest to the top of the stack WILL contain m_topFunc. This means: if that
  // function is defined conditionally, or defined in some other module, you
  // cannot correctly make that assertion. KNOWN indicates absolute certainty
  // about all possible futures.
  //
  // This strange "not-guessed-yet-but-could" state is required by our
  // VM design; at present, the ActRec is not easily recoverable from an
  // arbitrary instruction boundary. However, it can be recovered from the
  // instructions that need to do so.
  static const int InvalidEntryArDelta = INT_MAX;

  enum class State {
    GUESSABLE, KNOWN, UNKNOWABLE
  };

  struct Record {
    State          m_state;
    const Func*    m_topFunc;
    int            m_entryArDelta; // delta at BB entry to guessed ActRec.
  };

  std::vector<Record> m_arStack;

  ActRecState() {}
  void pushFunc(const NormalizedInstruction& ni);
  void pushFuncD(const Func* func);
  void pushDynFunc();
  void pop();
  bool checkByRef(int argNum, int stackOffset, RefDeps* outRefDeps);
  const Func* knownFunc();
  State currentState();
};

} } // HPHP::jit
#endif
