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
use ir::MOpMode;
use ir::QueryMOp;
use ir::ReadonlyOp;
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

pub(crate) fn write(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    iid: InstrId,
    mop: &ir::instr::MemberOp,
) -> Result {
    let mut locals = mop.locals.iter().copied();
    let mut operands = mop.operands.iter().copied();

    let mut base = write_base(w, &mop.base_op, &mut locals, &mut operands, state)?;

    for intermediate in mop.intermediate_ops.iter() {
        base = write_entry(
            w,
            state,
            base,
            &intermediate.key,
            intermediate.readonly,
            intermediate.mode,
            &mut locals,
            &mut operands,
        )?;
    }

    let output = write_final(w, state, &mop.final_op, base, &mut locals, &mut operands)?;
    state.set_iid(iid, output);

    assert!(locals.next().is_none());
    assert!(operands.next().is_none());

    Ok(())
}

fn write_base(
    w: &mut textual::FuncWriter<'_>,
    base_op: &BaseOp,
    locals: &mut impl Iterator<Item = LocalId>,
    operands: &mut impl Iterator<Item = ValueId>,
    state: &mut FuncState<'_>,
) -> Result<Sid> {
    match *base_op {
        BaseOp::BaseC { .. } => {
            let _ = operands.next();
            write_todo(w, state, "BaseC")
        }
        BaseOp::BaseGC { .. } => {
            let _ = operands.next();
            write_todo(w, state, "BaseGC")
        }
        BaseOp::BaseH { .. } => write_todo(w, state, "BaseH"),
        BaseOp::BaseL {
            mode: _,
            readonly: _,
            loc: _,
        } => {
            let lid = locals.next().unwrap();
            let value = textual::Expr::deref(lid);
            w.copy(value)
        }
        BaseOp::BaseSC { .. } => {
            let _ = operands.next();
            let _ = operands.next();
            write_todo(w, state, "BaseSC")
        }
    }
}

fn write_final(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    final_op: &FinalOp,
    base: Sid,
    locals: &mut impl Iterator<Item = LocalId>,
    operands: &mut impl Iterator<Item = ValueId>,
) -> Result<Sid> {
    match *final_op {
        FinalOp::IncDecM { .. } => todo!(),
        FinalOp::QueryM {
            ref key,
            readonly,
            query_m_op,
            loc: _,
        } => write_final_query_m(w, state, base, key, readonly, query_m_op, locals, operands),
        FinalOp::SetM {
            ref key,
            readonly,
            loc: _,
        } => {
            let base = write_entry(
                w,
                state,
                base,
                key,
                readonly,
                MOpMode::None,
                locals,
                operands,
            )?;
            let src = state.lookup_vid(operands.next().unwrap());
            let src = w.write_expr(src)?;
            w.store(base, src, tx_ty!(*HackMixed))?;
            Ok(src)
        }
        FinalOp::SetRangeM { .. } => todo!(),
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
            write_todo(w, state, "SetOpM")
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
            write_todo(w, state, "UnsetM")
        },
    }
}

