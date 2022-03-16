// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use pos::Symbol;

use crate::dependency_registrar::DeclName;
use crate::reason::Reason;
use crate::tast::{self, Tast};
use crate::typing_defs::{ParamMode, Ty};
use crate::typing_env::TEnv;
use crate::typing_return::{TypingReturn, TypingReturnInfo};
use crate::utils::core::LocalId;

pub type Result<T, E = Error> = std::result::Result<T, E>;

/// A system error preventing us from proceeding with typechecking. When we
/// encounter one during a bulk typecheck, we should abort the check, report the
/// error to the user, and log the error to Scuba. In some circumstances (e.g.,
/// decl-consistency errors), we might attempt the bulk check again. Includes
/// decl-provider errors like file-not-found (even though it was listed in our
/// global symbol table), decl-consistency errors (i.e., we detected that a
/// source file on disk changed under our feet), etc.
///
/// This type should not be used for internal compiler errors (i.e., invariant
/// violations in our own logic). In OCaml, those are represented as exceptions
/// which are caught at the typing entrypoint and reported as a Hack error
/// (i.e., `Typing_error.invariant_violation`). In Rust, we should represent
/// these with a panic.
#[derive(thiserror::Error, Debug)]
pub enum Error {
    #[error("{0}")]
    DeclProvider(#[from] crate::typing_decl_provider::Error),
    #[error("Decl Not Found: {0:?}")]
    DeclNotFound(DeclName),
}

pub struct Typing;

pub struct BindParamFlags {
    pub immutable: bool,
    pub can_read_globals: bool,
}

#[derive(Default)]
pub struct TypingFunFlags {
    pub abstract_: bool,
    pub disable: bool,
}

#[derive(Debug)]
pub struct TypingExprFlags<R> {
    _data: std::marker::PhantomData<R>,
}

impl Typing {
    pub fn bind_param<R: Reason>(
        env: &TEnv<R>,
        flags: BindParamFlags,
        ty1: Ty<R>,
        param: &oxidized::aast::FunParam<(), ()>,
    ) -> tast::FunParam<R> {
        let (param_te, ty1) = match param.expr {
            None => (None, ty1),
            Some(_) => unimplemented!(),
        };
        if !param.user_attributes.is_empty() {
            unimplemented!()
        }
        let name = Symbol::new(&param.name);
        let pos = R::Pos::from(&param.pos);
        let id = LocalId::new_unscoped(name);
        let param_mode = ParamMode::from(&param.callconv);
        env.set_param(id.clone(), ty1.clone(), pos.clone(), param_mode);
        env.set_local(flags.immutable, id, ty1.clone(), pos);
        if !param.user_attributes.is_empty() {
            unimplemented!()
        }
        tast::FunParam {
            annotation: ty1.clone(),
            type_hint: oxidized::aast::TypeHint(ty1, param.type_hint.1.clone()),
            is_variadic: param.is_variadic,
            pos: param.pos.clone(),
            name: param.name.clone(),
            expr: param_te,
            readonly: param.readonly,
            callconv: param.callconv.clone(),
            user_attributes: vec![],
            visibility: param.visibility,
        }
    }

    fn stmt_<R: Reason>(
        env: &TEnv<R>,
        _pos: &oxidized::pos::Pos,
        stmt: &oxidized::aast::Stmt_<(), ()>,
    ) -> tast::Stmt_<R> {
        use oxidized::aast::Stmt_ as OS;
        use tast::Stmt_ as TS;
        match stmt {
            OS::Noop => TS::Noop,
            OS::Expr(e) => {
                let (te, _) = Self::expr(env, Default::default(), e);
                // TODO(hrust): exits
                OS::Expr(Box::new(te))
            }
            s => unimplemented!("stmt_: {:?}", s),
        }
    }

    fn stmt<R: Reason>(env: &TEnv<R>, stmt: &oxidized::aast::Stmt<(), ()>) -> tast::Stmt<R> {
        let pos = &stmt.0;
        let st = Self::stmt_(env, pos, &stmt.1);
        oxidized::aast::Stmt(pos.clone(), st)
    }

    fn block<R: Reason>(env: &TEnv<R>, block: &[oxidized::aast::Stmt<(), ()>]) -> tast::Block<R> {
        let mut stl = Vec::new();
        for st in block {
            let st = Self::stmt(env, st);
            match st.1 {
                oxidized::aast::Stmt_::Block(new_stl) => stl.extend(new_stl),
                _ => stl.push(st),
            };
        }
        stl
    }

    fn set_valid_rvalue<R: Reason>(
        env: &TEnv<R>,
        p: R::Pos,
        x: &oxidized::local_id::LocalId,
        ty: Ty<R>,
    ) {
        let x = LocalId::from(x);
        env.set_local(false, x, ty, p);
        // TODO(hverr): set_local_expr_id
        // TODO(hverr): not check_defined
    }

    fn assign<R: Reason>(
        env: &TEnv<R>,
        p: R::Pos,
        e1: &oxidized::aast::Expr<(), ()>,
        pos2: &oxidized::pos::Pos,
        ty2: Ty<R>,
    ) -> (tast::Expr<R>, Ty<R>, Option<(Ty<R>, Ty<R>)>) {
        Self::assign_with_subtype_err_(env, p, (), e1, pos2, ty2)
    }

