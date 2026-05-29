# Program Representation — php::*, WideFunc, Factored CFG

> Sections marked with 📋 list specific names that may drift as code evolves. If a name isn't found, search for the pattern nearby.

## Core Data Structures

| Struct | File | What it holds |
|--------|------|--------------|
| `php::Program` | `representation.h` | Three vectors: `funcs`, `classes`, `units` |
| `php::Class` | `representation.h` | Methods, properties, constants, closures |
| `php::Func` | `representation.h` | Params, locals, compressed blocks (`rawBlocks`) |
| `php::Block` | `representation.h` | Bytecodes, `fallthrough`, `throwExit`, flags |
| `php::Unit` | `representation.h` | Top-level functions, type aliases |
| `Bytecode` | `bc.h` | Sum type over all HHBC opcodes |

**`php::Program`** is the top-level container. Classes own their methods;
closures are stored in `ClassBase::closures`.

## Factored CFG

`php::Block` has three edge types:
- **`fallthrough`** — unconditional successor (next block)
- **`throwExit`** — exception edge (from PEI instructions)
- **taken** — encoded in the last instruction (branches, switches)

This is the "factored" CFG model: blocks do NOT end at every instruction that
could throw an exception (PEI). Instead, throw edges are attached to the block
as a whole. When types are known, many instructions can be proven non-throwing,
so this avoids splitting blocks unnecessarily.

**Block flags**: `catchEntry` (catch block), `multiPred` (multiple
predecessors), `multiSucc` (multiple successors), `dead` (marked dead by
optimization).

**Block IDs are indices** into the `BlockVec`. Dead blocks keep their ID —
they're marked `dead`, not removed.

**`copy_ptr` for COW**: Blocks may be shared between functions (trait
flattening). Mutations trigger COW via `mutate()`.

## WideFunc — Compressed Bytecode Accessor

**Bytecode is compressed at rest.** `php::Func::rawBlocks` is a
`CompressedBytecodePtr`. You MUST create a `WideFunc` to access blocks.

```cpp
// Read-only access
auto wf = WideFunc::cns(&func);
for (auto const& blk : wf.blocks()) { ... }

// Mutable access (writes back compressed on destruction)
auto wf = WideFunc::mut(&func);
wf.blocks()[0]->hhbcs.push_back(...);

// New function with empty blocks
auto wf = WideFunc::create(func);
```

**Non-copyable, non-movable.** Must be stack-allocated with short lifetime.
Call `release()` if the backing Func is destroyed while WideFunc is live.

Key file: `wide-func.h`.

## Context Types

Multiple context types exist for different phases (`context.h`):

| Type | When | Key fields |
|------|------|-----------|
| `Context` | General | unit, func, cls, dep |
| `AnalysisContext` | During analysis | Requires live `WideFunc` reference |
| `VisitContext` | During optimization | index, ainfo, collect, func |
| `CallContext` | Inline analysis | callee, arg types, context type |

**Closure adjustment**: `ctx.func->cls` may differ from `ctx.cls` because
closures run in their declaring class's context.

## CFG Utilities

`cfg.h` provides graph traversal used everywhere:

- `rpoSortFromMain` — RPO from main entry only (**excludes** DV initializers)
- `rpoSortAddDVs` — includes default value initializer entries
- `forEachSuccessor` / `forEachNormalSuccessor` / `forEachNonThrowSuccessor`
- `computeNonThrowPreds` / `computeThrowPreds`

Key file: `cfg.h`.
