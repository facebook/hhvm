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
#include "hphp/hhbbc/func-util.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"

#include "hphp/runtime/vm/func.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

const StaticString s_86metadata("86metadata");

//////////////////////////////////////////////////////////////////////

uint32_t closure_num_use_vars(const php::Func* f) {
  // Properties on the closure object are either use vars, or storage
  // for static locals.  The first N are the use vars.
  return f->cls->properties.size() - f->staticLocals.size();
}

bool is_pseudomain(const php::Func* f) {
  return f->unit->pseudomain.get() == f;
}

bool is_volatile_local(const php::Func* func,
                       LocalId lid) {
  if (is_pseudomain(func)) return true;
  // Note: unnamed locals in a pseudomain probably are safe (i.e. can't be
  // changed through $GLOBALS), but for now we don't bother.
  auto const& l = func->locals[lid];
  if (!l.name) return false;
  return l.name->same(s_86metadata.get());
}

SString memoize_impl_name(const php::Func* func) {
  always_assert(func->isMemoizeWrapper);
  return Func::genMemoizeImplName(func->name);
}

bool check_nargs_in_range(const php::Func* func, uint32_t nArgs) {
  while (nArgs < func->dvEntries.size()) {
    if (func->dvEntries[nArgs++] == NoBlockId) return false;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

namespace {

using ExnNode = php::ExnNode;

void copy_into(php::FuncBase* dst, const php::FuncBase& other) {
  hphp_fast_map<ExnNode*, ExnNode*> processed;

  BlockId delta = dst->blocks.size();
  always_assert(!dst->exnNodes.size() || !other.exnNodes.size());
  dst->exnNodes.reserve(dst->exnNodes.size() + other.exnNodes.size());
  for (auto en : other.exnNodes) {
    if (delta) {
      match<void>(en.info,
                  [&](php::FaultRegion& fr) {
                    fr.faultEntry += delta;
                  },
                  [&](php::CatchRegion& cr) {
                    cr.catchEntry += delta;
                  });
    }
    dst->exnNodes.push_back(std::move(en));
  }
  for (auto theirs : other.blocks) {
    if (delta) {
      auto const ours = theirs.mutate();
      if (ours->fallthrough != NoBlockId) ours->fallthrough += delta;
      for (auto &id : ours->throwExits) id += delta;
      for (auto &id : ours->unwindExits) id += delta;
      for (auto& bc : ours->hhbcs) {
        // When merging functions (used for 86xints) we have to drop
        // the src info, because it might reference a different unit
        // (and as a generated function, the src info isn't very
        // meaningful anyway).
        bc.srcLoc = -1;
        bc.forEachTarget([&] (BlockId& b) { b += delta; });
      }
    }
    dst->blocks.push_back(std::move(theirs));
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

bool append_func(php::Func* dst, const php::Func& src) {
  if (src.numIters || src.locals.size()) return false;
  if (src.exnNodes.size() && dst->exnNodes.size()) return false;

  bool ok = false;
  for (auto& b : dst->blocks) {
    if (b->hhbcs.back().op != Op::RetC) continue;
    auto const blk = b.mutate();
    blk->hhbcs.back() = bc::PopC {};
    blk->fallthrough = dst->blocks.size();
    ok = true;
  }
  if (!ok) return false;
  copy_into(dst, src);
  return true;
}

php::FuncBase::FuncBase(const FuncBase& other) {
  copy_into(this, other);

  assertx(!other.nativeInfo);
}

//////////////////////////////////////////////////////////////////////

}}
