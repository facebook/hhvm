# Optimization Pipeline and DCE

## optimize_func Pipeline

`do_optimize()` in `optimize.cpp` orchestrates a multi-pass pipeline:

1. Apply block updates from analysis
2. Iterator optimization (normal → local iterators when base is unmodified)
3. **Loop until stable:**
   - Dead block removal
   - Local DCE
   - Split critical edges
   - Global DCE
   - Control flow optimizations
   - Re-analyze (DCE changes types) → repeat if changed
4. Remove trivial `86*init` methods (null + retc)
5. Insert type assertions (`AssertRATL` / `AssertRATStk`)
6. Shrink bytecode vectors

**Re-analysis within the loop**: After global DCE changes types, `analyze_func`
runs again and the loop continues until no more changes occur.

### Iterator optimization

`optimize_iterators` converts normal iterators to local iterators when the base
local is not modified during the loop body.

### Unreachable code

Replaced with `StaticAnalysisError` bytecode (not removed, to preserve block
ID stability).

### Control flow opts

`control_flow_opts` (`cfg-opts.h`): simplifies CFG, creates `Switch`/`SSwitch`
bytecodes, removes unnecessary exception handlers. Critical edge split blocks
(nop-only) are folded back.

Key files: `optimize.cpp`, `cfg-opts.h`, `cfg-opts.cpp`.

## DCE Architecture

Two DCE modes run in sequence:
- **`local_dce`** — within a block, conservative with locals
- **`global_dce`** — cross-block, with liveness analysis

### Backward stack model

`Use` enum: `Used`, `Not`, `AddElemC` (for NewStructDict optimization),
`Linked` (use/def tied to stack slot below — all must be removed together).

### DceAction

11 rich actions beyond simple deletion:

| Action | Effect |
|--------|--------|
| `Kill` | Remove instruction entirely |
| `PopInputs` | Replace with pops of inputs |
| `PopOutputs` | Replace with pops of outputs |
| `Replace` | Replace with different bytecodes |
| `PopAndReplace` | Pop inputs + replace |
| `MinstrStackFinal` / `MinstrStackFixup` / `MinstrPushBase` | MInstr chain transformations |
| `AdjustPop` | Adjust pop count |
| `UnsetLocalsAfter` / `UnsetLocalsBefore` | Insert UnsetL around instruction |

### MInstr chain tracking

`minstrUI` is set by minstr-final instructions. If the entire minstr sequence
is non-PEI and the result is unused, the whole chain can be killed. If the
result is a constant, it can be replaced.

### Local slot remapping

Global DCE builds an interference graph between locals and remaps them to
reduce frame size. Pinned locals (used in local ranges) cannot be remapped.

### willBeUnsetLocals

Tracks locals that are dead and will be explicitly unset before any PEI. DCE
inserts `UnsetL` instructions to help the runtime avoid unnecessary decrefs.

Key files: `dce.cpp`, `dce.h`.

## Pitfalls

- **After global DCE, `FuncAnalysis` is invalid** — types must be recomputed (this is why the optimize loop re-analyzes)
- **Critical edge splitting is required** for correct `UnsetL` insertion by DCE
- **`StaticAnalysisError` preserves block IDs** — dead code is replaced, not removed
- **Local remapping can reduce frame size** but pinned locals constrain it