    fn assign_with_subtype_err_<R: Reason>(
        env: &TEnv<R>,
        p: R::Pos,
        _ur: (),
        e1: &oxidized::aast::Expr<(), ()>,
        _pos2: &oxidized::pos::Pos,
        ty2: Ty<R>,
    ) -> (tast::Expr<R>, Ty<R>, Option<(Ty<R>, Ty<R>)>) {
        // TODO(hrust): Reason.URassign
        // TODO(hrust): Hole
        // TODO(hrust): awaitable
        use oxidized::aast::Expr_::*;
        // TODO(hrust): members
        match &e1.2 {
            Lvar(x) => {
                Self::set_valid_rvalue(env, p, &x.1, ty2.clone());
                let (te, ty) = Self::make_result(env, e1.1.clone(), Lvar(x.clone()), ty2);
                (te, ty, None)
            }
            e => unimplemented!("{:?}", e),
        }
    }

    fn make_result<R: Reason>(
        _env: &TEnv<R>,
        p: oxidized::pos::Pos,
        te: tast::Expr_<R>,
        ty: Ty<R>,
    ) -> (tast::Expr<R>, Ty<R>) {
        // TODO(hrust): Env.set_tyvar_variance
        (Tast::make_typed_expr(p, te, ty.clone()), ty)
    }

    fn expr_<R: Reason>(
        env: &TEnv<R>,
        _flags: TypingExprFlags<R>,
        outer: &oxidized::aast::Expr<(), ()>,
    ) -> (tast::Expr<R>, Ty<R>) {
        use oxidized::aast::Expr_::*;
        let p_ox = &outer.1;
        let p = R::Pos::from(p_ox);
        let e = &outer.2;
        match e {
            Int(s) => Self::make_result(env, p_ox.clone(), Int(s.clone()), Ty::int(R::witness(p))),
            Binop(e) => match &**e {
                (oxidized::ast_defs::Bop::Eq(op_opt), e1, e2) => {
                    let make_result = |env, p, te, ty| {
                        // TODO(hrust): check_assignment
                        Self::make_result::<R>(env, p, te, ty)
                    };
                    match op_opt {
                        Some(_) => unimplemented!("{:?}", e),
                        None => {
                            let (te2, ty2) = Self::expr_(env, Default::default(), e2);
                            let (te1, ty, err_opt) = Self::assign(env, p, e1, &e2.1, ty2);
                            // TODO(hrust): hole_on_err
                            assert!(err_opt.is_none());
                            let te = Binop(Box::new((oxidized::ast_defs::Bop::Eq(None), te1, te2)));
                            make_result(env, p_ox.clone(), te, ty)
                        }
                    }
                }
                _ => unimplemented!("{:?}", e),
            },
            Lvar(id) => {
                // TODO(hrust): accept_using_var
                // TODO(hrust): !check_defined
                let pos = R::Pos::from(&id.0);
                let lid = LocalId::from(&id.1);
                let ty = env.get_local_check_defined(pos, &lid);
                Self::make_result(env, id.0.clone(), Lvar(id.clone()), ty)
            }
            _ => unimplemented!("{:?}", e),
        }
    }

    fn expr<R: Reason>(
        env: &TEnv<R>,
        flags: TypingExprFlags<R>,
        e: &oxidized::aast::Expr<(), ()>,
    ) -> (tast::Expr<R>, Ty<R>) {
        // TODO(hrust): inconsistent tyvar state, expected_ty
        Self::expr_(env, flags, e)
    }

    fn fun_impl<R: Reason>(
        env: &TEnv<R>,
        flags: TypingFunFlags,
        return_: TypingReturnInfo<R>,
        pos: R::Pos,
        named_body: &oxidized::aast::FuncBody<(), ()>,
        f_kind: &oxidized::ast_defs::FunKind,
    ) -> tast::Block<R> {
        env.set_return(return_);
        let tb = if flags.disable {
            unimplemented!()
        } else {
            Self::block(env, &named_body.fb_ast)
        };
        let has_implicit_return = env.has_next();
        // TODO(hrust): hhi
        if has_implicit_return && !flags.abstract_ {
            let ret = env.get_return().return_type();
            TypingReturn::fun_implicit_return(env, pos, ret, f_kind);
        }
        // TODO(hrust): set_fun_tast_info
        tb
    }

    pub fn fun_<R: Reason>(
        env: &TEnv<R>,
        flags: TypingFunFlags,
        return_: TypingReturnInfo<R>,
        pos: R::Pos,
        named_body: &oxidized::aast::FuncBody<(), ()>,
        f_kind: &oxidized::ast_defs::FunKind,
    ) -> tast::Block<R> {
        let ret = env.get_return();
        let params = env.get_params();
        let res = Self::fun_impl(env, flags, return_, pos, named_body, f_kind);
        env.set_params(params);
        env.set_return(ret);
        res
    }

    pub fn type_param<R: Reason>(
        env: &TEnv<R>,
        t: &oxidized::aast::Tparam<(), ()>,
    ) -> tast::Tparam<R> {
        assert!(t.user_attributes.is_empty());
        let parameters = t
            .parameters
            .iter()
            .map(|t| Self::type_param(env, t))
            .collect();
        tast::Tparam {
            variance: t.variance.clone(),
            name: t.name.clone(),
            parameters,
            constraints: t.constraints.clone(),
            reified: t.reified.clone(),
            user_attributes: vec![],
        }
    }

    pub fn expr_with_pure_coeffects<R: Reason>(
        env: &TEnv<R>,
        _expected: Option<Ty<R>>,
        e: &oxidized::aast::Expr<(), ()>,
    ) -> (tast::Expr<R>, Ty<R>) {
        // TODO(hrust): expected, coeffects
        env.reinitialize_locals();
        Self::expr(env, Default::default(), e)
    }
}

impl<R> Default for TypingExprFlags<R> {
    fn default() -> Self {
        Self {
            _data: std::marker::PhantomData,
        }
    }
}
