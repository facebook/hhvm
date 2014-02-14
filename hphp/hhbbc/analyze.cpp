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
#include "hphp/hhbbc/analyze.h"

#include "hphp/util/trace.h"

#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"

namespace HPHP { namespace HHBBC {

namespace {

TRACE_SET_MOD(hhbbc);

//////////////////////////////////////////////////////////////////////

State entry_state(const Index& index,
                  Context const ctx,
                  ClassAnalysis* clsAnalysis) {
  auto ret = State{};
  ret.initialized = true;
  ret.thisAvailable = index.lookup_this_available(ctx.func);
  ret.locals.resize(ctx.func->locals.size());

  auto locId = uint32_t{0};
  for (; locId < ctx.func->params.size(); ++locId) {
    // Parameters may be Uninit (i.e. no InitCell).  Also note that if
    // a function takes a param by ref, it might come in as a Cell
    // still if FPassC was used.
    ret.locals[locId] = ctx.func->params[locId].byRef ? TGen : TCell;
  }

  /*
   * Closures have a hidden local that's always the first local, which
   * stores the closure itself.
   */
  if (ctx.func->isClosureBody) {
    assert(locId < ret.locals.size());
    assert(ctx.func->cls);
    auto const rcls = index.resolve_class(ctx, ctx.func->cls->name);
    assert(rcls && "Closure classes must always be unique and must resolve");
    ret.locals[locId++] = objExact(*rcls);
  }

  for (; locId < ctx.func->locals.size(); ++locId) {
    /*
     * Generators and closures don't (necessarily) start with the
     * frame locals uninitialized.
     *
     * Ideas:
     *
     *  - maybe we can do better for generators by adding edges from
     *    the yields to the top of the generator
     *
     *  - for closures, since they are all unique to their creation
     *    sites and in the same unit, looking at the CreateCl could
     *    tell the types of used vars, even in single unit mode.
     */
    ret.locals[locId] =
      ctx.func->isGeneratorBody || ctx.func->isClosureBody ? TGen : TUninit;
  }

  return ret;
}

/*
 * Closures inside of classes are analyzed in the context they are
 * created in (this affects accessibility rules, access to privates,
 * etc).
 *
 * Note that in the interpreter code, ctx.func->cls is not
 * necessarily the same as ctx.cls because of closures.
 */
Context adjust_closure_context(Context ctx) {
  if (ctx.cls && ctx.cls->closureContextCls) {
    ctx.cls = ctx.cls->closureContextCls;
  }
  return ctx;
}

FuncAnalysis do_analyze(const Index& index,
                        Context const inputCtx,
                        ClassAnalysis* clsAnalysis) {
  assert(inputCtx.func != inputCtx.unit->pseudomain.get() &&
         "pseudomains not supported");
  FTRACE(2, "{:-^70}\n", "Analyze");

  auto const ctx = adjust_closure_context(inputCtx);
  FuncAnalysis ai(ctx);

  auto rpoId = [&] (borrowed_ptr<php::Block> blk) {
    return ai.bdata[blk->id].rpoId;
  };

  /*
   * Set of RPO ids that still need to be visited.
   *
   * Initially, we need each entry block in this list.  As we visit
   * blocks, we propagate states to their successors and across their
   * back edges---when state merges cause a change to the block
   * stateIn, we will add it to this queue so it gets visited again.
   */
  std::set<uint32_t> incompleteQ;

  /*
   * We need to initialize the states for all function entries
   * (i.e. each dv init and the main entry), and all of them count as
   * places the function could be entered, so they all must be visited
   * at least once (add them to incompleteQ).
   */
  {
    auto const entryState = entry_state(index, ctx, clsAnalysis);
    for (auto& param : ctx.func->params) {
      if (auto const dv = param.dvEntryPoint) {
        ai.bdata[dv->id].stateIn = entryState;
        incompleteQ.insert(rpoId(dv));
      }
    }
    ai.bdata[ctx.func->mainEntry->id].stateIn = entryState;
    incompleteQ.insert(rpoId(ctx.func->mainEntry));
  }

  // For debugging, count how many times basic blocks get interpreted.
  auto interp_counter = uint32_t{0};

  /*
   * Iterate until a fixed point.
   *
   * We know a fixed point must occur because types increase
   * monotonically.
   *
   * We may visit a block up to as many times as there are state
   * variables coming into the block (locals and eval stack slots),
   * times the height of the type lattice.
   *
   * Each time a stateIn for a block changes, we re-insert the block's
   * rpo ID in incompleteQ.  Since incompleteQ is ordered, we'll
   * always visit blocks with earlier RPO ids first, which hopefully
   * means less iterations.
   */
  while (!incompleteQ.empty()) {
    auto blk = ai.rpoBlocks[*begin(incompleteQ)];
    incompleteQ.erase(begin(incompleteQ));
    PropertiesInfo props(index, inputCtx, clsAnalysis);

    FTRACE(2, "block #{}\nin {}\n{}", blk->id,
      state_string(*ctx.func, ai.bdata[blk->id].stateIn),
      property_state_string(props));
    ++interp_counter;

    auto propagate = [&] (php::Block& target, const State& st) {
      FTRACE(2, "     -> {}\n", target.id);
      FTRACE(4, "target old {}\n",
        state_string(*ctx.func, ai.bdata[target.id].stateIn));

      if (merge_into(ai.bdata[target.id].stateIn, st)) {
        incompleteQ.insert(rpoId(&target));
      }
      FTRACE(4, "target new {}\n",
        state_string(*ctx.func, ai.bdata[target.id].stateIn));
    };

    auto mergeReturn = [&] (Type type) {
      ai.inferredReturn = union_of(ai.inferredReturn, type);
    };

    auto stateOut = ai.bdata[blk->id].stateIn;
    Interpreter interp { &index, ctx, props, blk, stateOut };
    interp.run(propagate, mergeReturn);
  }

  /*
   * If inferredReturn is TBottom, the callee didn't execute a return
   * at all.  (E.g. it unconditionally throws, or is an abstract
   * function body.)
   *
   * In this case, we leave the return type as TBottom, to indicate
   * the same to callers.
   */
  assert(ai.inferredReturn.subtypeOf(TGen));

  // For debugging, print the final input states for each block.
  FTRACE(2, "{}", [&] {
    auto const bsep = std::string(60, '=') + "\n";
    auto const sep = std::string(60, '-') + "\n";
    auto ret = folly::format(
      "{}function {}{} ({} block interps):\n{}",
      bsep,
      ctx.cls ? folly::format("{}::", ctx.cls->name->data()).str()
              : std::string(),
      ctx.func->name->data(),
      interp_counter,
      bsep
    ).str();
    for (auto& bd : ai.bdata) {
      ret += folly::format(
        "{}block {}:\nin {}",
        sep,
        ai.rpoBlocks[bd.rpoId]->id,
        state_string(*ctx.func, bd.stateIn)
      ).str();
    }
    ret += sep + bsep;
    ret += folly::format(
      "Inferred return type: {}\n", show(ai.inferredReturn)).str();
    ret += bsep;
    return ret;
  }());

  return ai;
}

//////////////////////////////////////////////////////////////////////

/*
 * In the case of HNI builtin classes, private properties are
 * allowed to be mutated by native code, so we may not see all the
 * modifications.
 *
 * We are allowed to assume the type annotation on the property is
 * accurate, although nothing is currently checking that this is the
 * case.  We handle this right now by doing inference as if it
 * couldn't be affected by native code, then assert the inferred
 * type is at least a subtype of the annotated type, and expanding
 * it to be the annotated type if it is bigger.
 */
void expand_hni_prop_types(ClassAnalysis& clsAnalysis) {
  auto relax_prop = [&] (const php::Prop& prop, PropState& propState) {
    auto it = propState.find(prop.name);
    if (it == end(propState)) return;

    /*
     * When HardTypeHints isn't on, we don't require the constraints
     * to actually match, and relax all the HNI types to Gen.  (This
     * is because extensions may wish to assign to properties after a
     * typehint guard, which is going to fail without this flag on.)
     */
    auto const hniTy =
      !options.HardTypeHints ? TGen : from_hni_constraint(prop.typeConstraint);
    if (it->second.subtypeOf(hniTy)) {
      it->second = hniTy;
      return;
    }

    std::fprintf(
      stderr,
      "HNI class %s::%s inferred property type (%s) doesn't "
        "match annotation\n",
      clsAnalysis.ctx.cls->name->data(),
      prop.name->data(),
      show(it->second).c_str()
    );
    always_assert(!"HNI property type annotation was wrong");
  };

  for (auto& prop : clsAnalysis.ctx.cls->properties) {
    relax_prop(prop, clsAnalysis.privateProperties);
    relax_prop(prop, clsAnalysis.privateStatics);
  }
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

FuncAnalysis::FuncAnalysis(Context ctx)
  : ctx(ctx)
  , rpoBlocks(rpoSortAddDVs(*ctx.func))
  , bdata(ctx.func->blocks.size())
  , inferredReturn(TBottom)
{
  for (auto rpoId = size_t{0}; rpoId < rpoBlocks.size(); ++rpoId) {
    bdata[rpoBlocks[rpoId]->id].rpoId = rpoId;
  }
}

FuncAnalysis analyze_func(const Index& index, Context const ctx) {
  Trace::Bump bumper{Trace::hhbbc, kSystemLibBump,
    is_systemlib_part(*ctx.unit)};
  return do_analyze(index, ctx, nullptr);
}

ClassAnalysis analyze_class(const Index& index, Context const ctx) {
  Trace::Bump bumper{Trace::hhbbc, kSystemLibBump,
    is_systemlib_part(*ctx.unit)};
  assert(ctx.cls && !ctx.func);
  FTRACE(2, "{:#^70}\n", "Class");

  ClassAnalysis clsAnalysis(ctx);
  auto const associatedClosures = index.lookup_closures(ctx.cls);
  auto const isHNIBuiltin       = ctx.cls->attrs & AttrBuiltin;

  /*
   * Initialize inferred private property types to their in-class
   * initializers.
   *
   * We need to loosen_statics and loosen_values on instance
   * properties, because the class could be unserialized, which we
   * don't guarantee preserves those aspects of the type.
   *
   * Also, set Uninit properties to TBottom, so that analysis
   * of 86pinit methods sets them to the correct type.
   */
  for (auto& prop : ctx.cls->properties) {
    if (!(prop.attrs & AttrPrivate)) continue;

    auto const cellTy = from_cell(prop.val);
    if (isHNIBuiltin) {
      auto const hniTy = from_hni_constraint(prop.typeConstraint);
      if (!cellTy.subtypeOf(hniTy)) {
        std::fprintf(stderr, "hni %s::%s has impossible type\n",
                     ctx.cls->name->data(),
                     prop.name->data());
        always_assert(0 && "HNI systemlib has invalid type annotations");
      }
    }

    if (!(prop.attrs & AttrStatic)) {
      auto t = loosen_statics(loosen_values(cellTy));
      if (!is_closure(*ctx.cls) && t.subtypeOf(TUninit)) {
        /*
         * For non-closure classes, a property of type KindOfUninit
         * means that it has non-scalar initializer which will be set
         * by a 86pinit method.  For these classes, we want the
         * initial type of the property to be the type set by the
         * 86pinit method, so we set the type to TBottom.
         *
         * Closures will not have an 86pinit body, but still may have
         * properties of kind KindOfUninit (they will later contain
         * used variables or static locals for the closure body).  We
         * don't want to touch those.
         */
        t = TBottom;
      }
      clsAnalysis.privateProperties[prop.name] = t;
    } else {
      // Same thing as the above regarding TUninit and TBottom.
      // Static properties don't need to exclude closures for this,
      // though---we use instance properties for the closure
      // 86static_* properties.
      auto t = cellTy;
      if (t.subtypeOf(TUninit)) {
        t = TBottom;
      }
      clsAnalysis.privateStatics[prop.name] = t;
    }
  }

  /*
   * For classes with non-scalar initializers, the 86pinit and 86sinit
   * methods are guaranteed to run before any other method, and
   * are never called afterwards. Thus, we can analyze these
   * methods first to determine the initial types of properties with
   * non-scalar initializers, and these need not be be run again as part
   * of the fixedpoint computation.
   */
  if (auto f = find_method(ctx.cls, s_86pinit.get())) {
    do_analyze(
      index,
      Context { ctx.unit, f, ctx.cls },
      &clsAnalysis
    );
  }
  if (auto f = find_method(ctx.cls, s_86sinit.get())) {
    do_analyze(
      index,
      Context { ctx.unit, f, ctx.cls },
      &clsAnalysis
    );
  }

  // Verify that none of the class properties are TBottom, i.e.
  // any property of type KindOfUninit has been initialized (by
  // 86pinit or 86sinit).
  for (auto& prop : ctx.cls->properties) {
    if (!(prop.attrs & AttrPrivate)) continue;
    if (prop.attrs & AttrStatic) {
      assert(!clsAnalysis.privateStatics[prop.name].subtypeOf(TBottom));
    } else {
      assert(!clsAnalysis.privateProperties[prop.name].subtypeOf(TBottom));
    }
  }

  for (;;) {
    auto const previousProps   = clsAnalysis.privateProperties;
    auto const previousStatics = clsAnalysis.privateStatics;

    std::vector<FuncAnalysis> methodResults;
    std::vector<FuncAnalysis> closureResults;

    // Analyze every method in the class until we reach a fixed point
    // on the private property states.
    for (auto& f : ctx.cls->methods) {
      if (f->isAsync && f->isGeneratorBody) {
        /*
         * Inner-bodies of async functions don't need to have their
         * inner body analyzed for class analysis, because it is
         * required to do the same thing as the eager-execution
         * version.
         */
        continue;
      }
      if (f->name->isame(s_86pinit.get()) ||
          f->name->isame(s_86sinit.get())) {
        continue;
      }

      methodResults.push_back(
        do_analyze(
          index,
          Context { ctx.unit, borrow(f), ctx.cls },
          &clsAnalysis
        )
      );
    }

    for (auto& c : associatedClosures) {
      auto const invoke = borrow(c->methods[0]);
      closureResults.push_back(
        do_analyze(
          index,
          Context { ctx.unit, invoke, c },
          &clsAnalysis
        )
      );
    }

    // Check if we've reached a fixed point yet.
    if (previousProps   == clsAnalysis.privateProperties &&
        previousStatics == clsAnalysis.privateStatics) {
      clsAnalysis.methods  = std::move(methodResults);
      clsAnalysis.closures = std::move(closureResults);
      break;
    }
  }

  if (isHNIBuiltin) expand_hni_prop_types(clsAnalysis);

  // For debugging, print the final state of the class analysis.
  FTRACE(2, "{}", [&] {
    auto const bsep = std::string(60, '+') + "\n";
    auto ret = folly::format(
      "{}class {}:\n{}",
      bsep,
      ctx.cls->name->data(),
      bsep
    ).str();
    for (auto& kv : clsAnalysis.privateProperties) {
      ret += folly::format(
        "private ${: <14} :: {}\n",
        kv.first->data(),
        show(kv.second)
      ).str();
    }
    for (auto& kv : clsAnalysis.privateStatics) {
      ret += folly::format(
        "private static ${: <14} :: {}\n",
        kv.first->data(),
        show(kv.second)
      ).str();
    }
    ret += bsep;
    return ret;
  }());

  return clsAnalysis;
}

//////////////////////////////////////////////////////////////////////

std::vector<State>
locally_propagated_states(const Index& index,
                          const Context ctx,
                          borrowed_ptr<const php::Block> blk,
                          State state) {
  Trace::Bump bumper{Trace::hhbbc, 10};

  std::vector<State> ret;
  ret.reserve(blk->hhbcs.size());

  PropertiesInfo props(index, ctx, nullptr);
  Interpreter interp { &index, ctx, props, blk, state };
  for (auto& op : blk->hhbcs) {
    ret.push_back(state);
    interp.step(op);
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

}}
