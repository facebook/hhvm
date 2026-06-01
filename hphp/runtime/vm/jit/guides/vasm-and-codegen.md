# Vasm, Code Generation, and Relocation

> Sections marked with 📋 list specific names that may drift as code evolves. If a name isn't found, search for the pattern nearby.

## Vasm (Virtual Assembly)

Vasm is the IR between HHIR lowering and register allocation. A `Vunit` contains
`Vblock`s of `Vinstr`s.

### Three code areas

Each `Vblock` is tagged with an `AreaIndex`: **Main** (hot), **Cold** (unlikely
paths), **Frozen** (rarely-used, e.g., translation stubs and data). Code layout
respects these areas. Block weight (from profiling data) drives layout decisions.

### Vasm calling convention instructions

Three distinct ABIs are encoded as instruction pairs:
- `call` / `ret` (native C++ calls)
- `stublogue` / `stubret` (unique stub calls)
- `phplogue` / `phpret` (PHP function calls)

**Mixing these up corrupts the stack.**

### Vreg numbering

Virtual registers start at `Vreg::V0`. Physical registers are encoded directly.
`Vunit::next_vr` tracks the next available vreg.

### 📋 Key files

| File | Role |
|------|------|
| `vasm-unit.h` | `Vunit`, `Vblock`, `Vconst`, `Vframe` definitions |
| `vasm-gen.h` | `Vout` (instruction emitter), `Vasm` (manages main/cold/frozen) |
| `vasm-instr.h` | `VASM_OPCODES` macro — all vasm instructions |
| `vasm.h` | Optimization pass declarations, block sorting |
| `vasm-simplify.cpp` | Vasm-level peephole simplifications |
| `vasm-copy.cpp` | Copy propagation |
| `vasm-dead.cpp` | Dead code elimination |
| `vasm-graph-color.cpp` | Register allocation |
| `vasm-emit.cpp` | Vasm → machine code |

## Code Generation

Architecture-specific code emission:
- `code-gen-x64.cpp` — x86-64 backend
- `code-gen-arm.cpp` — AArch64 backend

## Relocation

After code is emitted into a temporary buffer, it's relocated into the final TC
location. Relocation may change instruction sizes (e.g., far branch → near
branch).

Four operations:
- `relocate()` — copy instructions to new location, possibly changing size
- `adjustForRelocation()` — fix addresses in moved instruction ranges
- `adjustCodeForRelocation()` — fix addresses in live code pointing to moved ranges
- `adjustMetaDataForRelocation()` — fix fixup map, debug info, etc.

**Smashable relocations** need special tracking since they may be patched at
runtime (smashable jumps/calls). Tracked via `markSmashableRelocation()`.

**Address immediates**: Instructions that load addresses (not just PC-relative
offsets) must be tracked in `addressImmediates` so relocation can fix them.

Key files: `relocation.h`, `relocation.cpp`, `relocation-x64.cpp`,
`relocation-arm.cpp`.

## Vasm Block Counters

Used for PGO. During serialization mode, `decqmlocknosf` instructions are
inserted at the start of each Vasm basic block to count executions. During
deserialization, the collected counts are applied as block weights via
`setWeights()` for better code layout decisions.

Key files: `vasm-block-counters.cpp`, `vasm-block-counters.h`.

## Pitfalls

- **`allocData()` allocates data blocks relocated alongside code** — `VdataPtr` members are automatically fixed up during relocation
- **`checkWidths()`** should only be run before simplify/lowering passes reduce zero-extending copies
- **`checkNoSideExits()`** — after certain passes, the unit must not contain instructions that implicitly leave (jcci, fallbackcc, bindjcc)
- **Block weight must be set accurately** from profiling data — it drives hot/cold splitting and code layout
