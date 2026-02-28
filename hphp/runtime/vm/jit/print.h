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

#pragma once

#include "hphp/runtime/vm/jit/guard-constraint.h"
#include "hphp/runtime/vm/jit/reg-alloc.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/util/trace.h"

#include <iosfwd>

namespace HPHP::jit {

struct AsmInfo;
struct Block;
struct GuardConstraints;
struct IRInstruction;
struct LoopInfo;
struct SSATmp;

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
           TransKind kind,
           const AsmInfo* asmInfo = nullptr,
           const GuardConstraints* guards = nullptr,
           BCMarker* curMarker = nullptr,
           const LoopInfo* loopInfo = nullptr);
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
bool dumpIREnabled(TransKind kind, int level = 1);

/*
 * The constants here define the behavior of different printir trace
 * levels. The levels are cumulative, so level 2 includes everything from level
 * 1, etc...
 *
 * 1: Print hhir units once, after all optimizations and code generation are
 *    complete.
 * 2: Same as 1, but also prints the generated machine code inline if the
 *    current build supports disassembly.
 * 3: Print hhir units immediately after initial lowering from hhbc -> hhir,
 *    before any optimizations.
 * 4: Print hhir units after all optimizations, right before lowering to vasm.
 * 5: Print hhir units created for region selection in region-tracelet.cpp, and
 *    print hhir units after code generation, before they are relocated to
 *    their final destination.
 * 6: Prints the machine code prior to relocation.
 * 7: In hhir unit dumps that contain disassembly, include the raw instruction
 *    encoding, printed as hex values.
 * 8: Print stats about the frequency of each opcode at the beginning of hhir
 *    unit dumps.
 */
constexpr int kCodeGenLevel = 1;
constexpr int kDisasmLevel = 2;
constexpr int kIRLevel = 3;
constexpr int kOptLevel = 4;
constexpr int kTraceletLevel = 5;
constexpr int kRelocationLevel = 6;
constexpr int kAsmEncodingLevel = 7;
constexpr int kExtraExtraLevel = 8;

void printUnit(int level, const IRUnit&, const char* caption,
               AsmInfo* ai = nullptr, const GuardConstraints* guards = nullptr,
               Annotations* annot = nullptr);

inline std::ostream& operator<<(std::ostream& os, const Type& t) {
  return os << t.toString();
}
inline std::ostream& operator<<(std::ostream& os, GuardConstraint gc) {
  return os << gc.toString();
}

std::string banner(const char* caption);

void disasmRange(std::ostream& os,
                 TransKind kind,
                 TCA begin,
                 TCA end,
                 uint64_t adjust,
                 bool useColor = false);

inline void disasmRange(std::ostream& os, TransKind kind, TcaRange r) {
  return disasmRange(os, kind, r.begin(), r.end(), 0);
}

}
