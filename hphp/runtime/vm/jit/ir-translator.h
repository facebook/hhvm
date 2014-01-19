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
namespace JIT {
struct NormalizedInstruction;
struct Tracelet;
struct Location;
struct RuntimeType;
}
namespace JIT {

/*
 * RegionIter is a temporary class used to traverse a region of hhbc
 * instruction that may be more than just a straight-line series of
 * instructions. It is used by shouldIRInline to traverse both Tracelets and
 * RegionDescs.
 */
struct RegionIter {
  virtual ~RegionIter() {}

  virtual bool finished() const = 0;
  virtual SrcKey sk() const = 0;
  virtual void advance() = 0;
};
bool shouldIRInline(const Func* caller, const Func* callee,
                    RegionIter& iter);
bool shouldIRInline(const Func* caller, const Func* callee,
                    const JIT::Tracelet& tlet);

/*
 * IRTranslator is used to convert hhbc instructions to an IRTrace of hhir
 * instructions. It uses an HhbcTranslator to do the actual translation,
 * driving it with a translate<Op> method for each supported bytecode.
 */
struct IRTranslator {
  IRTranslator(Offset bcOff, Offset spOff, const Func* curFunc);

  void translateInstr(const NormalizedInstruction& i);
  void checkType(const JIT::Location& l, const JIT::RuntimeType& rtt,
                 bool outerOnly);
  void assertType(const JIT::Location&, const JIT::RuntimeType&);
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
