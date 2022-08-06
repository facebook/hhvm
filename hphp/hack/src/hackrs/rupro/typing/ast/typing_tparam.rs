// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use oxidized::aast;
use oxidized::aast_defs;
use oxidized::ast_defs;
use pos::Symbol;
use ty::decl;
use ty::local;
use ty::local::Variance;
use ty::reason::Reason;

use crate::tast;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::typing_error::Result;

impl<R: Reason> Infer<R> for oxidized::aast::Tparam<(), ()> {
    type Params = ();
    type Typed = tast::Tparam<R>;

    fn infer(&self, env: &mut TEnv<R>, params: ()) -> Result<Self::Typed> {
        rupro_todo_assert!(self.user_attributes.is_empty(), AST);
        let parameters = self.parameters.infer(env, params)?;
        let tp = tast::Tparam {
            variance: self.variance.clone(),
            name: self.name.clone(),
            parameters,
            constraints: self.constraints.clone(),
            reified: self.reified.clone(),
            user_attributes: vec![],
        };
        Ok(tp)
    }
}

impl<R: Reason> Infer<R> for decl::Tparam<R, decl::Ty<R>> {
    type Params = ();
    type Typed = local::Tparam<R>;

    fn infer(&self, env: &mut TEnv<R>, params: ()) -> Result<Self::Typed> {
        rupro_todo_assert!(self.constraints.is_empty(), AST);
        let tp = local::Tparam {
            variance: self.variance.clone(),
            name: self.name.clone(),
            tparams: self.tparams.infer(env, params)?,
            constraints: vec![],
            reified: self.reified.clone(),
            user_attributes: self.user_attributes.to_vec(),
        };
        Ok(tp)
    }
}

pub struct TCTargs<'a, R: Reason> {
    /// A list of type parameters for the function type.
    pub tparams: &'a [decl::Tparam<R, ty::decl::Ty<R>>],
}

pub struct TCTargsParams<'a> {
    /// The positions where the type parameters are used.
    pub use_pos: &'a oxidized::pos::Pos,

    /// The name where the type parameters are used.
    pub use_name: Symbol,
}

impl<'a, R: Reason> Infer<R> for TCTargs<'a, R> {
    type Params = TCTargsParams<'a>;
    type Typed = Vec<tast::Targ<R>>;

    fn infer(&self, env: &mut TEnv<R>, params: TCTargsParams<'a>) -> Result<Self::Typed> {
        self.tparams
            .iter()
            .map(|tparam| mk_implicit_targ(env, tparam, &params))
            .collect()
    }
}

fn mk_implicit_targ<'a, R: Reason>(
    env: &mut TEnv<R>,
    tparam: &decl::Tparam<R, ty::decl::Ty<R>>,
    params: &TCTargsParams<'a>,
) -> Result<tast::Targ<R>> {
    let wildcard_hint = aast_defs::Hint(
        params.use_pos.clone(),
        Box::new(aast_defs::Hint_::Happly(
            ast_defs::Id(
                oxidized::pos::Pos::make_none(),
                special_names::typehints::wildcard.to_string(),
            ),
            vec![],
        )),
    );
    let use_pos = R::Pos::from(params.use_pos);
    let tvar = env.fresh(
        Variance::Bivariant,
        use_pos.clone(),
        Some(R::type_variable_generics(
            use_pos,
            tparam.name.id().0,
            params.use_name.clone(),
        )),
    );
    Ok(aast::Targ(tvar, wildcard_hint))
}
