// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Error;
use ir::instr::BaseOp;
use ir::instr::FinalOp;
use ir::instr::MemberKey;
use ir::InstrId;
use ir::LocalId;
use ir::StringInterner;
use ir::ValueId;

use crate::func::write_todo;
use crate::func::FuncState;
use crate::hack;
use crate::textual;
use crate::textual::Sid;

type Result<T = (), E = Error> = std::result::Result<T, E>;

// We need to be careful how we transform member operations so we only maintain
// a single base at a time and yet handle COW properly (so we mimic HHVM).
//
// $a[k1][k2][k3] = v1;
//
// Becomes:
//   base = &$a
//   base = hack_array_entry(base, k1); // this may write to $a
//   base = hack_array_entry(base, k2); // this may write to $a[k1]
//   hack_array_write(base, k3, v1); // this may write to $a[k1][k2] and $a[k1][k2][k3]
//
// Notation note (for builtins relating to member-ops):
//   Base refers to a function that takes a value and returns a **HackMixed.
//   Dim refers to a function that takes a **HackMixed and returns a **HackMixed.
//   Final refers to a function that takes a **HackMixed and returns a value.
//
pub(crate) fn write(
    fb: &mut textual::FuncBuilder<'_, '_>,
    state: &mut FuncState<'_>,
    iid: InstrId,
    mop: &ir::instr::MemberOp,
) -> Result {
    let mut locals = mop.locals.iter().copied();
    let mut operands = mop.operands.iter().copied();

    let mut base = write_base(fb, &mop.base_op, &mut locals, &mut operands, state)?;

    for intermediate in mop.intermediate_ops.iter() {
        base = write_entry(
            fb,
            state,
            base,
            &intermediate.key,
            &mut locals,
            &mut operands,
        )?;
    }

    match &mop.final_op {
        FinalOp::SetRangeM { .. } => todo!(),
        op => {
            let output = write_final(fb, state, op, base, &mut locals, &mut operands)?;
            state.set_iid(iid, output);
        }
    }

    assert!(locals.next().is_none());
    assert!(operands.next().is_none(), "MOP: {mop:?}");

    Ok(())
}

fn write_base(
    fb: &mut textual::FuncBuilder<'_, '_>,
    base_op: &BaseOp,
    locals: &mut impl Iterator<Item = LocalId>,
    operands: &mut impl Iterator<Item = ValueId>,
    state: &mut FuncState<'_>,
) -> Result<Sid> {
    match *base_op {
        BaseOp::BaseC { .. } => {
            // Get base from value.
            let base = base_from_vid(fb, state, operands.next().unwrap())?;
            fb.copy(base)
        }
        BaseOp::BaseGC { .. } => {
            // Get base from global name.
            let src = state.lookup_vid(operands.next().unwrap());
            hack::call_builtin(fb, hack::Builtin::BaseGetSuperglobal, [src])
        }
        BaseOp::BaseH { loc: _ } => {
            // Get base from $this.
            // Just pretend to be a BaseL w/ $this.
            let lid = LocalId::Named(state.strings.intern_str("$this"));
            let base = base_from_lid(lid);
            fb.copy(base)
        }
        BaseOp::BaseL {
            mode: _,
            readonly: _,
            loc: _,
        } => {
            // Get base from local.
            let base = base_from_lid(locals.next().unwrap());
            fb.copy(base)
        }
        BaseOp::BaseSC {
            mode: _,
            readonly: _,
            loc: _,
        } => {
            // Get base from static property.
            let base = base_from_vid(fb, state, operands.next().unwrap())?;
            let prop = state.lookup_vid(operands.next().unwrap());
            hack::call_builtin(fb, hack::Builtin::DimFieldGet, (base, prop))
        }
    }
}

