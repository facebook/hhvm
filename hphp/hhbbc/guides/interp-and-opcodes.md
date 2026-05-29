# Interpreter and Opcode Handling

## Cross-File Connections

**When tracing how a bytecode is optimized ahead-of-time:**
1. `interp.cpp` — opcode handler (constant folding when operands are literal)
2. `type-ops.cpp` — type-level evaluation (`typeConcat()`, `typeAdd()`, etc.)
3. `optimize.cpp` — peephole simplifications (`hasObviousStackOutput()`)
4. `dce.cpp` — dead code elimination (marks bytecodes removable when pure + unused)

**Opcode combining** (in `interp.cpp`): binary op + `SetL` → `SetOpL`:
```
Concat + SetL → SetOpL{ConcatEqual}
Add + SetL    → SetOpL{PlusEqual}
```

## Common Patterns

### Adding type inference for a new opcode

```cpp
// interp.cpp — add handler in the switch
case bc::MyOp: {
  auto const arg = popC(env);
  // Constant fold if possible
  if (auto const val = tv(arg)) {
    auto const result = eval_my_op(*val);
    push(env, from_cell(result));
    return;
  }
  // Otherwise, infer result type
  push(env, TStr);  // result is always a string
}
```

### Adding constant folding in type-ops

```cpp
// type-ops.cpp
Type typeMyOp(const Type& a, const Type& b) {
  if (auto const va = tv(a)) {
    if (auto const vb = tv(b)) {
      return from_cell(do_my_op(*va, *vb));  // fold constants
    }
  }
  return TStr;  // conservative result type
}
```

### Runtime constant evaluation (eval_cell)

`eval_cell` (`eval-cell.h`) executes a lambda at compile time, catches
exceptions, makes the result static, then wraps it with `from_cell()`.
Returns `std::nullopt` on any exception. Used for constant folding function
calls.

## Interpreter State

The abstract interpreter uses several layered state objects:

**`State`** (`interp-state.h`): local types + eval stack (`InterpStack`) +
iterator states + `equivLocals` + `thisType` + `unreachable` flag.

**`ISS`** (Interpreter Step State, `interp-internal.h`): per-step context with
undo log, `replacedBcs` for bytecode rewriting, `trackedElems` for AddElemC →
NewStructDict optimization.

**`CollectedInfo`**: aggregates cross-block data — closure use vars, property
info, MInstr state (base tracking + array chains), public static prop mutations,
effect-free tracking.

### StepFlags

Key flags set by opcode handlers:
- `wasPEI` — default true; set false to mark instruction as non-throwing
- `canConstProp` — result can be replaced with a constant
- `effectFree` — instruction doesn't prevent function from being DCE'd
- `reduced` — already handled via `reduce()` (bytecode rewriting)

### InterpStack pitfalls

`InterpStack` uses an indirection layer so that pops don't destroy element data
(needed for the undo log). `State` has no `operator=` — use `copy_from()` or
`swap()` explicitly.

## Exception Edge Propagation

When the abstract interpreter encounters an instruction that could throw (a
PEI), it propagates the state **from before the instruction** across all throw
edges for the block. This is critical — the throw edge sees the pre-instruction
state, not the post-instruction state. Branch taken edges and fallthrough edges
see post-instruction state.

## Pitfalls

- **Don't include HHBBC headers** (other than `hhbbc.h`, `options.h`) from outside this directory
- **`kMaxTrackedLocals = 512`** — locals beyond this are always assumed live
- **Exception edges get pre-instruction state** — not post-instruction. Getting this wrong causes unsound type inference.
