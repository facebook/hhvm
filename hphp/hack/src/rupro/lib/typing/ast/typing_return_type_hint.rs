// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::decl_defs::DeclTy;
use crate::reason::Reason;
use crate::special_names;
use crate::typing::ast::typing_localize::LocalizeEnv;
use crate::typing::ast::typing_trait::TC;
use crate::typing::env::typing_env::TEnv;
use crate::typing::hint_utils::HintUtils;
use crate::typing::typing_error::Result;
use crate::typing_defs::Ty;
use pos::Symbol;

/// This trait provides typing for return type hints.
///
/// These are just optional hints, so they are wrapped in a newtype wrapper
/// to provide trait ambiguation.
pub struct TCReturnTypeHint<'a>(pub &'a oxidized::aast::TypeHint<()>);

/// Parameters that influence the typing of a return type hint.
pub struct TCReturnTypeHintParams<R: Reason> {
    /// Does the return type hint belong to a function or a method?
    ///
    /// This influences the default return type generated in the absence of
    /// a hint.
    pub is_method: bool,

    /// The function's name can influence the default return type generated
    /// in the absence of a hint: for a method constructor we return void.
    pub fun_name: Symbol,

    /// The function name's position is used as a placeholder position for
    /// any default return type that has to be egnerated in the absence of
    /// a hint.
    pub fun_pos: R::Pos,

    /// We'll have to localize the return type. This influences the localization.
    pub localize_env: LocalizeEnv,
}

impl<'a, R: Reason> TC<R> for TCReturnTypeHint<'a> {
    type Params = TCReturnTypeHintParams<R>;
    type Typed = Ty<R>;

    fn infer(&self, env: &TEnv<R>, params: TCReturnTypeHintParams<R>) -> Result<Self::Typed> {
        let decl_ty = HintUtils::type_hint(self.0);
        let ty = match decl_ty {
            None => make_default_return(params),
            Some(decl_ty) => make_return_type(env, params, decl_ty)?,
        };
        rupro_todo_mark!(Dynamic, "should return a possibly enforced ty");
        rupro_todo_mark!(
            MissingError,
            "enforce return type constraints for generators"
        );
        Ok(ty)
    }
}

fn make_default_return<R: Reason>(params: TCReturnTypeHintParams<R>) -> Ty<R> {
    let reason = R::witness(params.fun_pos.clone());
    if params.is_method && params.fun_name == special_names::members::__construct.as_symbol() {
        Ty::void(reason)
    } else {
        Ty::any(reason)
    }
}

fn make_return_type<R: Reason>(
    env: &TEnv<R>,
    params: TCReturnTypeHintParams<R>,
    ty: DeclTy<R>,
) -> Result<Ty<R>> {
    rupro_todo_mark!(AwaitableAsync);
    ty.infer(env, params.localize_env)
}
