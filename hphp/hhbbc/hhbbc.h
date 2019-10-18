/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#ifndef incl_HPHP_HHBBC_H_
#define incl_HPHP_HHBBC_H_

#include <atomic>
#include <deque>
#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "hphp/runtime/base/repo-auth-type-array.h"

#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/hash.h"
#include "hphp/util/lock.h"
#include "hphp/util/synchronizable.h"

namespace HPHP {

struct UnitEmitter;

enum class Op : uint16_t;
struct OpHash {
  size_t operator()(Op op) const {
    return hash_int64(static_cast<uint16_t>(op));
  }
};

namespace HHBBC {

using MethodMap = hphp_fast_string_imap<hphp_fast_string_iset>;
using OpcodeSet = hphp_fast_set<Op,OpHash>;

//////////////////////////////////////////////////////////////////////

/*
 * This is the public API to this subsystem.
 */

//////////////////////////////////////////////////////////////////////

namespace php {
struct Program;
// basically a unique_ptr<Program>, but we want to be able to use it on a
// forward declared Program
struct ProgramPtr {
  ProgramPtr() = default;
  explicit ProgramPtr(Program* program) : m_program{program} {}
  ~ProgramPtr() { if (m_program) clear(); }
  ProgramPtr(ProgramPtr&& o) : m_program{o.m_program} {
    o.m_program = nullptr;
  }
  ProgramPtr& operator=(ProgramPtr&& o) {
    std::swap(m_program, o.m_program);
    return *this;
  }
  Program* get() const { return m_program; }
  Program& operator*() const { return *m_program; }
  Program* operator->() const { return m_program; }
private:
  void clear();
  Program* m_program{};
};
}

// Create a method map for the options structure from a SinglePassReadableRange
// containing a list of Class::methodName strings.
template<class SinglePassReadableRange>
MethodMap make_method_map(SinglePassReadableRange&);

template<class SinglePassReadableRange>
OpcodeSet make_bytecode_map(SinglePassReadableRange& bcs);

//////////////////////////////////////////////////////////////////////

void hard_constprop(bool);

//////////////////////////////////////////////////////////////////////

struct UnitEmitterQueue : Synchronizable {
  // Add another ue. Adding nullptr marks us done.
  void push(std::unique_ptr<UnitEmitter> ue);
  // Get the next ue, or nullptr to indicate we're done.
  std::unique_ptr<UnitEmitter> pop();
  // Fetch any remaining ues.
  // Must be called in single threaded mode, after we've stopped adding ues.
  void fetch(std::vector<std::unique_ptr<UnitEmitter>>& ues);
  // Clear done flag, and get us ready for reuse.
  void reset();
 private:
  std::deque<std::unique_ptr<UnitEmitter>> m_ues;
  std::atomic<bool> m_done{};
};

/*
 * Create a php::Program, and wrap it in a ProgramPtr.
 */
php::ProgramPtr make_program();

/*
 * Add the given unit to the program. May be called asynchronously.
 */
void add_unit_to_program(const UnitEmitter* ue, php::Program& program);

/*
 * Perform whole-program optimization on a set of UnitEmitters.
 *
 * Currently this process relies on some information from HPHPc.  It
 * expects traits are already flattened (it might be wrong if they
 * aren't).
 */
void whole_program(php::ProgramPtr program,
                   UnitEmitterQueue& ueq,
                   std::unique_ptr<ArrayTypeTable::Builder>& arrTable,
                   int num_threads = 0);

//////////////////////////////////////////////////////////////////////

/*
 * Main entry point when the program should behave like hhbbc.
 */
int main(int argc, char** argv);

//////////////////////////////////////////////////////////////////////

}}

#endif
