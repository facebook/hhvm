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
use crate::typing_return::TypingReturnInfo;
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
        _env: &TEnv<R>,
        _pos: &oxidized::pos::Pos,
        stmt: &oxidized::aast::Stmt_<(), ()>,
    ) -> tast::Stmt_<R> {
        use oxidized::aast::Stmt_ as OS;
        use tast::Stmt_ as TS;
        match stmt {
            OS::Noop => TS::Noop,
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
        _pos: R::Pos,
        named_body: &oxidized::aast::FuncBody<(), ()>,
        _f_kind: &oxidized::ast_defs::FunKind,
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
            // TODO(hrust): fun_implicit_return
            unimplemented!()
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