fn write_final(
    fb: &mut textual::FuncBuilder<'_, '_>,
    state: &mut FuncState<'_>,
    final_op: &FinalOp,
    base: Sid,
    locals: &mut impl Iterator<Item = LocalId>,
    operands: &mut impl Iterator<Item = ValueId>,
) -> Result<Sid> {
    let key = final_op.key().unwrap();
    let base = write_entry(fb, state, base, key, locals, operands)?;

    match *final_op {
        FinalOp::IncDecM { inc_dec_op, .. } => {
            let src = fb.load(tx_ty!(*HackMixed), base)?;
            let op = match inc_dec_op {
                ir::IncDecOp::PreInc | ir::IncDecOp::PostInc => hack::Hhbc::Add,
                ir::IncDecOp::PreDec | ir::IncDecOp::PostDec => hack::Hhbc::Sub,
                _ => unreachable!(),
            };
            let incr = hack::expr_builtin(hack::Builtin::Int, [1]);
            let dst =
                fb.write_expr_stmt(hack::expr_builtin(hack::Builtin::Hhbc(op), (src, incr)))?;
            fb.store(base, dst, tx_ty!(*HackMixed))?;
            Ok(match inc_dec_op {
                ir::IncDecOp::PreInc | ir::IncDecOp::PreDec => dst,
                ir::IncDecOp::PostInc | ir::IncDecOp::PostDec => src,
                _ => unreachable!(),
            })
        }
        FinalOp::QueryM { .. } => fb.load(tx_ty!(*HackMixed), base),
        FinalOp::SetM { .. } => {
            let src = state.lookup_vid(operands.next().unwrap());
            let src = fb.write_expr_stmt(src)?;
            fb.store(base, src, tx_ty!(*HackMixed))?;
            Ok(src)
        }
        FinalOp::SetRangeM { .. } => unreachable!(),
        FinalOp::SetOpM { ref key, .. } => textual_todo! {
            match *key {
                MemberKey::EC => { let _ = operands.next(); }
                MemberKey::EI(_) => { }
                MemberKey::EL => { let _ = locals.next(); }
                MemberKey::ET(_) => { }
                MemberKey::PC => { }
                MemberKey::PL => { }
                MemberKey::PT(_) => { }
                MemberKey::QT(_) => { }
                MemberKey::W => { }
            }
            let _ = operands.next();
            write_todo(fb, "SetOpM")
        },
        FinalOp::UnsetM { ref key, .. } => textual_todo! {
            match *key {
                MemberKey::EC => { let _ = operands.next(); }
                MemberKey::EI(_) => { }
                MemberKey::EL => { let _ = locals.next(); }
                MemberKey::ET(_) => { }
                MemberKey::PC => { }
                MemberKey::PL => { }
                MemberKey::PT(_) => { }
                MemberKey::QT(_) => { }
                MemberKey::W => { }
            }
            write_todo(fb, "UnsetM")
        },
    }
}

fn write_entry(
    fb: &mut textual::FuncBuilder<'_, '_>,
    state: &mut FuncState<'_>,
    base: Sid,
    key: &MemberKey,
    locals: &mut impl Iterator<Item = LocalId>,
    operands: &mut impl Iterator<Item = ValueId>,
) -> Result<Sid> {
    match *key {
        MemberKey::EC => {
            // $a[foo()]
            let key = state.lookup_vid(operands.next().unwrap());
            hack::call_builtin(fb, hack::Builtin::DimArrayGet, (base, key))
        }
        MemberKey::EI(i) => {
            // $a[3]
            let idx = hack::call_builtin(fb, hack::Builtin::Int, [i])?;
            hack::call_builtin(fb, hack::Builtin::DimArrayGet, (base, idx))
        }
        MemberKey::EL => {
            // $a[$b]
            let key = locals.next().unwrap();
            let key = fb.load(tx_ty!(*HackMixed), textual::Expr::deref(key))?;
            hack::call_builtin(fb, hack::Builtin::DimArrayGet, (base, key))
        }
        MemberKey::ET(s) => {
            // $a["hello"]
            let key = state.strings.lookup_bytes(s);
            let key = crate::util::escaped_string(&key);
            let key = hack::call_builtin(fb, hack::Builtin::String, [key])?;
            hack::call_builtin(fb, hack::Builtin::DimArrayGet, (base, key))
        }
        MemberKey::PC => {
            // $a->{foo()}
            let key = state.lookup_vid(operands.next().unwrap());
            hack::call_builtin(fb, hack::Builtin::DimFieldGet, (base, key))
        }
        MemberKey::PL => {
            // $a->{$b}
            let key = locals.next().unwrap();
            let key = fb.load(tx_ty!(*HackMixed), textual::Expr::deref(key))?;
            hack::call_builtin(fb, hack::Builtin::DimFieldGet, (base, key))
        }
        MemberKey::PT(prop) => {
            // $a->hello
            let key = state.strings.lookup_bytes(prop.id);
            let key = crate::util::escaped_string(&key);
            let key = hack::call_builtin(fb, hack::Builtin::String, [key])?;
            hack::call_builtin(fb, hack::Builtin::DimFieldGet, (base, key))
        }
        MemberKey::QT(prop) => {
            // $a?->hello
            let key = state.strings.lookup_bytes(prop.id);
            let key = crate::util::escaped_string(&key);
            let key = hack::call_builtin(fb, hack::Builtin::String, [key])?;
            hack::call_builtin(fb, hack::Builtin::DimFieldGetOrNull, (base, key))
        }
        MemberKey::W => {
            // $a[]
            hack::call_builtin(fb, hack::Builtin::DimArrayAppend, [base])
        }
    }
}

pub(crate) fn base_var(strings: &StringInterner) -> LocalId {
    const BASE_VAR: &str = "base";
    let base = strings.intern_str(BASE_VAR);
    LocalId::Named(base)
}

fn base_from_vid(
    fb: &mut textual::FuncBuilder<'_, '_>,
    state: &mut FuncState<'_>,
    src: ValueId,
) -> Result<textual::Expr> {
    // Unfortunately we need base to be a pointer to a value, not a
    // value itself - so store it in `base` so we can return a pointer
    // to `base`.
    let src = state.lookup_vid(src);
    let base_lid = base_var(state.strings);
    fb.store(textual::Expr::deref(base_lid), src, tx_ty!(*HackMixed))?;
    Ok(base_from_lid(base_lid))
}

fn base_from_lid(lid: LocalId) -> textual::Expr {
    textual::Expr::deref(lid)
}
