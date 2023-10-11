// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::bail;
use anyhow::Error;
use ascii::AsciiString;
use ir::instr::BaseOp;
use ir::instr::FinalOp;
use ir::instr::MemberKey;
use ir::InstrId;
use ir::LocalId;
use ir::QueryMOp;
use ir::ValueId;
use itertools::Itertools;
use naming_special_names_rust::special_idents;
use textual::Expr;
use textual::Ty;

use crate::func::FuncState;
use crate::hack;
use crate::mangle::FieldName;
use crate::textual;
use crate::textual::Sid;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// Intrinsics used by member operations:
///
///   hack_array_cow_append
///   hack_array_cow_set
///   hack_array_get
///   hack_get_superglobal
///   hack_prop_get
///   hack_prop_set
///   hack_set_superglobal
///
/// We break down member operations into sequences of hack_array_get() and
/// property accesses.
///
/// $a[k1][k2]->p1[k3][k4] = v1;
///
/// Becomes:
///   n0 = hack_array_get($a, k1, k2)
///   n1: *HackMixed = load n0.?.p1
///   n2 = hack_array_set(n1, k3, k4, v1)
///   store n1 <- n2: *HackMixed
///
pub(crate) fn write(
    state: &mut FuncState<'_, '_, '_>,
    iid: InstrId,
    mop: &ir::instr::MemberOp,
) -> Result {
    if let Some(sid) = emit_mop(state, mop)? {
        state.set_iid(iid, sid);
    }
    Ok(())
}

fn emit_mop(state: &mut FuncState<'_, '_, '_>, mop: &ir::instr::MemberOp) -> Result<Option<Sid>> {
    let mut locals = mop.locals.iter().copied();
    let mut operands = mop.operands.iter().copied();

    let mut emitter = MemberOpEmitter::new(state, &mut locals, &mut operands)?;
    emitter.write_base(&mop.base_op)?;

    for intermediate in mop.intermediate_ops.iter() {
        emitter.write_entry(&intermediate.key)?;
    }

    match &mop.final_op {
        FinalOp::SetRangeM { .. } => todo!(),
        op => emitter.finish(op),
    }
}

#[derive(Copy, Clone)]
enum PropOp {
    NullThrows,
    NullSafe,
}

enum PropKey {
    Expr(Expr),
    Name(FieldName),
}

enum Base {
    Field {
        base: Sid,
        prop: PropKey,
        op: PropOp,
    },
    Local(LocalId),
    Superglobal(Expr),
    Value(Expr),
}

impl Base {
    fn load(&self, state: &mut FuncState<'_, '_, '_>) -> Result<Sid> {
        use textual::Const;
        match *self {
            Base::Field {
                base,
                prop: PropKey::Name(ref key),
                op: PropOp::NullThrows,
            } => state.load_mixed(Expr::field(base, Ty::unknown(), key.clone())),
            Base::Field {
                base,
                prop: PropKey::Name(ref key),
                op: PropOp::NullSafe,
            } => {
                let key = key.display(&state.strings).to_string();
                let prop = Expr::Const(Const::String(
                    AsciiString::from_ascii(key.as_bytes()).unwrap(),
                ));
                let null_safe = Const::True;
                state.call_builtin(
                    hack::Builtin::HackPropGet,
                    (base, prop, Expr::Const(null_safe)),
                )
            }
            Base::Field {
                base,
                prop: PropKey::Expr(ref prop),
                op,
            } => {
                let null_safe = match op {
                    PropOp::NullThrows => Const::False,
                    PropOp::NullSafe => Const::True,
                };
                state.call_builtin(
                    hack::Builtin::HackPropGet,
                    (base, prop.clone(), Expr::Const(null_safe)),
                )
            }
            Base::Local(lid) => state.load_mixed(Expr::deref(lid)),
            Base::Superglobal(ref expr) => {
                state.call_builtin(hack::Builtin::GetSuperglobal, [expr.clone()])
            }
            Base::Value(ref expr) => state.fb.write_expr_stmt(expr.clone()),
        }
    }

