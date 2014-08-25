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
#ifndef incl_HPHP_IRTRANSLATOR_H_
#define incl_HPHP_IRTRANSLATOR_H_

#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/translator-instrs.h"

namespace HPHP { namespace jit {
struct NormalizedInstruction;
struct Location;

/*
 * IRTranslator is used to convert hhbc instructions to a cfg of Blocks
 * containing hhir instructions. It uses an HhbcTranslator to do the actual
 * translation, driving it with a translate<Op> method for each supported
 * bytecode.
 */
struct IRTranslator {
  explicit IRTranslator(TransContext context);

  void translateInstr(const NormalizedInstruction& i);

  /**
   * Check if `i' is an FPush{Func,ClsMethod}D followed by an FCall{,D} to
   * a function with a singleton pattern, and if so, inline it.  Returns true
   * if this succeeds, else false.
   */
  bool tryTranslateSingletonInline(const NormalizedInstruction& i,
                                   const Func* funcd);

  HhbcTranslator& hhbcTrans() { return m_hhbcTrans; }

 private:
  void translateInstrWork(const NormalizedInstruction& i);
  void interpretInstr(const NormalizedInstruction& i);

  // Generated callers to HhbcTranslator.
  //
  // There are two overloads of "unpackName" for each opcode name, distinguished
  // by the first parameter. The first overload will be taken out by enable_if
  // if HhbcTranslator::supportsName is not true. The first parameter of this
  // one is nullptr_t.
  //
  // The second overload is there to take over if the first one fails, and just
  // asserts. The first parameter is void*. These methods are always called with
  // nullptr as the first argument; this way, the first overload is preferred if
  // it exists but the compiler is allowed to fall back to the second overload
  // by way of an implicit conversion.
  //
  // The overloads are templates so that invalid calls to emitName methods on
  // HhbcTranslator are template parameter substitution failures, which is of
  // course Not An Error.
  //
  // If you're hitting the always_assert below, that means there's an opcode in
  // REGULAR_INSTRS that doesn't have a corresponding method in HhbcTranslator
  // with the correct signature.
# define O(nm, a_, b_, c_, d_) \
  template<class HT = HhbcTranslator> \
  typename std::enable_if<HT::supports##nm, void>::type \
  unpack##nm(std::nullptr_t, const NormalizedInstruction& i); \
  \
  template<class HT = HhbcTranslator> \
  void unpack##nm(void*, const NormalizedInstruction& i) { \
    always_assert(false); \
  }

  OPCODES
# undef O

  // Handwritten callers to HhbcTranslator.
# define CASE(nm) void translate##nm(const NormalizedInstruction& i);
  IRREGULAR_INSTRS
  PSEUDOINSTRS
# undef CASE

  HhbcTranslator m_hhbcTrans;
};

} }

#endif
