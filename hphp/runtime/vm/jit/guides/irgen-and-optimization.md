# IR Generation and Optimization — irgen, simplify, DCE, memory effects

## Cross-File Connections

**When tracing how a JIT IR opcode works end-to-end:**
1. `irgen-*.cpp` — emits the IR instruction
2. `simplify.cpp` — compile-time folding (e.g., `simplifyConcatStrStr()` folds literals)
3. `dce.cpp` — dead code elimination + instruction combining (e.g., two `ConcatStrStr` → `ConcatStr3`)
4. `memory-effects.cpp` — declares read/write/kill effects for alias analysis
5. `native-calls.cpp` — maps IR op to C++ helper (e.g., `ConcatStrStr` → `concat_ss()`)
6. `irlower-*.cpp` — emits vasm for the instruction

**Runtime helper lookup:**
```
native-calls.cpp  →  vm/runtime.h (concat_ss, add_int_int, ...)  →  base/ primitives
```

## Common Patterns

### Adding memory effects for an IR opcode

**Getting memory effects wrong is a common source of miscompilation.** The
optimizer trusts these annotations completely — incorrect effects can cause
load/store reordering bugs, missed optimizations, or data corruption. AI agents
frequently get this wrong. Always study similar existing opcodes carefully.

```cpp
// memory-effects.cpp — in memory_effects_impl()
case LdMonotypeDictVal: {
  // Follow existing patterns (LdMonotypeVecElem, LdVecElem):
  auto const base = inst.src(0);
  auto const pos  = inst.src(1);
  return PureLoad {
    pos->hasConstVal() ? AElemI { base, pos->intVal() } : AElemIAny
  };
}
```

### Adding IR generation for a bytecode

```cpp
// irgen-*.cpp — one emitFoo per HHBC opcode
void emitMyOp(IRGS& env) {
  auto const val = topC(env, BCSPRelOffset{0});

  // Fall back to interpreter if types don't match what we can handle
  if (!val->isA(TStr)) {
    gen(env, Jmp, makeExitSlow(env));
    return;
  }

  auto const result = gen(env, MyIROp, val);
  popC(env);
  push(env, result);
}
```

### Adding a simplification rule

```cpp
// simplify.cpp — constant folding
SSATmp* simplifyMyOp(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal(TStr)) {
    // Evaluate at compile time
    auto const result = doTheOp(src->strVal());
    return cns(env, result);
  }
  return nullptr;  // no simplification
}
```

### Adding a native call binding

```cpp
// native-calls.cpp — maps IR op to C++ helper
{MyIROp,    callDest(env, inst), SSync, {{SSA, 0}, {SSA, 1}},
  &HPHP::my_runtime_helper},
```

### Adding vasm lowering

```cpp
// irlower-*.cpp
void cgMyIROp(IRLS& env, const IRInstruction* inst) {
  auto const src = srcLoc(env, inst, 0).reg();
  auto const dst = dstLoc(env, inst, 0).reg();
  auto& v = vmain(env);
  // ... emit vasm ...
}
```

## Type System

Types represent sets of values. Key rules:

```cpp
// RIGHT — subset check (handles constant types like Int<5>)
if (val->isA(TInt)) { ... }

// WRONG — equality fails for specialized types
if (val->type() == TInt) { ... }  // DON'T

// "Known to not be Int"
if (!val->type().maybe(TInt)) { ... }

// "Not known to be Int" (weaker)
if (!(val->type() <= TInt)) { ... }
```

Types form a lattice. Use `isA()` for subset checks, `maybe()` to test if two types
could share any values, `|` to widen, and `&` to narrow.

## ir.specification

`hphp/doc/ir.specification` is the HHIR opcode reference. **Don't load the
whole file** — grep for specific opcodes:

```bash
grep '^| CheckType' hphp/doc/ir.specification    # find an opcode
grep '^|.*PRc' hphp/doc/ir.specification          # refcount-producing ops
```

Entry format: `| OpcodeName<T>, D(RetType), S(SrcType) S(Src2Type), Flags`

Flags: `NF` (none), `PRc` (produces refcounted), `T` (terminal), `B` (branch),
`P` (passthrough), `LA` (layout-agnostic).

## Critical Patterns When Adding New irgen Paths

### Trace exit after type-changing push

When an irgen path pushes a result whose type differs from what the trace
assumed (e.g., pushing `TDbl` when the tracelet expected `TInt`), you **must**
terminate the trace with a side-exit:

```cpp
push(env, result);                              // result is TDbl, trace expected TInt
gen(env, Jmp, makeExit(env, nextSrcKey(env)));  // exit to retranslate at the next bytecode
return;
```

**How to spot this:** look at existing branches in the same `emitFoo` function.
If other branches end with `gen(env, Jmp, makeExit(env, nextSrcKey(env)))`,
your new branch almost certainly needs it too. The `emitPow` function in
`irgen-arith.cpp` has multiple examples of this pattern.

Omitting the side-exit lets the trace continue with a type assumption that no
longer holds, leading to miscompilation or assertion failures downstream.

### Simplifier assertions on constant inputs

`gen()` runs `simplify()` inline during irgen (via `IRBuilder::optimizeInst`).
If your constant operands can fold to values that violate simplifier
preconditions, the assertion fires at JIT compile time — not at runtime.

**Key example:** `DivDbl` asserts `src2Val != 0.0` (`simplify.cpp`). If a
`MulDbl` chain with constant inputs folds to `0.0` and feeds `DivDbl`, the
assert fires. This chain is non-obvious because the fold happens *inside*
`gen()`, invisible in your irgen code — you never see the intermediate constant.

**Defense:** Guard or bail to `interpOne` for edge-case constant inputs
*before* emitting the IR:

```cpp
// If the divisor could be zero after constant folding, bail out
if (src2->hasConstVal(TDbl) && src2->dblVal() == 0.0) {
  interpOne(env);
  return;
}
auto const result = gen(env, DivDbl, src1, src2);
```

This is especially important when composing multiple arithmetic IR ops in
sequence, because each `gen()` call may fold its result to a constant that
becomes the input to the next `gen()`.

## Pitfalls

- **Always update `memory-effects.cpp`** when adding/modifying IR ops that touch memory — optimizer will miscompile without correct effects
- **Use `makeExitSlow(env)` for interpreter fallback** — prefer `gen(env, Jmp, makeExitSlow(env)); return;` over `PUNT()`. If all possible inputs are handled, no fallback is needed.
- **Don't use `val->type() == TFoo`** — use `val->isA(TFoo)` (subset check handles constant types)
- **Don't eliminate IncRef/DecRef during initial IR generation** — guard relaxation may loosen types; wait for reoptimize pass
- **Don't use flow-sensitive types for `PtrTo*` inner types** — use flow-insensitive info from HHBBC
