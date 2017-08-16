/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/php7/analysis.h"

#include "hphp/util/match.h"

namespace HPHP { namespace php7 {

namespace {

using bc::Local;
using bc::NamedLocal;
using bc::UniqueLocal;

struct LocalSet {
  void add(const Local& local) {
    match<void>(local,
      [&](const NamedLocal& named) {
        names.insert(named.name);
      },
      [&](const UniqueLocal& unique) {
        uniqueIds.insert(unique.id);
      }
    );
  }

  void allocateUniqueIds() {
    uint32_t id = names.size();
    for (auto& idPtr : uniqueIds) {
      *idPtr = id++;
    }
  }

  std::unordered_set<std::string> names;
  std::unordered_set<std::shared_ptr<uint32_t>> uniqueIds;
};

struct LocalsVisitor : CFGVisitor {
  explicit LocalsVisitor(LocalSet& locals)
    : locals(locals) {}

  void beginTry() override {}

  void beginCatch() override {}

  void endRegion() override {}

  void block(Block* blk) override {
    for (const auto& bc : blk->code) {
      bc.visit(*this);
    }
  }

  template <class T>
  void bytecode(const T& bc) {
    bc.visit_imms(*this);
  }

  void imm(Local local) {
    locals.add(local);
  }

  template <class T>
  void imm(const T&) {}

  LocalSet& locals;
};

} // namespace

std::unordered_set<std::string> analyzeLocals(const Function& func) {
  LocalSet locals;
  for (const auto& param : func.params) {
    locals.add(bc::NamedLocal{param.name});
  }
  func.cfg.visit(LocalsVisitor(locals));
  locals.allocateUniqueIds();
  return locals.names;
}

////////////////////////////////////////////////////////////////////////////////

}} // namespace HPHP::php7
