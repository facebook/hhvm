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
#pragma once

#include <atomic>
#include <deque>
#include <future>
#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "hphp/runtime/base/repo-auth-type-array.h"

#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/repo-file.h"

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

struct UnitEmitterQueue : Synchronizable {
  explicit UnitEmitterQueue(RepoAutoloadMapBuilder* b = nullptr,
                            bool storeUnitEmitters = false)
    : m_storeUnitEmitters{storeUnitEmitters}
    , m_repoBuilder{b} {}

  void push(std::unique_ptr<UnitEmitter> ue);
  void finish();
  // Get the next ue, or nullptr to indicate we're done.
  folly::Optional<RepoFileBuilder::EncodedUE> pop();
  std::unique_ptr<UnitEmitter> popUnitEmitter();
 private:
  bool m_storeUnitEmitters;
  std::deque<RepoFileBuilder::EncodedUE> m_encoded;
  std::deque<std::unique_ptr<UnitEmitter>> m_ues;
  RepoAutoloadMapBuilder* m_repoBuilder;
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
 */
void whole_program(php::ProgramPtr program,
                   UnitEmitterQueue& ueq,
                   std::unique_ptr<ArrayTypeTable::Builder>& arrTable,
                   int num_threads = 0,
                   std::promise<void>* arrTableReady = nullptr);

//////////////////////////////////////////////////////////////////////

/*
 * Main entry point when the program should behave like hhbbc.
 */
int main(int argc, char** argv);

//////////////////////////////////////////////////////////////////////

// Export set of functions to check for symbol uniqueness. This lets
// hphpc use the same logic and keep the same error messages.

struct NonUniqueSymbolException : std::exception {
  explicit NonUniqueSymbolException(std::string msg) : msg(msg) {}
  const char* what() const noexcept override { return msg.c_str(); }
private:
  std::string msg;
};

template <typename T, typename R>
void add_symbol(R& map, T t, const char* type) {
  assertx(t->attrs & AttrUnique);
  assertx(t->attrs & AttrPersistent);

  auto const name = t->name;
  auto const ret = map.emplace(name, std::move(t));
  if (ret.second) return;

  auto const filename = [] (auto const& unit) -> std::string {
    if (!unit) return "BUILTIN";
    return unit->filename->toCppString();
  };

  throw NonUniqueSymbolException{
    folly::sformat(
      "More than one {} with the name {}. In {} and {}",
      type,
      t->name,
      filename(t->unit),
      filename(ret.first->second->unit)
    )
  };
}

template <typename T, typename E>
void validate_uniqueness(const T& t, const E& other) {
  auto const iter = other.find(t->name);
  if (iter == other.end()) return;

  auto const filename = [] (auto const& unit) -> std::string {
    if (!unit) return "BUILTIN";
    return unit->filename->toCppString();
  };

  throw NonUniqueSymbolException{
    folly::sformat(
      "More than one symbol with the name {}. In {} and {}",
      t->name,
      filename(t->unit),
      filename(iter->second->unit)
    ).c_str()
  };
}

template <typename T, typename R, typename E, typename F>
void add_symbol(R& map,
                T t,
                const char* type,
                const E& other1,
                const F& other2) {
  validate_uniqueness(t, other1);
  validate_uniqueness(t, other2);
  add_symbol(map, std::move(t), type);
}

//////////////////////////////////////////////////////////////////////

}}
