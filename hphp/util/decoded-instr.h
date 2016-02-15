/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | (c) Copyright IBM Corporation 2016                                   |
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

#ifndef incl_HPHP_JIT_DECODED_INSTR_H_
#define incl_HPHP_JIT_DECODED_INSTR_H_

#include "hphp/runtime/base/arch.h"

#include "hphp/util/decoded-instr-x64.h"
#include "hphp/ppc64-asm/decoded-instr-ppc64.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

#if defined(__powerpc64__)
using DecodedInstruction = ppc64_asm::DecodedInstruction;
#else
using DecodedInstruction = x64::DecodedInstruction;
#endif

///////////////////////////////////////////////////////////////////////////////

}}

#endif
