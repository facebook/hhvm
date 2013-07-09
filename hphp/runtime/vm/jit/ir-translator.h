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
#ifndef incl_HPHP_IRTRANSLATOR_H_
#define incl_HPHP_IRTRANSLATOR_H_

#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/translator-instrs.h"

namespace HPHP {
namespace Transl {
struct NormalizedInstruction;
struct Tracelet;
struct Location;
struct RuntimeType;
}
namespace JIT {
using Transl::NormalizedInstruction;

bool shouldIRInline(const Func* curFunc, const Func* func,
                    const Transl::Tracelet& callee);

/*
 * IRTranslator is used to convert hhbc instructions to an IRTrace of hhir
 * instructions. It uses an HhbcTranslator to do the actual translation,
 * driving it with a translate<Op> method for each supported bytecode.
 */
struct IRTranslator {
  IRTranslator(Offset bcOff, Offset spOff, const Func* curFunc);

  void translateInstr(const NormalizedInstruction& i);
  void checkType(const Transl::Location& l, const Transl::RuntimeType& rtt);
  void assertType(const Transl::Location&, const Transl::RuntimeType&);
  HhbcTranslator& hhbcTrans() { return m_hhbcTrans; }

 private:
  void translateInstrWork(const NormalizedInstruction& i);
  void interpretInstr(const NormalizedInstruction& i);
  void passPredictedAndInferredTypes(const NormalizedInstruction& i);
# define CASE(nm) void translate ## nm(const NormalizedInstruction& i);
INSTRS
PSEUDOINSTRS
# undef CASE

  HhbcTranslator m_hhbcTrans;
};

} }

#endif
