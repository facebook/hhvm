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

namespace HPHP {
namespace VM {
namespace JIT {

struct IRInstruction;
class  SSATmp;
struct Block;
struct AsmInfo;
class  Trace;

// IRInstruction
void print(std::ostream& ostream, const IRInstruction*);
void print(const IRInstruction*);
void printSrc(std::ostream& ostream, const IRInstruction*, uint32_t srcIndex);

// SSATmp
void print(std::ostream& ostream, const SSATmp*,
           bool printLastUse = false);
void print(const SSATmp*);

// Trace
void print(std::ostream& ostream, const Trace*,
           const AsmInfo* asmInfo = nullptr);
void print(const Trace*);

/*
 * Some utilities related to dumping. Rather than file-by-file control, we
 * control most IR logging via the hhir trace module.
 */
static inline bool dumpIREnabled(int level = 1) {
  return HPHP::Trace::moduleEnabledRelease(HPHP::Trace::hhir, level);
}

void dumpTraceImpl(const Trace* trace, std::ostream& out,
                   const AsmInfo* asmInfo = nullptr);
void dumpTrace(int level, const Trace* trace, const char* caption,
               AsmInfo* ai = nullptr);

}}}

#endif
