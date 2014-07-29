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

namespace HPHP { namespace JIT {
namespace X64 {
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
const Vlabel InvalidVlabel;

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

// vasm proxy for TransBCMapping
struct Vbcmap {
  MD5 md5;
  Offset bcStart;
  Vpoint mainStart;
  Vpoint coldStart;
  Vpoint frozenStart;
};

// holds information generated while assembling final code;
// designed to outlive instances of Vunit and Vasm.
struct Vmeta {
  Vpoint makePoint() {
    auto next = points.size();
    points.push_back(nullptr);
    return Vpoint{next};
  }
  smart::vector<CodeAddress> points;
};

void allocateRegisters(X64::Vunit&, const Abi&);
void optimizeJmps(X64::Vunit&);
void removeDeadCode(X64::Vunit&);
folly::Range<Vlabel*> succs(X64::Vinstr& inst);
folly::Range<Vlabel*> succs(X64::Vblock& block);
smart::vector<Vlabel> sortBlocks(X64::Vunit& unit);

}}
#endif
