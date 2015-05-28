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

#include <memory>
#include <vector>

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

/*
 * A NormalizedInstruction contains information about a decoded bytecode
 * instruction, including the unit it lives in, decoded immediates, and a few
 * flags of interest the various parts of the jit.
 */
struct NormalizedInstruction {
  SrcKey source;
  const Func* funcd; // The Func in the topmost AR on the stack. Guaranteed to
                     // be accurate. Don't guess about this. Note that this is
                     // *not* the function whose body the NI belongs to.
                     // Note that for an FPush* may be set to the (statically
                     // known Func* that /this/ instruction is pushing)
  const Unit* m_unit;

  std::vector<Location> inputs;
  ArgUnion imm[4];
  ImmVector immVec; // vector immediate; will have !isValid() if the
                    // instruction has no vector immediate

  // The member codes for the M-vector.
  std::vector<MemberCode> immVecM;

  bool endsRegion:1;
  bool nextIsMerge:1;
  bool preppedByRef:1;
  bool ignoreInnerType:1;

  /*
   * Used with HHIR. Instruction shoud be interpreted, because previous attempt
   * to translate it has failed.
   */
  bool interp:1;

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
};

///////////////////////////////////////////////////////////////////////////////
}}
#endif
