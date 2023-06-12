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
use itertools::Itertools;
use naming_special_names_rust::special_idents;
use textual::Expr;
use textual::Ty;

use crate::func::FuncState;
use crate::hack;
use crate::mangle;
use crate::textual;
use crate::textual::Sid;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// Intrinsics used by member operations:
///
///   hack_array_append
///   hack_array_cow_incr
///   hack_array_cow_set
///   hack_array_get
///   hack_array_get
///   hack_prop_get
///
/// We break down member operations into sequences of hack_array_get() and
/// property accesses.
///
/// $a[k1][k2]->p1[k3][k4] = v1;
///
/// Becomes:
///   n0 = hack_array_get(&$a, k1, k2)
///   n1 = n0.?.p1
///   hack_array_set(n1, k3, k4, v1)
///
pub(crate) fn write(
    state: &mut FuncState<'_, '_, '_>,
    iid: InstrId,
    mop: &ir::instr::MemberOp,
) -> Result {
    let sid = emit_mop(state, mop)?;
    state.set_iid(iid, sid);
    Ok(())
}

fn emit_mop(state: &mut FuncState<'_, '_, '_>, mop: &ir::instr::MemberOp) -> Result<Sid> {
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

enum Pending {
    /// No operation pending. This should only ever be a transient state while
    /// we're consuming a previous one.
    None,
    /// We have a base with no array or property accessess yet. Since we have a
    /// MemberOp we know this will be replaced before we're done.
    Base(Expr),
    /// We're currently processing a sequence of array accesses.
    ArrayGet { base: Expr, dim: Vec<Expr> },
    /// We're currently processing a sequence of property accesses.
    PropGet {
        base: Expr,
        dim: Vec<(Expr, PropOp)>,
    },
}

impl Pending {
    /// Read the current pending operation and return the value it represents.
    fn read(self, state: &mut FuncState<'_, '_, '_>) -> Result<Sid> {
        match self {
            Pending::None | Pending::Base(_) => unreachable!(),
            Pending::ArrayGet { base, dim } => {
                let params = std::iter::once(base).chain(dim.into_iter()).collect_vec();
                state.call_builtin(hack::Builtin::HackArrayGet, params)
            }
            Pending::PropGet { base, dim } => {
                let value = match base {
                    Expr::Sid(sid) => sid,
                    _ => state.load_mixed(base)?,
                };
                let value = Self::read_prop_dims(state, dim, value)?;
                Ok(value)
            }
        }
    }

    /// Read the current pending operation and return the address of the value it
    /// represents.
    fn read_base(self, state: &mut FuncState<'_, '_, '_>) -> Result<Expr> {
        Ok(match self {
            Pending::None | Pending::Base(_) => unreachable!(),
            Pending::ArrayGet { .. } => {
                // I don't believe this state is possible but I left it separate
                // so we can see the error if it ever comes up.
                unreachable!();
            }
            Pending::PropGet { base, mut dim } => {
                let value = match base {
                    Expr::Sid(sid) => sid,
                    _ => state.load_mixed(base)?,
                };
                let final_ = dim.pop().unwrap();
                let value = Self::read_prop_dims(state, dim, value)?;
                Self::prop_expr(state, value.into(), final_)?
            }
        })
    }

    /// Write a value to the access this represents.
    fn write(self, state: &mut FuncState<'_, '_, '_>, src: Sid) -> Result {
        match self {
            Pending::None | Pending::Base(_) => unreachable!(),
            Pending::ArrayGet { base, dim } => {
                let params = std::iter::once(base)
                    .chain(dim.into_iter())
                    .chain(std::iter::once(src.into()))
                    .collect_vec();
                state.call_builtin(hack::Builtin::HackArrayCowSet, params)?;
            }
            Pending::PropGet { base, mut dim } => {
                let value = match base {
                    Expr::Sid(sid) => sid,
                    _ => state.load_mixed(base)?,
                };
                let final_ = dim.pop().unwrap();
                let value = Self::read_prop_dims(state, dim, value)?;
                let final_ = Self::prop_expr(state, value.into(), final_)?;
                state.store_mixed(final_, src)?;
            }
        }
        Ok(())
    }

    fn write_incr(self, state: &mut FuncState<'_, '_, '_>, pre: Expr, post: Expr) -> Result<Sid> {
        Ok(match self {
            Pending::None | Pending::Base(_) => unreachable!(),
            Pending::ArrayGet { base, dim } => {
                let params = std::iter::once(base)
                    .chain(dim.into_iter())
                    .chain(std::iter::once(pre))
                    .chain(std::iter::once(post))
                    .collect_vec();
                state.call_builtin(hack::Builtin::HackArrayCowIncr, params)?
            }
            Pending::PropGet { .. } => unreachable!(),
        })
    }

    fn read_prop_dims(
        state: &mut FuncState<'_, '_, '_>,
        dim: Vec<(Expr, PropOp)>,
        mut value: Sid,
    ) -> Result<Sid> {
        for key in dim {
            let expr = Self::prop_expr(state, value.into(), key)?;
            value = state.load_mixed(expr)?;
        }
        Ok(value)
    }

    fn prop_expr(
        state: &mut FuncState<'_, '_, '_>,
        base: Expr,
        (prop_expr, prop_op): (Expr, PropOp),
    ) -> Result<Expr> {
        use textual::Const;
        Ok(match (prop_expr, prop_op) {
            (Expr::Const(Const::String(key)), PropOp::NullThrows) => {
                Expr::field(base, Ty::unknown(), key)
            }
            (prop_expr, prop_op) => {
                let null_safe = match prop_op {
                    PropOp::NullThrows => Const::False,
                    PropOp::NullSafe => Const::True,
                };
                state
                    .call_builtin(
                        hack::Builtin::HackPropGet,
                        (base, prop_expr, Expr::Const(null_safe)),
                    )?
                    .into()
            }
        })
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
                self.pending = Pending::Base(base);
            }
            BaseOp::BaseGC { .. } => {
                // Get base from global name: STDIN[k]
                let src = self.next_operand();
                let base = self
                    .state
                    .call_builtin(hack::Builtin::BaseGetSuperglobal, [src])?;
                self.pending = Pending::Base(base.into());
            }
            BaseOp::BaseH { loc: _ } => {
                // Get base from $this: $this[k]
                // Just pretend to be a BaseL w/ $this.
                let lid = LocalId::Named(self.state.strings.intern_str(special_idents::THIS));
                let base = base_from_lid(lid);
                self.pending = Pending::Base(base);
            }
            BaseOp::BaseL {
                mode: _,
                readonly: _,
                loc: _,
            } => {
                // Get base from local: $x[k]
                let base = base_from_lid(self.next_local());
                self.pending = Pending::Base(base);
            }
            BaseOp::BaseSC {
                mode: _,
                readonly: _,
                loc: _,
            } => {
                // Get base from static property: C::$x[k]
                let prop = self.next_operand();
                let base = self.next_operand();
                self.pending = Pending::Base(base);
                self.push_prop_dim(prop, PropOp::NullThrows)?;
            }
            BaseOp::BaseST {
                mode: _,
                readonly: _,
                loc: _,
                prop,
            } => {
                // Get base from static property: C::$x[k]
                let base = self.next_operand();
                self.pending = Pending::Base(base);
                let key = {
                    let key = self.state.strings.lookup_bytes(prop.id);
                    crate::util::escaped_string(&key)
                };
                self.push_prop_dim(key.into(), PropOp::NullThrows)?;
            }
        };
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
                self.push_prop_dim(key, PropOp::NullThrows)?;
            }
            MemberKey::PL => {
                // $a->{$b}
                let key = self.next_local();
                let key = self.state.load_mixed(Expr::deref(key))?;
                self.push_prop_dim(key.into(), PropOp::NullThrows)?;
            }
            MemberKey::PT(prop) => {
                // $a->hello
                let key = {
                    let key = self.state.strings.lookup_bytes(prop.id);
                    crate::util::escaped_string(&key)
                };
                self.push_prop_dim(key.into(), PropOp::NullThrows)?;
            }
            MemberKey::QT(prop) => {
                // $a?->hello
                let key = {
                    let key = self.state.strings.lookup_bytes(prop.id);
                    crate::util::escaped_string(&key)
                };
                self.push_prop_dim(key.into(), PropOp::NullSafe)?;
            }
            MemberKey::W => {
                // $a[]
                self.push_array_dim_append()?;
            }
        }
        Ok(())
    }

    fn finish(mut self, final_op: &FinalOp) -> Result<Sid> {
        use textual::Const;

        let key = final_op.key().unwrap();
        self.write_entry(key)?;

        let value: Sid = match *final_op {
            FinalOp::IncDecM { inc_dec_op, .. } => {
                let (pre, post) = match inc_dec_op {
                    ir::IncDecOp::PreInc => (Const::Int(1), Const::Int(0)),
                    ir::IncDecOp::PreDec => (Const::Int(-1), Const::Int(0)),
                    ir::IncDecOp::PostInc => (Const::Int(0), Const::Int(1)),
                    ir::IncDecOp::PostDec => (Const::Int(0), Const::Int(-1)),
                    _ => unreachable!(),
                };
                self.pending
                    .write_incr(self.state, Expr::Const(pre), Expr::Const(post))?
            }
            FinalOp::QueryM { query_m_op, .. } => match query_m_op {
                QueryMOp::CGet | QueryMOp::CGetQuiet => self.pending.read(self.state)?,
                QueryMOp::Isset => {
                    let value = self.pending.read(self.state)?;
                    self.state
                        .call_builtin(hack::Builtin::Hhbc(hack::Hhbc::IsTypeNull), [value])?
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
                src
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
                self.state.write_todo("SetOpM")?
            },
            FinalOp::UnsetM { ref key, .. } => textual_todo! {
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
                self.state.write_todo("UnsetM")?
            },
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
            Pending::ArrayGet { base, mut dim } => {
                dim.push(key);
                (base, dim)
            }
            Pending::PropGet { .. } => {
                let base = pending.read_base(self.state)?;
                (base, vec![key])
            }
        };
        self.pending = Pending::ArrayGet { base, dim };
        Ok(())
    }

    fn push_array_dim_append(&mut self) -> Result {
        let key = Expr::call(
            mangle::FunctionName::Builtin(hack::Builtin::HackArrayAppend),
            (),
        );
        self.push_array_dim(key)
    }

    fn push_prop_dim(&mut self, key: Expr, op: PropOp) -> Result {
        let pending = std::mem::replace(&mut self.pending, Pending::None);
        let (base, dim) = match pending {
            Pending::None => unreachable!(),
            Pending::Base(base) => (base, vec![(key, op)]),
            Pending::ArrayGet { .. } => {
                let base = pending.read(self.state)?.into();
                (base, vec![(key, op)])
            }
            Pending::PropGet { base, mut dim } => {
                dim.push((key, op));
                (base, dim)
            }
        };
        self.pending = Pending::PropGet { base, dim };
        Ok(())
    }

    fn next_operand(&mut self) -> Expr {
        self.state.lookup_vid(self.operands.next().unwrap())
    }

    fn next_local(&mut self) -> LocalId {
        self.locals.next().unwrap()
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

fn base_from_expr(state: &mut FuncState<'_, '_, '_>, src: impl Into<Expr>) -> Result<Expr> {
    // Unfortunately we need base to be a pointer to a value, not a
    // value itself - so store it in `base` so we can return a pointer
    // to `base`.
    let base_lid = base_var(&state.strings);
    state.store_mixed(Expr::deref(base_lid), src.into())?;
    Ok(base_from_lid(base_lid))
}

pub(crate) fn base_from_vid(state: &mut FuncState<'_, '_, '_>, src: ValueId) -> Result<Expr> {
    let src = state.lookup_vid(src);
    base_from_expr(state, src)
}

fn base_from_lid(lid: LocalId) -> Expr {
    Expr::deref(lid)
}
