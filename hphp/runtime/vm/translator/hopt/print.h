/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include "util/trace.h"
#include "runtime/vm/translator/hopt/linearscan.h"

namespace HPHP {
namespace VM {
namespace JIT {

struct IRInstruction;
class  SSATmp;
struct Block;
struct AsmInfo;
class  Trace;
struct LifetimeInfo;

// IRInstruction
void print(std::ostream& ostream, const IRInstruction*,
           const LifetimeInfo* lifetime = nullptr);
void print(const IRInstruction*);
void printSrc(std::ostream& ostream, const IRInstruction*, uint32_t srcIndex,
              const LifetimeInfo* lifetime);

// SSATmp
void print(std::ostream& ostream, const SSATmp*,
           const LifetimeInfo* lifetime = nullptr, bool printLastUse = false);
void print(const SSATmp*);

// Trace
void print(std::ostream& ostream, const Trace*,
           const LifetimeInfo* lifetime = nullptr,
           const AsmInfo* asmInfo = nullptr);
void print(const Trace*);

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

void dumpTraceImpl(const Trace* trace, std::ostream& out,
                   const LifetimeInfo*, const AsmInfo*);
void dumpTrace(int level, const Trace* trace, const char* caption,
               const LifetimeInfo* lifetime = nullptr,
               AsmInfo* ai = nullptr);

}}}

#endif
