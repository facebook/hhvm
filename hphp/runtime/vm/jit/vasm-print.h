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

#ifndef incl_HPHP_JIT_VASM_PRINT_H_
#define incl_HPHP_JIT_VASM_PRINT_H_

#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

#include <vector>
#include <iostream>
#include <string>

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct Vinstr;
struct Vunit;
struct Vconst;

///////////////////////////////////////////////////////////////////////////////

std::string show(Vreg r);
std::string show(Vptr p);
std::string show(Vconst c);
std::string show(const Vunit& unit);
std::string show(const Vunit& unit, const Vinstr& inst);

// print a dot-compatible digraph of the blocks (without contents)
void printCfg(const Vunit& unit, const jit::vector<Vlabel>& blocks);
void printCfg(std::ostream& out, const Vunit& unit,
              const jit::vector<Vlabel>& blocks);

// Tracing level constants.
constexpr int kInitialVasmLevel = 1;
constexpr int kVasmImmsLevel = 2;
constexpr int kVasmSimplifyLevel = 2;
constexpr int kVasmFusionLevel = 2;
constexpr int kVasmCodeGenLevel = 2;
constexpr int kVasmRegAllocLevel = 3;
constexpr int kVasmCopyPropLevel = 4;
constexpr int kVasmARMFoldLevel = 4;
constexpr int kVasmJumpsLevel = 4;
constexpr int kVasmExitsLevel = 4;
constexpr int kVasmHoistFbccsLevel = 4;
constexpr int kVasmDCELevel = 4;
constexpr int kVasmLowerLevel = 4;
constexpr int kVasmUnreachableLevel = 6;

// Print the cfg digraph followed by a vasm code listing, if the trace level is
// above `level'.
void printUnit(int level,
               const std::string& caption,
               const Vunit& unit);

void printInstrs(std::ostream& out,
                 const Vunit& unit,
                 const jit::vector<Vinstr>& code);

// main, cold, frozen
extern const char* area_names[];

///////////////////////////////////////////////////////////////////////////////
}}

#endif
