# ABI, Calling Conventions, Stubs, and Fixups

> Sections marked with 📋 list specific names that may drift as code evolves. If a name isn't found, search for the pattern nearby.

## Calling Conventions

Three calling conventions exist in the JIT (`unique-stubs.h`):
- **native**: Standard C++ calls (`call`/`ret`)
- **PHP**: Calls between PHP functions (`phpcall`/`phpret`/`phplogue`)
- **stub**: Calls to unique stubs (`callstub`/`stubret`/`stublogue`)

**Mixing these up corrupts the stack.** Each has different invariants about frame
layout and saved registers.

### 📋 Reserved registers (x64)

`rsp`, `rvmfp()` (rbp), `rvmtl()` (r12) — never available to the register
allocator.

**Important: these are PhysReg constants, not Vregs.** See "Vreg vs PhysReg at
irlower time" below for why this matters when writing `cgFoo` functions.

### 📋 Four ABI contexts (`types.h`)

| Context | When | Notes |
|---------|------|-------|
| Trace | Normal PHP code | Full register set minus reserved |
| Prologue | Function prologues | Input regs: flags, callee, ctx, numArgs in `rarg(0..3)` |
| CrossTrace | TC boundaries, service requests | Only `rvmfp` + `rvmtl` are live |
| Helper | Scratch-only (runtime calls) | Very limited: `r10`, `r11`, `xmm5-7` on x64 |

**Stack alignment invariant**: After prologue instructions, the native stack
pointer is always 16-byte aligned.

### 📋 Prologue input registers

- `r_func_prologue_flags()` = `rarg(0)`
- `r_func_prologue_callee()` = `rarg(1)`
- `r_func_prologue_ctx()` = `rarg(2)`
- `r_func_prologue_num_args()` = `rarg(3)`

Func entry uses a different convention: `r_func_entry_ar_flags()`,
`r_func_entry_callee_id()`, `r_func_entry_ctx()`.

Key files: `abi.h`, `abi-x64.cpp`, `abi-arm.cpp`, `phys-reg.h`.

## Fixups

Every call from JIT to C++ that may need to walk the VM stack must have a fixup
recorded. Missing fixups cause assertion failures or incorrect stack traces / GC
roots.

Three fixup types (`fixup.h`):
- **Direct**: Stores pcOffset and spOffset. Most common.
- **Indirect**: Used in stublogue mode (no full frame). Chases two levels of
  return addresses.
- **AsioStub**: For suspending/freeing resumable frames.

**Fixup map is keyed by TC offset** (not raw TCA) — `tc::addrToOffset(tca)` converts.

`syncVMRegs()` is the public API — it's a no-op when `regState() == CLEAN`.

Key files: `fixup.cpp`, `fixup.h`.

## Service Requests

When JIT code reaches an untranslated SrcKey, it hits a **service request stub**
(small code in the cold TC that encodes the SrcKey), which jumps to a **unique
stub handler**, which triggers translation. After translation, the service
request stub is smashed to jump directly to the new code.

Service request stubs are cached in `s_stubMap` — `getOrEmitStub()` is idempotent.

**Stubs emit into cold code** (`view.cold()`), not main.

**FuncEntry stubs have different register conventions** than non-funcEntry stubs.

Key files: `service-requests.cpp`, `service-requests.h`.

## Unique Stubs

`UniqueStubs` struct holds TCA pointers to shared entry/exit stubs, emitted once
during TC initialization (`emitAll()`). Their addresses live for the process
lifetime.

Key stubs: `enterTCHelper` (native→JIT bridge), `callToExit` (JIT→native),
`resumeHelper`, `funcPrologue` entry points.

**VM register helpers**: `loadVMRegs()`/`storeVMRegs()` load/store `rvmfp()` and
`rvmsp()` from/to RDS. Getting the load/store order wrong causes register
corruption.

Key files: `unique-stubs.h`, `unique-stubs.cpp`, `unique-stubs-x64.cpp`,
`unique-stubs-arm.cpp`.

## Vreg vs PhysReg at irlower time

In `cgFoo` functions (`irlower-*.cpp`), `srcLoc(env, inst, N).reg()` returns a
**Vreg**, while `rvmfp()`, `rvmsp()`, `rvmtl()` are **PhysReg constants**. These
are different number spaces — **comparing them with `==` is always wrong**.

Register allocation runs *after* irlower, so at irlower time the Vreg → PhysReg
mapping doesn't exist yet. To assert that a value will be in a specific physical
register, check the HHIR instruction that produced it:

```cpp
// WRONG — Vreg vs PhysReg comparison, always false
assertx(srcLoc(env, inst, 0).reg() == rvmfp());

// RIGHT — check the SSATmp's origin instruction
// DefFP is guaranteed to coalesce to rvmfp() by regalloc
assertx(inst.src(0)->inst()->is(DefFP));
```

Key files: `irlower.h`, `irlower-internal.h`, `phys-reg.h`, `vasm-unit.h`.

## Pitfalls

- **Every JIT→C++ call needs a fixup** — missing fixups cause assertion failures in stack walking and GC
- **Don't mix calling conventions in vasm** — `call`/`ret`, `stublogue`/`stubret`, `phplogue`/`phpret` have different frame layout invariants
- **x64 Helper registers are very limited**: only `r10`, `r11` for GP and `xmm5-7` for SIMD. Using other registers in Helper context corrupts state.
- **`cross_trace_regs()`** = `vm_regs_no_sp()` = rvmfp + rvmtl. These are the live registers at tracelet boundaries.
