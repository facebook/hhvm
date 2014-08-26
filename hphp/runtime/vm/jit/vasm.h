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

#ifndef incl_HPHP_JIT_VASM_H_
#define incl_HPHP_JIT_VASM_H_

#include "hphp/runtime/vm/jit/types.h"
#include <folly/Range.h>
#include <iosfwd>

namespace HPHP { namespace jit {
namespace x64 {
struct Vunit;
struct Vinstr;
struct Vblock;
}
struct Abi;

// Vlabel wraps a block number
struct Vlabel {
  Vlabel() : n(0xffffffff) {}
  explicit Vlabel(size_t n) : n(static_cast<unsigned>(n)) {}
  /* implicit */ operator size_t() const { return n; }
private:
  unsigned n; // index in Vunit::blocks
};

// Vpoint is a handle to record or retreive a code address
struct Vpoint {
  Vpoint(){}
  explicit Vpoint(size_t n) : n(static_cast<unsigned>(n)) {}
  /* implicit */ operator size_t() const { return n; }
private:
  unsigned n; // index in Vmeta::points
};

// Vtuple is an index to a tuple in Vunit::tuples
struct Vtuple {
  Vtuple() : n(0xffffffff) {}
  explicit Vtuple(size_t n) : n(static_cast<unsigned>(n)) {}
  /* implicit */ operator size_t() const { return n; }
private:
  unsigned n; // index in Vunit::reglists
};

enum class VregKind : uint8_t { Any, Gpr, Simd };

// holds information generated while assembling final code;
// designed to outlive instances of Vunit and Vasm.
struct Vmeta {
  Vpoint makePoint() {
    auto next = points.size();
    points.push_back(nullptr);
    return Vpoint{next};
  }
  jit::vector<CodeAddress> points;
};

void allocateRegisters(x64::Vunit&, const Abi&);
void optimizeJmps(x64::Vunit&);
void removeDeadCode(x64::Vunit&);
folly::Range<Vlabel*> succs(x64::Vinstr& inst);
folly::Range<Vlabel*> succs(x64::Vblock& block);
jit::vector<Vlabel> sortBlocks(x64::Vunit& unit);
std::string formatInstr(x64::Vunit& unit, x64::Vinstr& inst);
void printBlock(std::ostream& out, x64::Vunit& unit, Vlabel b);

// print a dot-compatible digraph of the blocks (without contents)
void printCfg(x64::Vunit& unit, jit::vector<Vlabel>& blocks);
void printCfg(std::ostream& out, x64::Vunit& unit,
              jit::vector<Vlabel>& blocks);

// print the cfg digraph followed by a code listing
void printUnit(std::string caption, x64::Vunit& unit);

}}
#endif
