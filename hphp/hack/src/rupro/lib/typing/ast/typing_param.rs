// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::reason::Reason;
use crate::tast;
use crate::typing::ast::typing_localize::LocalizeEnv;
use crate::typing::ast::typing_trait::TC;
use crate::typing::env::typing_env::TEnv;
use crate::typing::hint_utils::HintUtils;
use crate::typing::typing_error::Result;
use crate::typing_defs::{ParamMode, Ty};
use crate::utils::core::LocalId;
use pos::Symbol;

/// Type function parameters and bind the variables in the environment.
impl<R: Reason> TC<R> for oxidized::aast::FunParam<(), ()> {
    type Params = ();
    type Typed = tast::FunParam<R>;

    fn infer(&self, env: &TEnv<R>, _params: ()) -> Result<Self::Typed> {
        let ty = match HintUtils::fun_param(self) {
            None => Ty::any(R::witness(R::Pos::from(&self.pos))),
            Some(ty) => ty.infer(env, LocalizeEnv::no_subst())?,
        };
        assert!(self.expr.is_none(), "unimplemented");
        assert!(self.user_attributes.is_empty(), "unimplemented");
        assert!(self.type_hint.1.is_some(), "unimplemented");

        let name = Symbol::new(&self.name);
        let pos = R::Pos::from(&self.pos);
        let id = LocalId::new_unscoped(name);
        let param_mode = ParamMode::from(&self.callconv);
        env.set_param(id.clone(), ty.clone(), pos.clone(), param_mode);
        env.set_local(false, id, ty.clone(), pos);
        let fp = tast::FunParam {
            annotation: ty.clone(),
            type_hint: oxidized::aast::TypeHint(ty, self.type_hint.1.clone()),
            is_variadic: self.is_variadic,
            pos: self.pos.clone(),
            name: self.name.clone(),
            expr: None,
            readonly: self.readonly,
            callconv: self.callconv.clone(),
            user_attributes: vec![],
            visibility: self.visibility,
        };
        Ok(fp)
    }
}
