# Translation Pipeline — PGO, Guards, Region Selection, RetranslateAll

> Sections marked with 📋 list specific names that may drift as code evolves. If a name isn't found, search for the pattern nearby.

## Translation Kinds

Translations have a `TransKind` (`types.h`): **Profile**, **Optimize**, or **Live**.

- **Profile** translations insert profiling counters and collect `TargetProfile`
  data into RDS. They use more conservative region selection.
- **Optimize** translations consume profile data to make better decisions about
  types, inlining, and code layout. Only produced during RetranslateAll.
- **Live** translations are produced without profiling — used when PGO is
  disabled or for code that doesn't benefit from profiling.

Each kind has a prologue variant. The kind determines which optimizations are
legal and which register set is available.

## Translation Driver

The full pipeline is driven by `tc::Translator` (`tc.h`), 5 steps:
(0) acquire lease and check preconditions, (1) translate (region select →
irgen → optimize → lower → vasm → emit), (2) relocate, (3) bind outgoing
edges, (4) publish.

**Locking hierarchy**: Translation requires the write lease (`LeaseHolder`),
then code lock (`tc::lockCode()`), then metadata lock (`tc::lockMetadata()`).
RTA bypasses normal locking by using thread-local buffers.

### 📋 Key files

| File | Role |
|------|------|
| `tc.h` | `tc::Translator` base class — defines the 5-step pipeline |
| `translator.cpp` | `TransContext`, `instrInfoSparse[]` (opcode stack shapes) |
| `translate-region.cpp` | `irGenRegion()` — main HHBC→HHIR driver, retry logic |
| `mcgen-translate.cpp` | `retranslateAll()`, `optimize()`, serialization |

## Region Selection

Regions define which HHBC ranges to compile together. `RegionDesc` (`region-selection.h`)
is the IR-level CFG of `RegionDesc::Block`s.

**Tracelet termination** — a tracelet (region) ends when:
- `opcodeBreaksBB()` returns true (control flow changes)
- In profiling mode, `instrBreaksProfileBB()` returns true (any side-exiting
  instruction, to keep profile counters accurate)
- An input has an unknown type requiring a new guard
- `maxBCInstrs` is reached

**PGO region modes** (`Hottrace`, `Hotblock`, `HotCFG`, `WholeCFG`) determine
how profiled blocks are stitched into optimized regions during RetranslateAll.

Key files: `region-selection.cpp`, `region-tracelet.cpp`.

## Guards

Guards are type checks at trace entry, emitted from
`RegionDesc::Block::typePreConditions()` by `emitGuards()` in
`translate-region.cpp`.

**Guard relaxation** weakens guards to the minimum needed by consumers. The
`GuardConstraint` (`guard-constraint.h`) tracks what each consumer actually
needs:

```
DataTypeGeneric < DataTypeCountness < DataTypeCountnessInit < DataTypeSpecific < DataTypeSpecialized
```

**Specialized types are checked in two steps**: first the unspecialized type
(e.g., `Obj`), then a `CheckType` for the specialization (e.g., `Obj<ClassName>`).
During region formation (`env.formingRegion == true`), specialization checking
is skipped — it only happens during actual compilation.

Key files: `irgen-guards.cpp`, `guard-constraint.h`, `guard-type-profile.cpp`.

## RetranslateAll (PGO recompilation)

Driven by `retranslateAll()` in `mcgen-translate.cpp`. Steps:
1. Compute function ordering via hfsort on the call graph
2. Bespoke array coloring
3. Finalize lazy APC classes
4. Optionally serialize profile data (jumpstart)
5. Generate optimized machine code in parallel (thread-local buffers)
6. Relocate all translations into TC in hfsort order

Optimized translations go into thread-local buffers first, then
`relocatePublishSortedOptFuncs()` relocates them into the global TC
to control code layout.

Key files: `mcgen-translate.cpp`, `tc-region.cpp`, `prof-data-serialize.cpp`.

## TargetProfile (two-phase profiling pattern)

```cpp
// target-profile.h — used in irgen
TargetProfile<MyProfileData> prof(env.context, env.irb->curMarker(), ...);

if (prof.profiling()) {
  // Profile translation: emit IR to collect data
  gen(env, ProfileMyThing, RDSHandleData{prof.handle()}, ...);
} else if (prof.optimizing()) {
  // Optimize translation: read accumulated data
  auto const data = prof.data();
  // ... use data to make better code generation decisions ...
}
```

**`reduce()`** folds data from ALL thread-local RDS bases. If a profile type
contains pointers, it must define `serialize()`/`deserialize()` methods.

Key files: `target-profile.h`, `prof-data.h`, `prof-data.cpp`.

## Pitfalls

- **Don't access `profData()` without null-checking** — it's discarded after RetranslateAll completes
- **Profile counter semantics**: `transCounter()` returns `initVal - counter` because counters decrement from a large initial value
- **`BlockId` encoding**: Non-negative BlockIds correspond to `TransProfile` translations (indexable into ProfData). Negative BlockIds are for inlined blocks. Getting this wrong corrupts profile data lookups.
- **TranslateRetryContext**: If irgen fails for a specific bytecode, `translate-region.cpp` retries with that instruction blacklisted to be interpreted. The `toInterp` set grows across retries.
