/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <memory>

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP {
namespace JIT {


struct DynLocation;

// A NormalizedInstruction has been decorated with its typed inputs.
struct NormalizedInstruction {
  SrcKey source;
  const Func* funcd; // The Func in the topmost AR on the stack. Guaranteed to
                     // be accurate. Don't guess about this. Note that this is
                     // *not* the function whose body the NI belongs to.
                     // Note that for an FPush* may be set to the (statically
                     // known Func* that /this/ instruction is pushing)
  const Unit* m_unit;

  std::vector<DynLocation*> inputs;
  Type         outPred;
  ArgUnion imm[4];
  ImmVector immVec; // vector immediate; will have !isValid() if the
                    // instruction has no vector immediate

  // The member codes for the M-vector.
  std::vector<MemberCode> immVecM;

  Offset nextOffset; // for intra-trace* non-call control-flow instructions,
                     // this is the offset of the next instruction in the trace*
  bool breaksTracelet:1;
  bool includeBothPaths:1;
  bool nextIsMerge:1;
  bool changesPC:1;
  bool preppedByRef:1;
  bool outputPredicted:1;
  bool ignoreInnerType:1;

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

  Op op() const;
  Op mInstrOp() const;
  PC pc() const;
  const Unit* unit() const;
  const Func* func() const;
  Offset offset() const;
  SrcKey nextSk() const;

  NormalizedInstruction();
  NormalizedInstruction(SrcKey, const Unit*);
  ~NormalizedInstruction();

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