    fn store(&self, state: &mut FuncState<'_, '_, '_>, value: Expr) -> Result {
        use textual::Const;
        match *self {
            Base::Field {
                base,
                prop: PropKey::Name(ref key),
                op: PropOp::NullThrows,
            } => state.store_mixed(Expr::field(base, Ty::unknown(), key.clone()), value)?,
            Base::Field {
                base,
                prop: PropKey::Name(ref key),
                op: PropOp::NullSafe,
            } => {
                let key = key.display(&state.strings).to_string();
                let prop = Expr::Const(Const::String(
                    AsciiString::from_ascii(key.as_bytes()).unwrap(),
                ));
                let null_safe = Const::True;
                state.call_builtin(
                    hack::Builtin::HackPropSet,
                    (base, prop, Expr::Const(null_safe), value),
                )?;
            }
            Base::Field {
                base,
                prop: PropKey::Expr(ref prop),
                op,
            } => {
                let null_safe = match op {
                    PropOp::NullThrows => Const::False,
                    PropOp::NullSafe => Const::True,
                };
                state.call_builtin(
                    hack::Builtin::HackPropSet,
                    (base, prop.clone(), Expr::Const(null_safe), value),
                )?;
            }
            Base::Local(lid) => state.store_mixed(Expr::deref(lid), value)?,
            Base::Superglobal(ref expr) => {
                state.call_builtin(hack::Builtin::SetSuperglobal, (expr.clone(), value))?;
            }
            Base::Value(_) => {
                // This is something like `foo()[2] = 3`.  We don't have
                // something to write back into so just ignore it.
            }
        }
        Ok(())
    }

    fn unset(&self, state: &mut FuncState<'_, '_, '_>) -> Result {
        // Treat this as a store of null.
        self.store(state, Expr::Const(textual::Const::Null))
    }
}

enum Pending {
    /// No operation pending. This should only ever be a transient state while
    /// we're consuming a previous one.
    None,
    /// We have a base with no array or property accessess yet. Since we have a
    /// MemberOp we know this will be replaced before we're done.
    Base(Base),
    /// We're currently processing a sequence of array accesses ending with an
    /// append.
    ArrayAppend { base: Base, dim: Vec<Expr> },
    /// We're currently processing a sequence of array accesses.
    ArrayGet { base: Base, dim: Vec<Expr> },
}

impl Pending {
    /// Read the current pending operation and return the value it represents.
    fn read(&self, state: &mut FuncState<'_, '_, '_>) -> Result<Sid> {
        match self {
            Pending::None => unreachable!(),
            Pending::Base(base) => base.load(state),
            Pending::ArrayAppend { .. } => {
                bail!("Cannot use [] with vecs for reading in an lvalue context");
            }
            Pending::ArrayGet { base, dim } => {
                let base_value = base.load(state)?;
                let params = std::iter::once(base_value.into())
                    .chain(dim.iter().cloned())
                    .collect_vec();
                state.call_builtin(hack::Builtin::HackArrayGet, params)
            }
        }
    }

    /// Write a value to the access this represents.
    fn write(self, state: &mut FuncState<'_, '_, '_>, src: Sid) -> Result {
        match self {
            Pending::None => unreachable!(),
            Pending::Base(base) => base.store(state, src.into())?,
            Pending::ArrayAppend { base, dim } => {
                let base_value = base.load(state)?;
                let params = std::iter::once(base_value.into())
                    .chain(dim.into_iter())
                    .chain(std::iter::once(src.into()))
                    .collect_vec();
                let base_value = state.call_builtin(hack::Builtin::HackArrayCowAppend, params)?;
                base.store(state, base_value.into())?;
            }
            Pending::ArrayGet { base, dim } => {
                let base_value = base.load(state)?;
                let params = std::iter::once(base_value.into())
                    .chain(dim.into_iter())
                    .chain(std::iter::once(src.into()))
                    .collect_vec();
                let base_value = state.call_builtin(hack::Builtin::HackArrayCowSet, params)?;
                base.store(state, base_value.into())?;
            }
        }
        Ok(())
    }

    fn unset(self, state: &mut FuncState<'_, '_, '_>) -> Result {
        match self {
            Pending::None => unreachable!(),
            Pending::Base(base) => {
                base.unset(state)?;
            }
            Pending::ArrayAppend { .. } => unreachable!(),
            Pending::ArrayGet { base, dim } => {
                let base_value = base.load(state)?;
                let params = std::iter::once(base_value.into()).chain(dim).collect_vec();
                let base_value = state.call_builtin(hack::Builtin::HackArrayCowUnset, params)?;
                base.store(state, base_value.into())?;
            }
        }
        Ok(())
    }
}

/// The member operation state machine.
///
/// A member operation will consist of a call to write_base(), 0 or more calls
/// to write_entry(), and a call to finish().
///
struct MemberOpEmitter<'a, 'b, 'c, 'd, L, O>
where
    L: Iterator<Item = LocalId>,
    O: Iterator<Item = ValueId>,
{
    locals: L,
    operands: O,
    pending: Pending,
    state: &'a mut FuncState<'b, 'c, 'd>,
}

