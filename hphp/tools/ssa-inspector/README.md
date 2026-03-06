# SSA Inspector — HHVM tcprint IR Analysis Tool

## What This Tool Does

This tool queries HHVM's daily tcprint Dataswarm pipeline, which dumps the JIT
translation cache as JSON-encoded SSA IR for production web tenants. It parses
the IR and presents it in a format optimized for analysis by an LLM agent.

The data lives in Hive at `infrastructure.hhvm_tcprint_data`, partitioned by
date (`ds`) and run UUID. Each row contains one JIT translation's full SSA IR:
blocks, instructions with HHIR opcodes/types/SSATmps, phi nodes, inlining
decisions, machine code ranges with disassembly, and profile execution counts.

## Quick Start

```bash
# 1. List available runs (find recent data)
buck run fbcode//hphp/tools/ssa-inspector:main -- list-runs --days 7

# 2. Find the hottest translations in a run (includes Xenon gCPU %)
buck run fbcode//hphp/tools/ssa-inspector:main -- \
  top --date 2026-02-25 --run-uuid "2026-02-25.prod.web.cln.0-1" --top-n 10

# 2b. Skip Xenon lookup (faster, no Scuba query)
buck run fbcode//hphp/tools/ssa-inspector:main -- \
  top --date 2026-02-25 --run-uuid "2026-02-25.prod.web.cln.0-1" --top-n 10 \
  --xenon-hours 0

# 3. Inspect a specific translation in detail (shows gCPU % in header)
buck run fbcode//hphp/tools/ssa-inspector:main -- \
  inspect --trans-id 58753 \
  --date 2026-02-25 --run-uuid "2026-02-25.prod.web.cln.0-1" \
  --format detail

# 4. Search by function name
buck run fbcode//hphp/tools/ssa-inspector:main -- \
  inspect --function "contains_key" \
  --date 2026-02-25 --run-uuid "2026-02-25.prod.web.cln.0-1" \
  --format summary

# 5. Include machine code disassembly
buck run fbcode//hphp/tools/ssa-inspector:main -- \
  inspect --trans-id 58753 \
  --date 2026-02-25 --run-uuid "2026-02-25.prod.web.cln.0-1" \
  --format detail --disasm

# 6. Load from a local trace file (no Hive needed)
buck run fbcode//hphp/tools/ssa-inspector:main -- \
  inspect --local-file /tmp/trace.out --function "someFunc" --format detail
```

## Agent Workflow

When analyzing HHVM JIT output for optimization opportunities, follow this
sequence:

### Step 1: Find Hot Code

Run `top` to identify the hottest translations by execution count (profCount).
Focus on `TransOptimize` translations — these are the fully optimized JIT output
and represent the code that actually runs in production.

By default, the `top` and `inspect` commands also query the **Xenon** Scuba
dataset (`xenon`) to show each function's **exclusive gCPU %** — what fraction
of total CPU the function itself consumes (leaf cost, not including callees).
This answers "is optimizing this function worth it?" since a high profCount
doesn't necessarily mean high CPU cost.

- `gCPU=0.18%` means the function is 0.18% of total fleet CPU (last 4 hours).
- `gCPU=    -` means the function isn't in xenon's top 500 — it likely consumes
  negligible CPU at the leaf level (common for builtins like `ord`, `strlen`
  whose actual cost is attributed to native code).
- Use `--xenon-hours N` to change the lookback window (default: 4 hours).
- Use `--xenon-hours 0` to skip the Xenon query entirely.

### Step 2: Inspect in Summary Mode

Use `--format summary` to quickly scan a translation's structure. The summary
flags anomalies automatically:
- **"hot in cold area"** — Block has high profCount but is placed in the Cold
  code area. This means the code is frequently executed but was predicted to be
  rare, causing instruction cache pressure.
- **"hot but Unlikely"** — Block has high profCount but its hint says Unlikely.
  The hint drives code layout; a wrong hint means hot code is placed far from
  the main path.
- **"dead?"** — Block has zero profCount in a TransOptimize translation,
  meaning it was never executed in the profiling run that guided optimization.

### Step 3: Inspect in Detail Mode

Use `--format detail` to see the full SSA IR. The output format is:

