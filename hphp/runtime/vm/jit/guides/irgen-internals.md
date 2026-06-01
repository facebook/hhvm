# IRGen Internals — IRBuilder, FrameState, Inlining, Side Exits

> Sections marked with 📋 list specific names that may drift as code evolves. If a name isn't found, search for the pattern nearby.

## IRBuilder

`IRBuilder` (`ir-builder.h`) is the central coordinator for IR construction.
When `gen()` is called, `optimizeInst()` runs two passes before appending:

1. **`preOptimize()`** — state-aware: eliminates redundant guards (if FrameState
   already knows the type), folds loads from known values
2. **`simplify()`** — state-independent: strength reduction, constant folding,
   copy propagation

After appending, `m_state.update()` tracks the instruction's effects on locals,
stack, and the member base register.

**Guard constraint tracking**: `constrainGuard()`, `constrainValue()`,
`constrainLocation()` trace back to the guard that provided a type and ensure
it won't be relaxed beyond what the consumer needs.

Key file: `ir-builder.h`.

### Pitfalls

- `preOptimize()` can convert guards to nops via `fwdGuardSource()` — this
  removes the instruction and forwards the unguarded SSATmp
- `exceptionStackBoundary()` must be called if an HHBC opcode modifies the
  stack before it could throw — otherwise catch blocks will have wrong stack state

## FrameStateMgr

`FrameStateMgr` (`frame-state.h`) tracks per-frame state during IR construction:
types/values for locals, stack slots, the member base register, `$this`/ctx,
and frame/stack pointers.

**`fpValue` vs `fixupFPValue`** — the most common source of confusion:
- `fpValue` is the *logical* frame pointer (may be an inlined frame not yet
  pushed to memory)
- `fixupFPValue` is the *physical* frame pointer (matches the `rvmfp()` register)

Confusing these causes fixup bugs.

**`stackModified` flag** must be false when catch traces are created (unless
`exceptionStackBoundary()` was called). Otherwise the catch trace will have
wrong stack state.

Key file: `frame-state.h`.

## Side Exits

Side exits leave compiled code and return to the interpreter or request a new
translation. Key functions in `irgen-exit.h`:

| Function | When to use |
|----------|-------------|
| `makeExit(sk)` | Guard failure or branch leaving the region — emits `ReqBindJmp` |
| `makeExitSlow()` | Can't handle this case — interpret current bytecode then exit |
| `makeExitSurprise(sk)` | Handle surprise flags (async signals, OOM), then exit |
| `makeExitOpt()` | Request a PGO upgrade (retranslate-opt). **Never inside inlined frames.** |
| `makeUnreachable(reason)` | Debug assertion for code that should never execute |

All exit blocks are created with `Block::Hint::Unlikely`.

**First-instruction guard failures** emit `ReqRetranslate` (try different types).
**Mid-trace exits** emit `ReqBindJmp` (bind to a specific SrcKey).

Key files: `irgen-exit.h`, `irgen-exit.cpp`.

## Inlining

The inlining subsystem decides whether to inline function calls, manages
entry/exit of inlined frames, and stitches callee IRUnits into the caller.

### 📋 Key files

| File | Role |
|------|------|
| `inlining-decider.h/.cpp` | Policy: `canInlineAt()` (shallow checks), `shouldInline()` (cost model) |
| `irgen-inlining.h/.cpp` | IR generation for inlined frames, side exits from inlined code |
| `inline-state.h` | `InlineFrame`/`InlineState` data structures |
| `inline-stitching.h/.cpp` | `stitchCalleeUnit()` — merging callee IR into caller |
| `irlower-inlining.cpp` | Lowering of `DefCalleeFP`, `InlineCall`, `LeaveInlineFrame` |

### Cost model

Inlining cost is measured in vasm instructions (`computeTranslationCostSlow`).
The budget is dynamic, scaled by profile data:

```
baseVasmCost * (callerProfCount/baseProfCount)^callerExp
             * (callerProfCount/calleeProfCount)^calleeExp
             * (1/(1+depth))^depthExp
```

Clamped between `AlwaysInlineVasmCostLimit` and `InliningMaxVasmCostLimit`.
`__ALWAYS_INLINE` bypasses cost; `__NEVER_INLINE` prevents inlining entirely.

### Critical invariants

- Every callee region must contain a single `DefCalleeFP` that dominates all
  callee instructions
- `LeaveInlineFrame` must post-dominate all callee instructions (excluding
  side exits / early returns)
- Side exits from inlined code must publish frames using `InlineCall` before
  exiting — `InlineCall` stores `m_sfp`, `m_savedRip`, and pushes to `rvmfp`
- Inlined frames may not be materialized in memory initially —
  `spillInlinedFrames()` materializes them when needed (e.g., before a side exit)

## Optimization Pass Ordering

`opt.cpp` defines the order of all HHIR optimization passes. The ordering has
subtle dependencies — adding passes in the wrong place causes miscompilation.

### 📋 Pass sequence (from `optimize()`)

1. Initial DCE
2. Prediction optimization
3. Simplification + DCE + cleanCfg
4. CheckType optimization (iterates to fixpoint with simplify + optimizePhis)
5. GVN + DCE, then refineTmps if GVN changed anything
6. **Load/store/phi loop** (repeats until stable):
   - `optimizeLoads` + DCE
   - `refineTmpsPass`
   - `lowerBespokes` (if load-elim propagated new array layout info) — triggers re-simplify
   - `optimizeStores` + DCE
   - `optimizePhis` + DCE — if Phis changed, loop again
7. Refcount optimization + DCE
8. `simplifyOrdStrIdx` (fuse `StringGet` + `OrdStr`)
9. Def sinking + DCE
10. Final bespoke lowering
11. Final cleanup (cleanCfg + GVN + refineTmps + optimizeCheckTypes)
12. Block hint fixing (no block hotter than its predecessors)
13. Selective DecRef weakening (Optimize only)

### Pitfalls

- `lowerBespokes` changes CFG structure — must be followed by simplify + cleanCfg
- Several passes (GVN, loads, stores, refcounts, sinkDefs, weakenDecRefs) are
  **skipped for Profile translations** to keep profiling fast
- `optimizePhis` returning true triggers re-iteration of the load/store loop
- `mandatoryPropagation` (copyProp + constProp + retypeDests) runs if
  simplification is disabled — it's the minimum needed for correctness

Key file: `opt.cpp`.
