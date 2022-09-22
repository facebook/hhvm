// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Error;
use ir::instr::BaseOp;
use ir::instr::FinalOp;
use ir::instr::MOpMode;
use ir::instr::MemberKey;
use ir::instr::QueryMOp;
use ir::instr::ReadonlyOp;
use ir::InstrId;
use ir::LocalId;
use ir::ValueId;

use crate::func::FuncState;
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

    let mut base = write_base(w, &mop.base_op, &mut locals, &mut operands)?;

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
    _operands: &mut impl Iterator<Item = ValueId>,
) -> Result<Sid> {
    match *base_op {
        BaseOp::BaseC { .. } => todo!(),
        BaseOp::BaseGC { .. } => todo!(),
        BaseOp::BaseH { .. } => todo!(),
        BaseOp::BaseL {
            mode: _,
            readonly: _,
            loc: _,
        } => {
            let lid = locals.next().unwrap();
            let value = textual::Expr::deref(lid);
            w.copy(value)
        }
        BaseOp::BaseSC { .. } => todo!(),
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
            w.store(base, src, &textual::Ty::Mixed)?;
            Ok(src)
        }
        FinalOp::SetRangeM { .. } => todo!(),
        FinalOp::SetOpM { .. } => todo!(),
        FinalOp::UnsetM { .. } => todo!(),
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
            w.call(
                "hack_array_get",
                (base, textual::Expr::hack_int(i), op_name),
            )
        }
        MemberKey::EL => {
            // $a[$b]
            let key = locals.next().unwrap();
            let key = w.load(&textual::Ty::Mixed, textual::Expr::deref(key))?;
            w.call("hack_array_get", (base, key, op_name))
        }
        MemberKey::ET(s) => {
            // $a["hello"]
            let key = state.strings.lookup_bytes(s);
            let key = textual::Expr::hack_string(key);
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
            let key = textual::Expr::hack_string(key);
            w.call("hack_field_get", (base, key, op_name))
        }
        MemberKey::QT(_) => {
            // $a?->hello
            todo!();
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
            w.call("hack_array_entry", (base, textual::Expr::hack_int(i), mode))
        }
        MemberKey::EL => {
            // $a[$b]
            let key = locals.next().unwrap();
            let key = w.load(&textual::Ty::Mixed, textual::Expr::deref(key))?;
            w.call("hack_array_entry", (base, key, mode))
        }
        MemberKey::ET(s) => {
            // $a["hello"]
            let key = state.strings.lookup_bytes(s);
            let key = textual::Expr::hack_string(key);
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
            let key = textual::Expr::hack_string(key);
            w.call("hack_field_entry", (base, key, mode))
        }
        MemberKey::QT(_) => {
            // $a?->hello
            todo!();
        }
        MemberKey::W => {
            // $a[]
            todo!();
        }
    }
}
