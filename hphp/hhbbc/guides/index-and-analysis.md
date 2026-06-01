# Index, Dependency System, and Whole-Program Analysis

> Sections marked with 📋 list specific names that may drift as code evolves. If a name isn't found, search for the pattern nearby.

## The Index

The Index (`index.h`) is the cross-function type database. All inter-procedural
queries go through the `IIndex` interface.

### Two Index implementations

- **`Index`** — whole-program, classic mode (single process)
- **`AnalysisIndex`** — per-worker in extern-worker mode (distributed)

Both implement `IIndex`. Code should use `IIndex&` for portability.

### res::Class and res::Func

**`res::Class`** is an opaque handle (`Either<void*, const StringData*>`) —
resolved (pointing to ClassInfo) or unresolved (just a name). Supports
subtype/couldBe queries with nonRegular flags for interfaces/traits/abstract
classes.

**`res::Func`** is a variant of ~14 representations: `FuncName` (just a name),
`Fun`/`Method` (resolved single function), `MethodFamily` (class hierarchy
family), `Isect` (intersection of families), `MissingFunc`/`MissingMethod`
(definitely doesn't exist). The variant determines how return type, param count,
etc. are resolved.

### Dependency Tracking

The `Dep` enum controls what gets tracked:

| Dep | Triggers re-analysis when... |
|-----|-----|
| `ReturnTy` | A callee's inferred return type changes |
| `ConstVal` | A constant's value changes |
| `ClsConst` | A class constant changes |
| `PropBadInitialValues` | Property initial value types change |
| `PublicSProp` | Public static property types change |
| `InlineDepthLimit` | Inline depth limit changes |

When the Index is queried, it records what was asked for, so dependents can be
re-analyzed when that information changes.

### Frozen vs unfrozen

- **Unfrozen** (during analysis): querying a callee's return type may
  recursively invoke type inference on the callee's body (inline analysis)
- **Frozen** (final pass): recursive analysis is forbidden because other threads
  may be modifying bytecode

## Fixed-Point Iteration

### Two execution modes

**Classic mode** (`analyze_iteratively` in `whole-program.cpp`): uses
`parallel::map` for analysis + single-threaded update loop.

**Extern-worker mode** (`AnalyzeJob`): runs in remote workers, each doing its
own fixed-point.

### Work items

- `WorkType::Class` — analyzes class + all methods + closures together (needed
  for private prop inference)
- `WorkType::Func` — standalone function

### Update step (single-threaded in classic mode)

After parallel analysis, the update step refines:
`refine_return_info`, `refine_constants`, `refine_class_constants`,
`update_prop_initial_values`, `refine_closure_use_vars`,
`refine_private_props`, `refine_private_statics`, `refine_public_statics`.

Each may add to the `DependencyContextSet` for re-analysis.

**Soundness invariant**: Information in the Index is never incorrect — it just
might not be as useful as it could be. Each piece of information starts
conservative and becomes more refined. No analysis pass should ever deduce an
incorrect fact from the Index, even during early iterations.

### Class-at-a-time analysis

`ClassAnalysisWorklist` has its OWN internal fixed-point loop with dependency
tracking on properties, return types, and closure use vars. Methods within a
class are re-analyzed until stable.

### Final pass

Index is frozen, then `optimize_func` is called on every function in parallel.
Bytecode may be mutated at this point.

## FuncAnalysisResult

Carries analysis output: `inferredReturn`, `effectFree`, `closureUseTypes`,
`publicSPropMutations`, `blockUpdates`, `usedParams`, `retParam`,
`resolvedInitializers`.

## Pitfalls

- **Don't introduce mutable shared state during parallel analysis** — analysis must be a pure function of immutable program + Index
- **Don't modify `index.cpp` without understanding the thread safety model** — concurrent reads during analysis, single-threaded writes during update
- **Don't assume Index types are final before fixed point** — early iterations are conservative
- **Don't manually construct UnitEmitters** to feed HHBBC — use `WholeProgramInput` API in `hhbbc.h`

Key files: `index.h`, `index.cpp`, `whole-program.cpp`, `analyze.h`.
