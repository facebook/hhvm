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
#include "hphp/runtime/vm/jit/hhbc-translator.h"

#include <cstdlib>

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * Instructions that unconditionally are implemented with InterpOne are
 * translated here.
 */

#define INTERP emitInterpOne(*m_currentNormalizedInstruction);

void HhbcTranslator::emitFPushObjMethod(int32_t, ObjMethodOp) { INTERP }

void HhbcTranslator::emitLowInvalid()              { std::abort(); }
void HhbcTranslator::emitCGetL3(int32_t)           { INTERP }
void HhbcTranslator::emitBox()                     { INTERP }
void HhbcTranslator::emitBoxR()                    { INTERP }
void HhbcTranslator::emitAddElemV()                { INTERP }
void HhbcTranslator::emitAddNewElemV()             { INTERP }
void HhbcTranslator::emitClsCns(int32_t)           { INTERP }
void HhbcTranslator::emitExit()                    { INTERP }
void HhbcTranslator::emitFatal(FatalOp)            { INTERP }
void HhbcTranslator::emitUnwind()                  { INTERP }
void HhbcTranslator::emitThrow()                   { INTERP }
void HhbcTranslator::emitCGetN()                   { INTERP }
void HhbcTranslator::emitVGetN()                   { INTERP }
void HhbcTranslator::emitIssetN()                  { INTERP }
void HhbcTranslator::emitEmptyN()                  { INTERP }
void HhbcTranslator::emitSetN()                    { INTERP }
void HhbcTranslator::emitSetOpN(SetOpOp)           { INTERP }
void HhbcTranslator::emitSetOpG(SetOpOp)           { INTERP }
void HhbcTranslator::emitSetOpS(SetOpOp)           { INTERP }
void HhbcTranslator::emitIncDecN(IncDecOp)         { INTERP }
void HhbcTranslator::emitIncDecG(IncDecOp)         { INTERP }
void HhbcTranslator::emitIncDecS(IncDecOp)         { INTERP }
void HhbcTranslator::emitBindN()                   { INTERP }
void HhbcTranslator::emitUnsetN()                  { INTERP }
void HhbcTranslator::emitUnsetG()                  { INTERP }
void HhbcTranslator::emitFPassN(int32_t)           { INTERP }
void HhbcTranslator::emitFCallUnpack(int32_t)      { INTERP }
void HhbcTranslator::emitCufSafeArray()            { INTERP }
void HhbcTranslator::emitCufSafeReturn()           { INTERP }
void HhbcTranslator::emitIncl()                    { INTERP }
void HhbcTranslator::emitInclOnce()                { INTERP }
void HhbcTranslator::emitReq()                     { INTERP }
void HhbcTranslator::emitReqDoc()                  { INTERP }
void HhbcTranslator::emitReqOnce()                 { INTERP }
void HhbcTranslator::emitEval()                    { INTERP }
void HhbcTranslator::emitDefTypeAlias(int32_t)     { INTERP }
void HhbcTranslator::emitCatch()                   { INTERP }
void HhbcTranslator::emitHighInvalid()             { std::abort(); }

//////////////////////////////////////////////////////////////////////

}}

