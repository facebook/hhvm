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

#ifndef incl_HPHP_VM_PRINT_H_
#define incl_HPHP_VM_PRINT_H_

#include <iosfwd>
#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/code-gen.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/type-constraint.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP { namespace jit {

struct AsmInfo;
struct Block;
struct GuardConstraints;
struct IRInstruction;
class  SSATmp;

// IRInstruction
void printInstr(std::ostream& ostream, const IRInstruction*,
                const GuardConstraints* guards = nullptr);
void printDsts(std::ostream& os, const IRInstruction* inst);
void printSrcs(std::ostream& os, const IRInstruction* inst);
void printOpcode(std::ostream& os, const IRInstruction* inst,
                 const GuardConstraints* guards);
void printSrcs(std::ostream& os, const IRInstruction* inst);
void printDsts(std::ostream& os, const IRInstruction* inst);
void print(std::ostream& ostream, const IRInstruction*,
           const GuardConstraints* guards = nullptr);
void print(const IRInstruction*);

// SSATmp
void print(std::ostream& ostream, const SSATmp*);
void print(const SSATmp*);

// Block
void print(std::ostream& os, const Block* block,
           AreaIndex area,
           const AsmInfo* asmInfo = nullptr,
           const GuardConstraints* guards = nullptr,
           BCMarker* curMarker = nullptr);
void print(const Block* block);

// Unit
void print(std::ostream& ostream, const IRUnit&,
           const AsmInfo* asmInfo = nullptr,
           const GuardConstraints* guards = nullptr);
void print(const IRUnit& unit);

/*
 * Some utilities related to dumping. Rather than file-by-file control, we
 * control most IR logging via the hhir trace module.
 */
static inline bool dumpIREnabled(int level = 1) {
  return HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir, level) ||
         RuntimeOption::EvalDumpIR >= level;
}

constexpr int kCodeGenLevel = 1;
constexpr int kIRLevel = 2;
constexpr int kOptLevel = 3;
constexpr int kTraceletLevel = 4;
constexpr int kRegAllocLevel = 4;
constexpr int kRelocationLevel = 4;
constexpr int kExtraLevel = 6;
constexpr int kExtraExtraLevel = 7;

void printUnit(int level, const IRUnit&, const char* caption,
               AsmInfo* ai = nullptr, const GuardConstraints* guards = nullptr);

inline std::ostream& operator<<(std::ostream& os, Type t) {
  return os << t.toString();
}
inline std::ostream& operator<<(std::ostream& os, TypeConstraint tc) {
  return os << tc.toString();
}

std::string banner(const char* caption);

void disasmRange(std::ostream& os, TCA begin, TCA end);
inline void disasmRange(std::ostream& os, TcaRange r) {
  return disasmRange(os, r.begin(), r.end());
}

}}

#endif
