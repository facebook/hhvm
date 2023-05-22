// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Error;
use ir::instr::BaseOp;
use ir::instr::FinalOp;
use ir::instr::MemberKey;
use ir::instr::MemberOp;
use ir::InstrId;
use ir::LocalId;
use ir::QueryMOp;
use ir::StringInterner;
use ir::ValueId;
use naming_special_names_rust::special_idents;

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
    state: &mut FuncState<'_, '_, '_>,
    iid: InstrId,
    mop: &ir::instr::MemberOp,
) -> Result {
    let mut locals = mop.locals.iter().copied();
    let mut operands = mop.operands.iter().copied();

    let mut base = write_base(state, &mop.base_op, &mut locals, &mut operands)?;

    for intermediate in mop.intermediate_ops.iter() {
        base = write_entry(state, base, &intermediate.key, &mut locals, &mut operands)?;
    }

    match &mop.final_op {
        FinalOp::SetRangeM { .. } => todo!(),
        op => {
            let output = write_final(state, op, base, &mut locals, &mut operands)?;
            state.set_iid(iid, output);
        }
    }

    assert!(locals.next().is_none());
    assert!(operands.next().is_none(), "MOP: {mop:?}");

    Ok(())
}

fn write_base(
    state: &mut FuncState<'_, '_, '_>,
    base_op: &BaseOp,
    locals: &mut impl Iterator<Item = LocalId>,
    operands: &mut impl Iterator<Item = ValueId>,
) -> Result<Sid> {
    match *base_op {
        BaseOp::BaseC { .. } => {
            // Get base from value.
            let base = base_from_vid(state, operands.next().unwrap())?;
            state.fb.write_expr_stmt(base)
        }
        BaseOp::BaseGC { .. } => {
            // Get base from global name.
            let src = state.lookup_vid(operands.next().unwrap());
            state.call_builtin(hack::Builtin::BaseGetSuperglobal, [src])
        }
        BaseOp::BaseH { loc: _ } => {
            // Get base from $this.
            // Just pretend to be a BaseL w/ $this.
            let lid = LocalId::Named(state.strings.intern_str(special_idents::THIS));
            let base = base_from_lid(lid);
            state.fb.write_expr_stmt(base)
        }
        BaseOp::BaseL {
            mode: _,
            readonly: _,
            loc: _,
        } => {
            // Get base from local.
            let base = base_from_lid(locals.next().unwrap());
            state.fb.write_expr_stmt(base)
        }
        BaseOp::BaseSC {
            mode: _,
            readonly: _,
            loc: _,
        } => {
            // Get base from static property.
            let prop = state.lookup_vid(operands.next().unwrap());
            let base = base_from_vid(state, operands.next().unwrap())?;
            state.call_builtin(hack::Builtin::DimFieldGet, (base, prop))
        }
    }
}

fn write_final(
    state: &mut FuncState<'_, '_, '_>,
    final_op: &FinalOp,
    base: Sid,
    locals: &mut impl Iterator<Item = LocalId>,
    operands: &mut impl Iterator<Item = ValueId>,
) -> Result<Sid> {
    let key = final_op.key().unwrap();
    let base = write_entry(state, base, key, locals, operands)?;

    match *final_op {
        FinalOp::IncDecM { inc_dec_op, .. } => {
            let src = state.load_mixed(base)?;
            let op = match inc_dec_op {
                ir::IncDecOp::PreInc | ir::IncDecOp::PostInc => hack::Hhbc::Add,
                ir::IncDecOp::PreDec | ir::IncDecOp::PostDec => hack::Hhbc::Sub,
                _ => unreachable!(),
            };
            let incr = hack::expr_builtin(hack::Builtin::Int, [1]);
            let dst = state
                .fb
                .write_expr_stmt(hack::expr_builtin(hack::Builtin::Hhbc(op), (src, incr)))?;
            state.store_mixed(base, dst)?;
            Ok(match inc_dec_op {
                ir::IncDecOp::PreInc | ir::IncDecOp::PreDec => dst,
                ir::IncDecOp::PostInc | ir::IncDecOp::PostDec => src,
                _ => unreachable!(),
            })
        }
        FinalOp::QueryM { query_m_op, .. } => match query_m_op {
            QueryMOp::CGet | QueryMOp::CGetQuiet => state.load_mixed(base),
            QueryMOp::Isset => {
                let value = state.load_mixed(base)?;
                state.call_builtin(hack::Builtin::Hhbc(hack::Hhbc::IsTypeNull), [value])
            }
            QueryMOp::InOut => textual_todo! {
                state.load_mixed(base)
            },
            _ => unreachable!(),
        },
        FinalOp::SetM { .. } => {
            let src = state.lookup_vid(operands.next().unwrap());
            let src = state.fb.write_expr_stmt(src)?;
            state.store_mixed(base, src)?;
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
            state.write_todo("SetOpM")
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
            state.write_todo("UnsetM")
        },
    }
}

