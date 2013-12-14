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
#ifndef incl_HPHP_TRACELET_H_
#define incl_HPHP_TRACELET_H_

#include <boost/ptr_container/ptr_vector.hpp>

#include "hphp/util/base.h"
#include "hphp/runtime/vm/jit/runtime-type.h"

namespace HPHP {
namespace JIT {

struct NormalizedInstruction;

/*
 * A tracelet is a unit of input to the back-end. It is a partially typed,
 * non-maximal basic block, representing the next slice of the program to
 * be executed.
 * It is a consecutive set of instructions, only the last of which may be a
 * transfer of control, annotated types and locations for each opcode's input
 * and output.
 */
typedef hphp_hash_map<Location, DynLocation*, Location> ChangeMap;
typedef ChangeMap DepMap;

struct InstrStream {
  InstrStream() : first(nullptr), last(nullptr) {}
  void append(NormalizedInstruction* ni);
  void remove(NormalizedInstruction* ni);
  NormalizedInstruction* first;
  NormalizedInstruction* last;
};

struct RefDeps {
  struct Record {
    vector<bool> m_mask;
    vector<bool> m_vals;

    std::string pretty() const {
      std::ostringstream out;
      out << "mask=";
      for (size_t i = 0; i < m_mask.size(); ++i) {
        out << (m_mask[i] ? "1" : "0");
      }
      out << " vals=";
      for (size_t i = 0; i < m_vals.size(); ++i) {
        out << (m_vals[i] ? "1" : "0");
      }
      return out.str();
    }
  };
  typedef hphp_hash_map<int64_t, Record, int64_hash> ArMap;
  ArMap m_arMap;

  RefDeps() {}

  void addDep(int entryArDelta, unsigned argNum, bool isRef) {
    if (m_arMap.find(entryArDelta) == m_arMap.end()) {
      m_arMap[entryArDelta] = Record();
    }
    Record& r = m_arMap[entryArDelta];
    if (argNum >= r.m_mask.size()) {
      assert(argNum >= r.m_vals.size());
      r.m_mask.resize(argNum + 1);
      r.m_vals.resize(argNum + 1);
    }
    r.m_mask[argNum] = true;
    r.m_vals[argNum] = isRef;
  }

  size_t size() const {
    return m_arMap.size();
  }
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

struct Tracelet : private boost::noncopyable {
  ChangeMap      m_changes;
  DepMap         m_dependencies;
  DepMap         m_resolvedDeps; // dependencies resolved by static analysis
  InstrStream    m_instrStream;
  int            m_stackChange;

  // SrcKey for the start of the Tracelet. This will be the same as
  // m_instrStream.first->source.
  SrcKey         m_sk;

  // numOpcodes is the number of raw opcode instructions, before optimization.
  // The immediates optimization may both:
  //
  // 1. remove the first opcode, thus making
  //        sk.instr != instrs.first->source.instr
  // 2. remove no longer needed instructions
  int            m_numOpcodes;

  // Assumptions about entering actRec's reffiness.
  ActRecState    m_arState;
  RefDeps        m_refDeps;

  /*
   * If we were unable to make sense of the instruction stream (e.g., it
   * used instructions that the translator does not understand), then this
   * tracelet is useful only for defining the boundaries of a basic block.
   * The low-level translator can handle this by backing off to the
   * bytecode interpreter.
   */
  bool           m_analysisFailed;

  /*
   * If IR inlining failed we may still need access to the trace for profiling
   * purposes if stats are enabled so maintain this to verify that we should use
   * this Tracelet for inlining purposes.
   */
  bool           m_inliningFailed;

  // Track which NormalizedInstructions and DynLocations are owned by this
  // Tracelet; used for cleanup purposes
  boost::ptr_vector<NormalizedInstruction> m_instrs;
  boost::ptr_vector<DynLocation> m_dynlocs;

  Tracelet();
  ~Tracelet();

  NormalizedInstruction* newNormalizedInstruction();
  DynLocation* newDynLocation(Location l, DataType t);
  DynLocation* newDynLocation(Location l, RuntimeType t);
  DynLocation* newDynLocation();

  /* These aren't merged into a single method with a default argument
   * to make gdb happy. */
  void print() const;
  void print(std::ostream& out) const;
  std::string toString() const;

  SrcKey nextSk() const;
  const Func* func() const;
};

} } // HPHP::JIT
#endif
