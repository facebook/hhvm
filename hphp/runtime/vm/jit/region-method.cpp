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
#include "hphp/util/arena.h"
#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/verifier/cfg.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(region);

//////////////////////////////////////////////////////////////////////

namespace {

bool isFuncEntry(const Func* func, Offset off) {
  return off == func->base();
}

int numInstrs(PC start, PC end) {
  int ret{};
  for (; start != end; ++ret) {
    start += instrLen((Op*)start);
  }
  return ret;
}

}

//////////////////////////////////////////////////////////////////////

/*
 * Region-selector that unintelligently takes a whole method at a
 * time.  This is primarily intended for use for debugging and
 * development on the JIT.
 *
 * Adds no type annotations to the region beyond those known from the
 * context.  It will list only the parameter types as guards.
 *
 * If the context is not a method entry point, returns nullptr to fall
 * back to the tracelet compiler.  (This will happen for side-exits
 * from method regions, for example.)
 */
RegionDescPtr selectMethod(const RegionContext& context) {
  using namespace HPHP::Verifier;

  if (!isFuncEntry(context.func, context.bcOffset)) return nullptr;
  FTRACE(1, "function entry for {}: using selectMethod\n",
         context.func->fullName()->data());

  auto ret = std::make_shared<RegionDesc>();

  Arena arena;
  GraphBuilder gb(arena, context.func);
  auto const graph = gb.build();
  auto const unit = context.func->unit();

  /*
   * Spit out the blocks in a RPO, but skip DV-initializer blocks
   * (i.e. start with graph->first_linear.  We don't handle those in
   * our method regions for now---they'll get handled by the tracelet
   * compiler and then may branch to the main entry point.
   */
  sortRpo(graph);
  Offset spOffset = context.spOffset;
  for (Block* b = graph->first_linear; b != nullptr; b = b->next_rpo) {
    auto const start  = unit->offsetOf(b->start);
    auto const length = numInstrs(b->start, b->end);
    ret->blocks.emplace_back(
      std::make_shared<RegionDesc::Block>(context.func, start, length,
                                          spOffset)
    );
    spOffset = -1; // flag SP offset as unknown for all but the first block
  }

  assert(!ret->blocks.empty());
  auto const startSK = ret->blocks.front()->start();
  for (auto& lt : context.liveTypes) {
    typedef RegionDesc::Location::Tag LTag;

    switch (lt.location.tag()) {
    case LTag::Stack:
      break;
    case LTag::Local:
      if (lt.location.localId() < context.func->numParams()) {
        // Only predict objectness, not the specific class type.
        auto const type = lt.type.strictSubtypeOf(Type::Obj)
                           ? Type::Obj
                           : lt.type;
        ret->blocks.front()->addPredicted(startSK, {lt.location, type});
      }
      break;
    }
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