fn write_final_query_m(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    base: Sid,
    key: &MemberKey,
    _readonly: ReadonlyOp,
    query_m_op: QueryMOp,
    locals: &mut impl Iterator<Item = LocalId>,
    operands: &mut impl Iterator<Item = ValueId>,
) -> Result<Sid> {
    let op_name = match query_m_op {
        QueryMOp::CGet => "cget",
        QueryMOp::CGetQuiet => "cget_quiet",
        QueryMOp::Isset => "isset",
        QueryMOp::InOut => "inout",
        _ => unreachable!(),
    };

    match *key {
        MemberKey::EC => {
            // $a[foo()]
            let key = state.lookup_vid(operands.next().unwrap());
            w.call("hack_array_get", (base, key, op_name))
        }
        MemberKey::EI(i) => {
            // $a[3]
            let idx = hack::call_builtin(w, hack::Builtin::Int, [i])?;
            w.call("hack_array_get", (base, idx, op_name))
        }
        MemberKey::EL => {
            // $a[$b]
            let key = locals.next().unwrap();
            let key = w.load(tx_ty!(*HackMixed), textual::Expr::deref(key))?;
            w.call("hack_array_get", (base, key, op_name))
        }
        MemberKey::ET(s) => {
            // $a["hello"]
            let key = state.strings.lookup_bytes(s);
            let key = crate::util::escaped_string(&key);
            let key = hack::call_builtin(w, hack::Builtin::String, [key])?;
            w.call("hack_array_get", (base, key, op_name))
        }
        MemberKey::PC => {
            // $a->(foo())

            todo!();
        }
        MemberKey::PL => {
            // $a->($b)
            todo!();
        }
        MemberKey::PT(prop) => {
            // $a->hello

            // Since we don't know the actual type (right now everything is
            // HackMixed) then we need to use dynamic access.  In the future if
            // we know the actual type we may be able to use direct field
            // access.

            let key = state.strings.lookup_bytes(prop.id);
            let key = crate::util::escaped_string(&key);
            let key = hack::call_builtin(w, hack::Builtin::String, [key])?;
            w.call("hack_field_get", (base, key, op_name))
        }
        MemberKey::QT(prop) => {
            // $a?->hello
            textual_todo! {
                let key = state.strings.lookup_bytes(prop.id);
                let key = crate::util::escaped_string(&key);
                let key = hack::call_builtin(w, hack::Builtin::String, [key])?;
                w.call("hack_field_get", (base, key, op_name))
            }
        }
        MemberKey::W => {
            // $a[]
            todo!();
        }
    }
}

fn write_entry(
    w: &mut textual::FuncWriter<'_>,
    state: &mut FuncState<'_>,
    base: Sid,
    key: &MemberKey,
    _readonly: ReadonlyOp,
    mode: MOpMode,
    locals: &mut impl Iterator<Item = LocalId>,
    operands: &mut impl Iterator<Item = ValueId>,
) -> Result<Sid> {
    let mode = match mode {
        MOpMode::None => "none",
        MOpMode::Warn => "warn",
        MOpMode::Define => "define",
        MOpMode::Unset => "unset",
        MOpMode::InOut => "inout",
        _ => unreachable!(),
    };

    match *key {
        MemberKey::EC => {
            // $a[foo()]
            let key = state.lookup_vid(operands.next().unwrap());
            w.call("hack_array_entry", (base, key, mode))
        }
        MemberKey::EI(i) => {
            // $a[3]
            let idx = hack::call_builtin(w, hack::Builtin::Int, [i])?;
            w.call("hack_array_entry", (base, idx, mode))
        }
        MemberKey::EL => {
            // $a[$b]
            let key = locals.next().unwrap();
            let key = w.load(tx_ty!(*HackMixed), textual::Expr::deref(key))?;
            w.call("hack_array_entry", (base, key, mode))
        }
        MemberKey::ET(s) => {
            // $a["hello"]
            let key = state.strings.lookup_bytes(s);
            let key = crate::util::escaped_string(&key);
            let key = hack::call_builtin(w, hack::Builtin::String, [key])?;
            w.call("hack_array_entry", (base, key, mode))
        }
        MemberKey::PC => {
            // $a->(foo())
            todo!();
        }
        MemberKey::PL => {
            // $a->($b)
            todo!();
        }
        MemberKey::PT(prop) => {
            // $a->hello
            let key = state.strings.lookup_bytes(prop.id);
            let key = crate::util::escaped_string(&key);
            let key = hack::call_builtin(w, hack::Builtin::String, [key])?;
            w.call("hack_field_entry", (base, key, mode))
        }
        MemberKey::QT(prop) => {
            // $a?->hello
            textual_todo! {
                let key = state.strings.lookup_bytes(prop.id);
                let key = crate::util::escaped_string(&key);
                let key = hack::call_builtin(w, hack::Builtin::String, [key])?;
                w.call("hack_field_entry", (base, key, mode))
            }
        }
        MemberKey::W => {
            // $a[]
            write_todo(w, state, "MemberKey_W")
        }
    }
}
