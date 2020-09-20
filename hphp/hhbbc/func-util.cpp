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
#include "hphp/hhbbc/wide-func.h"

#include "hphp/runtime/vm/func.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

const StaticString s_reified_generics_var("0ReifiedGenerics");

//////////////////////////////////////////////////////////////////////

uint32_t closure_num_use_vars(const php::Func* f) {
  // Properties on the closure object are use vars.
  return f->cls->properties.size();
}

bool is_volatile_local(const php::Func* func, LocalId lid) {
  auto const& l = func->locals[lid];
  if (!l.name) return false;

  return (RuntimeOption::EnableArgsInBacktraces &&
          l.name->same(s_reified_generics_var.get())) ||
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

  auto& params = func->params;
  auto size = params.size();
  if (nArgs > size) {
    return size > 0 && params[size - 1].isVariadic;
  }
  return true;
}

int dyn_call_error_level(const php::Func* func)  {
  auto const def = [&] {
    if (!(func->attrs & AttrDynamicallyCallable) ||
        RuntimeOption::EvalForbidDynamicCallsWithAttr) {
      if (func->cls) {
        if (func->attrs & AttrStatic) {
          return RuntimeOption::EvalForbidDynamicCallsToClsMeth;
        }
        return RuntimeOption::EvalForbidDynamicCallsToInstMeth;
      }
      return RuntimeOption::EvalForbidDynamicCallsToFunc;
    }
    return 0;
  }();

  if (def > 0 && func->sampleDynamicCalls) return 1;
  return def;
}

//////////////////////////////////////////////////////////////////////

namespace {

void copy_into(php::WideFunc& dst, const php::WideFunc& src) {
  assertx(!src.blocks().empty());
  assertx(!dst.blocks().empty());
  always_assert(src->exnNodes.empty() || dst->exnNodes.empty());

  auto const delta = dst.blocks().size();
  dst->exnNodes.reserve(dst->exnNodes.size() + src->exnNodes.size());
  for (auto en : src->exnNodes) {
    en.region.catchEntry += delta;
    dst->exnNodes.push_back(std::move(en));
  }
  for (auto src_block : src.blocks()) {
    auto const dst_block = src_block.mutate();
    if (dst_block->fallthrough != NoBlockId) dst_block->fallthrough += delta;
    if (dst_block->throwExit != NoBlockId)   dst_block->throwExit += delta;
    for (auto& bc : dst_block->hhbcs) {
      // When merging functions (used for 86xints) we have to drop the srcLoc,
      // because it might reference a different unit. Since these functions
      // are generated, the srcLoc isn't that meaningful anyway.
      bc.srcLoc = -1;
      bc.forEachTarget([&] (BlockId& b) { b += delta; });
    }
    dst.blocks().push_back(std::move(src_block));
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

bool append_func(php::Func* dst, const php::Func& src) {
  if (src.numIters || src.locals.size()) return false;
  if (src.exnNodes.size() && dst->exnNodes.size()) return false;

  auto const src_func = php::WideFunc::cns(&src);
  auto dst_func = php::WideFunc::mut(dst);

  bool ok = false;
  for (auto const bid : dst_func.blockRange()) {
    auto const& cblk = dst_func.blocks()[bid];
    if (cblk->hhbcs.back().op != Op::RetC) continue;
    auto const blk = dst_func.blocks()[bid].mutate();
    blk->hhbcs.back() = bc::PopC {};
    blk->fallthrough = dst_func.blocks().size();
    ok = true;
  }
  if (!ok) return false;
  copy_into(dst_func, src_func);
  return true;
}

BlockId make_block(php::WideFunc& func, const php::Block* srcBlk) {
  auto newBlk    = copy_ptr<php::Block>{php::Block{}};
  auto const blk = newBlk.mutate();
  blk->exnNodeId = srcBlk->exnNodeId;
  blk->throwExit = srcBlk->throwExit;
  auto const bid = func.blocks().size();
  func.blocks().push_back(std::move(newBlk));
  return bid;
}

php::FuncBase::FuncBase(const FuncBase& other) {
  always_assert(!other.nativeInfo);
  exnNodes = other.exnNodes;
  rawBlocks = other.rawBlocks;
}

//////////////////////////////////////////////////////////////////////

}}