impl<'a, 'b, 'c, 'd, L, O> MemberOpEmitter<'a, 'b, 'c, 'd, L, O>
where
    L: Iterator<Item = LocalId>,
    O: Iterator<Item = ValueId>,
{
    fn new(state: &'a mut FuncState<'b, 'c, 'd>, locals: L, operands: O) -> Result<Self> {
        Ok(Self {
            locals,
            operands,
            pending: Pending::None,
            state,
        })
    }

    fn write_base(&mut self, base_op: &BaseOp) -> Result {
        assert!(matches!(self.pending, Pending::None));
        match *base_op {
            BaseOp::BaseC { .. } => {
                // Get base from value: foo()[k]
                let base = self.next_operand();
                self.pending = Pending::Base(Base::Value(base));
            }
            BaseOp::BaseGC { .. } => {
                // Get base from global name: $_SERVER[k]
                let src = self.next_operand();
                self.pending = Pending::Base(Base::Superglobal(src));
            }
            BaseOp::BaseH { loc: _ } => {
                // Get base from $this: $this[k]
                // Just pretend to be a BaseL w/ $this.
                let lid = LocalId::Named(self.state.strings.intern_str(special_idents::THIS));
                self.pending = Pending::Base(Base::Local(lid));
            }
            BaseOp::BaseL {
                mode: _,
                readonly: _,
                loc: _,
            } => {
                // Get base from local: $x[k]
                self.pending = Pending::Base(Base::Local(self.next_local()));
            }
            BaseOp::BaseSC {
                mode: _,
                readonly: _,
                loc: _,
            } => {
                // Get base from static property: $x::{$y}[k]
                // TODO: Is this even possible in Hack anymore?
                let prop = self.next_operand();
                let base = self.next_operand();
                self.pending = Pending::Base(Base::Value(base));
                self.push_prop_dim(PropKey::Expr(prop), PropOp::NullThrows)?;
            }
            BaseOp::BaseST {
                mode: _,
                readonly: _,
                loc: _,
                prop,
            } => {
                // Get base from static property with known key: $x::$y[k]
                // We treat C::$x the same as `get_static(C)->x`.
                let base = self.next_operand();
                let key = PropKey::Name(FieldName::prop(prop));
                self.pending = Pending::Base(Base::Value(base));
                self.push_prop_dim(key, PropOp::NullThrows)?;
            }
        }
        Ok(())
    }

    fn write_entry(&mut self, key: &MemberKey) -> Result {
        match *key {
            MemberKey::EC => {
                // $a[foo()]
                let key = self.next_operand();
                self.push_array_dim(key)?;
            }
            MemberKey::EI(i) => {
                // $a[3]
                let idx = self.state.call_builtin(hack::Builtin::Int, [i])?;
                self.push_array_dim(idx.into())?;
            }
            MemberKey::EL => {
                // $a[$b]
                let key = self.next_local();
                let key = self.state.load_mixed(Expr::deref(key))?;
                self.push_array_dim(key.into())?;
            }
            MemberKey::ET(s) => {
                // $a["hello"]
                let key = {
                    let key = self.state.strings.lookup_bytes(s);
                    crate::util::escaped_string(&key)
                };
                let key = self.state.call_builtin(hack::Builtin::String, [key])?;
                self.push_array_dim(key.into())?;
            }
            MemberKey::PC => {
                // $a->{foo()}
                let key = self.next_operand();
                self.push_prop_dim(PropKey::Expr(key), PropOp::NullThrows)?;
            }
            MemberKey::PL => {
                // $a->{$b}
                let key = self.next_local();
                let key = self.state.load_mixed(Expr::deref(key))?;
                self.push_prop_dim(PropKey::Expr(key.into()), PropOp::NullThrows)?;
            }
            MemberKey::PT(prop) => {
                // $a->hello
                self.push_prop_dim(PropKey::Name(FieldName::prop(prop)), PropOp::NullThrows)?;
            }
            MemberKey::QT(prop) => {
                // $a?->hello
                self.push_prop_dim(PropKey::Name(FieldName::prop(prop)), PropOp::NullSafe)?;
            }
            MemberKey::W => {
                // $a[]
                self.push_array_append()?;
            }
        }
        Ok(())
    }

    fn finish(mut self, final_op: &FinalOp) -> Result<Option<Sid>> {
        use textual::Const;

        let key = final_op.key().unwrap();
        self.write_entry(key)?;

        let value: Option<Sid> = match *final_op {
            FinalOp::IncDecM { inc_dec_op, .. } => {
                let pre = self.pending.read(self.state)?;
                let op = match inc_dec_op {
                    ir::IncDecOp::PreInc | ir::IncDecOp::PostInc => hack::Hhbc::Add,
                    ir::IncDecOp::PreDec | ir::IncDecOp::PostDec => hack::Hhbc::Sub,
                    _ => unreachable!(),
                };
                let incr = hack::expr_builtin(hack::Builtin::Int, [Expr::Const(Const::Int(1))]);
                let post = self
                    .state
                    .call_builtin(hack::Builtin::Hhbc(op), (pre, incr))?;
                self.pending.write(self.state, post)?;
                match inc_dec_op {
                    ir::IncDecOp::PreInc | ir::IncDecOp::PreDec => Some(post),
                    ir::IncDecOp::PostInc | ir::IncDecOp::PostDec => Some(pre),
                    _ => unreachable!(),
                }
            }
            FinalOp::QueryM { query_m_op, .. } => match query_m_op {
                QueryMOp::CGet | QueryMOp::CGetQuiet => Some(self.pending.read(self.state)?),
                QueryMOp::Isset => {
                    let value = self.pending.read(self.state)?;
                    let result = self
                        .state
                        .call_builtin(hack::Builtin::Hhbc(hack::Hhbc::IsTypeNull), [value])?;
                    Some(result)
                }
                QueryMOp::InOut => textual_todo! {
                    todo!();
                },
                _ => unreachable!(),
            },
            FinalOp::SetM { .. } => {
                let src = self.next_operand();
                let src = self.state.fb.write_expr_stmt(src)?;
                self.pending.write(self.state, src)?;
                Some(src)
            }
            FinalOp::SetRangeM { .. } => unreachable!(),
            FinalOp::SetOpM { ref key, .. } => textual_todo! {
                match *key {
                    MemberKey::EC => { self.next_operand(); }
                    MemberKey::EI(_) => { }
                    MemberKey::EL => { let _ = self.locals.next(); }
                    MemberKey::ET(_) => { }
                    MemberKey::PC => { }
                    MemberKey::PL => { }
                    MemberKey::PT(_) => { }
                    MemberKey::QT(_) => { }
                    MemberKey::W => { }
                }
                self.next_operand();
                Some(self.state.write_todo("SetOpM")?)
            },
            FinalOp::UnsetM { ref key, .. } => {
                let _ = key;
                self.pending.unset(self.state)?;
                None
            }
        };

        assert!(self.locals.next().is_none());
        assert!(self.operands.next().is_none());

        Ok(value)
    }

    fn push_array_dim(&mut self, key: Expr) -> Result {
        let pending = std::mem::replace(&mut self.pending, Pending::None);
        let (base, dim) = match pending {
            Pending::None => unreachable!(),
            Pending::Base(base) => (base, vec![key]),
            Pending::ArrayAppend { .. } => {
                bail!("Cannot use [] with vecs for reading in an lvalue context")
            }
            Pending::ArrayGet { base, mut dim } => {
                dim.push(key);
                (base, dim)
            }
        };
        self.pending = Pending::ArrayGet { base, dim };
        Ok(())
    }

    fn push_array_append(&mut self) -> Result {
        let pending = std::mem::replace(&mut self.pending, Pending::None);
        let (base, dim) = match pending {
            Pending::None => unreachable!(),
            Pending::Base(base) => (base, vec![]),
            Pending::ArrayAppend { .. } => {
                bail!("Cannot use [] with vecs for reading in an lvalue context")
            }
            Pending::ArrayGet { base, dim } => (base, dim),
        };
        self.pending = Pending::ArrayAppend { base, dim };
        Ok(())
    }

    fn push_prop_dim(&mut self, key: PropKey, op: PropOp) -> Result {
        let pending = std::mem::replace(&mut self.pending, Pending::None);
        let (base, prop, op) = match pending {
            Pending::None => unreachable!(),
            Pending::Base(base) => {
                let base = base.load(self.state)?;
                (base, key, op)
            }
            Pending::ArrayAppend { .. } => {
                bail!("Cannot use [] with vecs for reading in an lvalue context")
            }
            Pending::ArrayGet { .. } => {
                let base = pending.read(self.state)?.into();
                (base, key, op)
            }
        };
        self.pending = Pending::Base(Base::Field { base, prop, op });
        Ok(())
    }

    fn next_operand(&mut self) -> Expr {
        self.state.lookup_vid(self.operands.next().unwrap())
    }

    fn next_local(&mut self) -> LocalId {
        self.locals.next().unwrap()
    }
}
