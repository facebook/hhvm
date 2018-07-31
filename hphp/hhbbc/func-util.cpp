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
#include "hphp/hhbbc/func-util.h"

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"

#include "hphp/runtime/vm/func.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

const StaticString s_http_response_header("http_response_header");
const StaticString s_php_errormsg("php_errormsg");
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
  return l.name->same(s_http_response_header.get()) ||
         l.name->same(s_php_errormsg.get()) ||
         l.name->same(s_86metadata.get());
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

std::unique_ptr<ExnNode> cloneExnTree(
  ExnNode* in,
  BlockId delta,
  hphp_fast_map<ExnNode*, ExnNode*>& processed) {

  auto clone = std::make_unique<ExnNode>();
  always_assert(!processed.count(in));
  processed[in] = clone.get();

  clone->id = in->id;
  clone->depth = in->depth;
  clone->parent = in->parent ? processed[in->parent] : nullptr;
  clone->info = in->info;
  for (auto& child : in->children) {
    clone->children.push_back(cloneExnTree(child.get(), delta, processed));
  }
  if (delta) {
    match<void>(clone->info,
                [&](php::FaultRegion& fr) {
                  fr.faultEntry += delta;
                },
                [&](php::CatchRegion& cr) {
                  cr.catchEntry += delta;
                });
  }
  return clone;
}

void fixupSwitch(SwitchTab& s, BlockId delta) {
  for (auto& id : s) id += delta;
}

void fixupSwitch(SSwitchTab& s, BlockId delta) {
  for (auto& ent : s) ent.second += delta;
}

// generic do-nothing function, thats an inexact match
template <typename Opcode>
void fixupBlockIds(const Opcode& /*op*/, bool) {}

// exact match if there's a targets field with matching fixupSwitch
template<typename Opcode>
auto fixupBlockIds(Opcode& op, BlockId delta) ->
  decltype(fixupSwitch(op.targets, delta)) {
  return fixupSwitch(op.targets, delta);
}

// exact match if there's a target field
template<typename Opcode>
auto fixupBlockIds(Opcode& op, BlockId delta) -> decltype(op.target, void()) {
  op.target += delta;
}

void fixupBlockIds(Bytecode& bc, BlockId delta) {
#define O(opcode, ...) case Op::opcode: return fixupBlockIds(bc.opcode, delta);
  switch (bc.op) { OPCODES }
#undef O
}

void copy_into(php::FuncBase* dst, const php::FuncBase& other) {
  hphp_fast_map<ExnNode*, ExnNode*> processed;

  BlockId delta = dst->blocks.size();
  for (auto& theirs : other.exnNodes) {
    auto ours = cloneExnTree(theirs.get(), delta, processed);
    dst->exnNodes.push_back(std::move(ours));
  }

  for (auto& theirs : other.blocks) {
    auto ours = std::make_unique<php::Block>(*theirs);
    if (theirs->exnNode) {
      ours->exnNode = processed[theirs->exnNode];
      assertx(ours->exnNode);
    }
    if (delta) {
      ours->id += delta;
      if (ours->fallthrough != NoBlockId) ours->fallthrough += delta;
      for (auto &id : ours->throwExits) id += delta;
      for (auto &id : ours->unwindExits) id += delta;
      for (auto& bc : ours->hhbcs) {
        // When merging functions (used for 86xints) we have to drop
        // the src info, because it might reference a different unit
        // (and as a generated function, the src info isn't very
        // meaningful anyway).
        bc.srcLoc = -1;
        fixupBlockIds(bc, delta);
      }
    }
    dst->blocks.push_back(std::move(ours));
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
    b->hhbcs.back() = bc::PopC {};
    b->fallthrough = dst->blocks.size();
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
