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

#ifndef incl_HPHP_VM_PRINT_H_
#define incl_HPHP_VM_PRINT_H_

#include <iosfwd>
#include "hphp/util/trace.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/linear-scan.h"

namespace HPHP {
namespace JIT {

struct IRInstruction;
class  SSATmp;
struct Block;
struct AsmInfo;
class  IRTrace;
struct LifetimeInfo;

// IRInstruction
void print(std::ostream& ostream, const IRInstruction*,
           const RegAllocInfo* regs = nullptr,
           const LifetimeInfo* lifetime = nullptr,
           const GuardConstraints* guards = nullptr);
void print(const IRInstruction*);
void printInstr(std::ostream& ostream, const IRInstruction*,
                const RegAllocInfo* regs = nullptr,
                const LifetimeInfo* lifetime = nullptr,
                const GuardConstraints* guards = nullptr);
void printDsts(std::ostream& os, const IRInstruction* inst,
               const RegAllocInfo* regs,
               const LifetimeInfo* lifetime);
void printSrcs(std::ostream& os, const IRInstruction* inst,
               const RegAllocInfo* regs,
               const LifetimeInfo* lifetime);
void printOpcode(std::ostream& os, const IRInstruction* inst,
                 const GuardConstraints* guards);

// SSATmp
void print(std::ostream& ostream, const SSATmp*,
           const PhysLoc* loc = nullptr,
           const LifetimeInfo* lifetime = nullptr,
           bool printLastUse = false);
void print(const SSATmp*);

// Block
void print(std::ostream& os, const Block* block,
           const RegAllocInfo* regs = nullptr,
           const LifetimeInfo* lifetime = nullptr,
           const AsmInfo* asmInfo = nullptr,
           const GuardConstraints* guards = nullptr,
           BCMarker* curMarker = nullptr);
void print(const Block* block);

// Trace
void print(std::ostream& ostream, const IRUnit&, const IRTrace*,
           const RegAllocInfo* regs = nullptr,
           const LifetimeInfo* lifetime = nullptr,
           const AsmInfo* asmInfo = nullptr,
           const GuardConstraints* guards = nullptr);

// Print the whole unit
inline void print(std::ostream& ostream, const IRUnit& unit,
                  const RegAllocInfo* regs = nullptr,
                  const LifetimeInfo* lifetime = nullptr,
                  const AsmInfo* asmInfo = nullptr,
                  const GuardConstraints* guards = nullptr) {
  print(ostream, unit, unit.main(), regs, lifetime, asmInfo, guards);
}

void print(const IRTrace*);

/*
 * Some utilities related to dumping. Rather than file-by-file control, we
 * control most IR logging via the hhir trace module.
 */
static inline bool dumpIREnabled(int level = 1) {
  return HPHP::Trace::moduleEnabledRelease(HPHP::Trace::printir, level);
}

static const int kIRLevel = 1;
static const int kCodeGenLevel = 2;
static const int kRegAllocLevel = 3;
static const int kOptLevel = 4;
static const int kExtraLevel = 6;

void dumpTrace(int level, const IRUnit&, const char* caption,
               const RegAllocInfo* regs = nullptr,
               const LifetimeInfo* lifetime = nullptr,
               AsmInfo* ai = nullptr, const GuardConstraints* guards = nullptr);

}}

#endif
