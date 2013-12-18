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
#ifndef incl_HPHP_NORMALIZED_INSTRUCTION_H_
#define incl_HPHP_NORMALIZED_INSTRUCTION_H_

#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP {
namespace JIT {


struct DynLocation;
struct Tracelet;

// A NormalizedInstruction has been decorated with its typed inputs and
// outputs.
class NormalizedInstruction {
 public:
  NormalizedInstruction* next;
  NormalizedInstruction* prev;

  SrcKey source;
  const Func* funcd; // The Func in the topmost AR on the stack. Guaranteed to
                     // be accurate. Don't guess about this. Note that this is
                     // *not* the function whose body the NI belongs to.
                     // Note that for an FPush* may be set to the (statically
                     // known Func* that /this/ instruction is pushing)
  const StringData* funcName;
    // For FCall's, an opaque identifier that is either null, or uniquely
    // identifies the (functionName, -arity) pair of this call site.
  const Unit* m_unit;

  std::vector<DynLocation*> inputs;
  DynLocation* outStack;
  DynLocation* outLocal;
  DynLocation* outLocal2; // Used for IterInitK, MIterInitK, IterNextK,
                          //   MIterNextK
  DynLocation* outStack2; // Used for CGetL2
  DynLocation* outStack3; // Used for CGetL3
  Type         outPred;
  vector<Location> deadLocs; // locations that die at the end of this
                             // instruction
  ArgUnion imm[4];
  ImmVector immVec; // vector immediate; will have !isValid() if the
                    // instruction has no vector immediate

  // The member codes for the M-vector.
  std::vector<MemberCode> immVecM;

  /*
   * For property dims, if we know the Class* for the base when we'll
   * be executing a given dim, it is stored here (at the index for the
   * relevant member code minus 1, because the known class for the
   * first member code is given by the base in inputs[]).
   *
   * Other entries here store null.  See MetaInfo::MVecPropClass.
   */
  std::vector<Class*> immVecClasses;

  /*
   * On certain FCalls, we can inspect the callee and generate a
   * tracelet with information about what happens over there.
   *
   * The HHIR translator uses this to possibly inline callees.
   */
  std::unique_ptr<Tracelet> calleeTrace;

  unsigned checkedInputs;
  // StackOff: logical delta at *start* of this instruction to
  // stack at tracelet entry.
  int stackOffset;
  int sequenceNum;
  Offset nextOffset; // for intra-trace* non-call control-flow instructions,
                     // this is the offset of the next instruction in the trace*
  bool breaksTracelet:1;
  bool changesPC:1;
  bool fuseBranch:1;
  bool preppedByRef:1;
  bool outputPredicted:1;
  bool outputPredictionStatic:1;
  bool ignoreInnerType:1;

  /*
   * guardedThis indicates that we know that ar->m_this is
   * a valid $this. eg:
   *
   *   $this->foo = 1; # needs to check that $this is non-null
   *   $this->bar = 2; # can skip the check
   *   return 5;       # can decRef ar->m_this unconditionally
   */
  bool guardedThis:1;

  /*
   * guardedCls indicates that we know the class exists
   */
  bool guardedCls:1;

  /*
   * instruction is statically known to have no effect, e.g. unboxing a Cell
   */
  bool noOp:1;

  /*
   * Used with HHIR. Instruction shoud be interpreted, because previous attempt
   * to translate it has failed.
   */
  bool interp:1;

  /*
   * Indicates that a RetC/RetV should generate inlined return code
   * rather than calling the shared stub.
   */
  bool inlineReturn:1;

  // For returns, this tracks local ids that are statically known not
  // to be reference counted at this point (i.e. won't require guards
  // or decrefs).
  boost::dynamic_bitset<> nonRefCountedLocals;

  Op op() const;
  Op mInstrOp() const;
  PC pc() const;
  const Unit* unit() const;
  const Func* func() const;
  Offset offset() const;
  SrcKey nextSk() const;

  NormalizedInstruction();
  ~NormalizedInstruction();

  void markInputInferred(int i) {
    if (i < 32) checkedInputs |= 1u << i;
  }

  bool inputWasInferred(int i) const {
    return i < 32 && ((checkedInputs >> i) & 1);
  }

  enum class OutputUse {
    Used,
    Unused,
    Inferred,
    DoesntCare
  };
  OutputUse getOutputUsage(const DynLocation* output) const;
  bool isOutputUsed(const DynLocation* output) const;
  bool isAnyOutputUsed() const;

  std::string toString() const;

  // Returns a DynLocation that will be destroyed with this
  // NormalizedInstruction.
  template<typename... Args>
  DynLocation* newDynLoc(Args&&... args) {
    m_dynLocs.push_back(
      smart::make_unique<DynLocation>(std::forward<Args>(args)...));
    return m_dynLocs.back().get();
  }

 private:
  smart::vector<smart::unique_ptr<DynLocation>> m_dynLocs;
};

} } // HPHP::JIT
#endif