```
Block B0 [Main, Neither] (profCount=138,075,990, preds=[], next->B1):
  (000) t0:FramePtr = DefFuncEntryFP
  (001) t1:Int      = LdLoc<0> t0
  (002)               CheckType<Bool> t1 -> B5
  (003) t3:Bool     = AssertType<Bool> t1
```

Each line is one SSA instruction:
- `(NNN)` — instruction ID
- `tN:Type` — SSA temporary with its type
- `= Opcode<TypeParam, Extra>` — the HHIR opcode with optional parameters
- `tN, tM` — source operands (SSA temporaries)
- `-> BN` — taken branch (for conditional/guard instructions)

Phi nodes appear at block entry on `DefLabel` instructions:
```
  phi t10:Bool = [t5@B2, t8@B3]
  (020) t10:Bool = DefLabel
```

### Step 4: What to Look For

These are the main optimization patterns to identify:

#### 1. Redundant Type Checks
A `CheckType<T>` guards that a value has type T, branching to a side exit if
not. If the value's SSA type already satisfies the guard (e.g., checking
`CheckType<Bool>` on a value already typed `Bool`), the check is redundant.

#### 2. Hot Code in Cold Blocks
Blocks in the "Cold" area with high profCount values. The JIT places Cold blocks
far from Main blocks in memory. If these blocks are frequently executed, it
causes instruction cache misses. Look for blocks flagged "hot in cold area" in
summary mode.

#### 3. Missed Inlining
Check the "Inlining Decisions" section. Functions marked "not inlined" with a
reason like "too large" or "too deep" in a hot path may benefit from raising
inlining thresholds or refactoring.

#### 4. Dead Code
Blocks with profCount=0 in TransOptimize translations were never executed during
profiling. If these blocks contain significant code (not just catch handlers),
they may represent over-specialization.

#### 5. Suboptimal Type Specialization
Long chains of `CheckType` instructions testing multiple types sequentially
(e.g., Dict → Vec → Keyset → Obj) suggest the JIT isn't specializing well for
the dominant type. If profCount shows one path is overwhelmingly hot, the
compiler could potentially specialize for just that type.

#### 6. Excessive Phi Nodes
Large phi nodes at block merge points can indicate the JIT is maintaining too
many live values across branches, increasing register pressure.

#### 7. Instruction Patterns
- `DecRefNZ` / `DecRef` clusters: excessive reference counting overhead
- `StLoc` immediately followed by `LdLoc` of the same local: unnecessary
  store-load pairs
- `AssertType` after `CheckType` where the assert is trivially true
- Multiple `BespokeGet<Unknown>` calls: array access without specialization

## HHIR Reference

Key opcodes and their meanings:
- `CheckType<T>` — Guard: check value has type T, side-exit if not
- `AssertType<T>` — Assert value has type T (no runtime check, just narrows SSA type)
- `LdLoc<N>` — Load local variable N from the frame
- `StLoc<N>` — Store to local variable N
- `DefLabel` — Block entry point (may have phi nodes)
- `Jmp` — Unconditional jump
- `JmpZero` / `JmpNZero` — Conditional jump on zero/non-zero
- `CheckSurpriseFlagsEnter` — Check for async signals/stack overflow at function entry
- `EnterInlineFrame` / `LeaveInlineFrame` — Inline call boundaries
- `InlineCall` / `InlineSideExit` — Inlining machinery
- `RetCtrl` — Return from translation
- `ReqBindJmp` — Request the JIT to compile and bind a new translation
- `DecRef` / `DecRefNZ` — Decrement reference count (NZ = known non-zero)
- `AKExistsDict` / `AKExistsKeyset` — Array key existence checks
- `BespokeGet` — Generic array element access
- `InstanceOfBitmask` — Fast class hierarchy check
- `VerifyParamFailHard` — Type constraint violation (always throws)

## File Structure

| File | Purpose |
|------|---------|
| `ir_model.py` | Dataclasses matching `hphp/doc/printir_json_schema.ts` |
| `data_fetcher.py` | Presto/Hive queries + local file loading |
| `ir_parser.py` | JSON blob → dataclass parsing |
| `formatter.py` | Summary and detail output formatting |
| `xenon_fetcher.py` | Xenon Scuba queries for gCPU % (exclusive/leaf cost) |
| `main.py` | CLI entry point (argparse) |