fn write_entry(
    state: &mut FuncState<'_, '_, '_>,
    base: Sid,
    key: &MemberKey,
    locals: &mut impl Iterator<Item = LocalId>,
    operands: &mut impl Iterator<Item = ValueId>,
) -> Result<Sid> {
    match *key {
        MemberKey::EC => {
            // $a[foo()]
            let key = state.lookup_vid(operands.next().unwrap());
            state.call_builtin(hack::Builtin::DimArrayGet, (base, key))
        }
        MemberKey::EI(i) => {
            // $a[3]
            let idx = state.call_builtin(hack::Builtin::Int, [i])?;
            state.call_builtin(hack::Builtin::DimArrayGet, (base, idx))
        }
        MemberKey::EL => {
            // $a[$b]
            let key = locals.next().unwrap();
            let key = state.load_mixed(textual::Expr::deref(key))?;
            state.call_builtin(hack::Builtin::DimArrayGet, (base, key))
        }
        MemberKey::ET(s) => {
            // $a["hello"]
            let key = {
                let key = state.strings.lookup_bytes(s);
                crate::util::escaped_string(&key)
            };
            let key = state.call_builtin(hack::Builtin::String, [key])?;
            state.call_builtin(hack::Builtin::DimArrayGet, (base, key))
        }
        MemberKey::PC => {
            // $a->{foo()}
            let key = state.lookup_vid(operands.next().unwrap());
            state.call_builtin(hack::Builtin::DimFieldGet, (base, key))
        }
        MemberKey::PL => {
            // $a->{$b}
            let key = locals.next().unwrap();
            let key = state.load_mixed(textual::Expr::deref(key))?;
            state.call_builtin(hack::Builtin::DimFieldGet, (base, key))
        }
        MemberKey::PT(prop) => {
            // $a->hello
            let key = {
                let key = state.strings.lookup_bytes(prop.id);
                crate::util::escaped_string(&key)
            };
            let key = state.call_builtin(hack::Builtin::String, [key])?;
            state.call_builtin(hack::Builtin::DimFieldGet, (base, key))
        }
        MemberKey::QT(prop) => {
            // $a?->hello
            let key = {
                let key = state.strings.lookup_bytes(prop.id);
                crate::util::escaped_string(&key)
            };
            let key = state.call_builtin(hack::Builtin::String, [key])?;
            state.call_builtin(hack::Builtin::DimFieldGetOrNull, (base, key))
        }
        MemberKey::W => {
            // $a[]
            state.call_builtin(hack::Builtin::DimArrayAppend, [base])
        }
    }
}

pub(crate) fn base_var(strings: &StringInterner) -> LocalId {
    const BASE_VAR: &str = "base";
    let base = strings.intern_str(BASE_VAR);
    LocalId::Named(base)
}

pub(crate) fn func_needs_base_var(func: &ir::Func<'_>) -> bool {
    use ir::instr::Hhbc;
    use ir::Instr;
    for instr in func.instrs.iter() {
        match instr {
            Instr::MemberOp(MemberOp {
                base_op: BaseOp::BaseC { .. } | BaseOp::BaseSC { .. },
                ..
            })
            | Instr::Hhbc(Hhbc::SetS(..)) => {
                return true;
            }
            _ => {}
        }
    }

    false
}

pub(crate) fn base_from_expr(
    state: &mut FuncState<'_, '_, '_>,
    src: impl Into<textual::Expr>,
) -> Result<textual::Expr> {
    // Unfortunately we need base to be a pointer to a value, not a
    // value itself - so store it in `base` so we can return a pointer
    // to `base`.
    let base_lid = base_var(&state.strings);
    state.store_mixed(textual::Expr::deref(base_lid), src.into())?;
    Ok(base_from_lid(base_lid))
}

pub(crate) fn base_from_vid(
    state: &mut FuncState<'_, '_, '_>,
    src: ValueId,
) -> Result<textual::Expr> {
    let src = state.lookup_vid(src);
    base_from_expr(state, src)
}

fn base_from_lid(lid: LocalId) -> textual::Expr {
    textual::Expr::deref(lid)
}
