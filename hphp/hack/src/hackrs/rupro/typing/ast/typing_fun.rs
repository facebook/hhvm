// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use pos::Symbol;
use ty::reason::Reason;

use crate::tast;
use crate::typing::ast::typing_localize::LocalizeEnv;
use crate::typing::ast::typing_return_type_hint::TCReturnTypeHint;
use crate::typing::ast::typing_return_type_hint::TCReturnTypeHintParams;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::shared::typing_return::TypingReturn;
use crate::typing::typing_error::Result;

impl<R: Reason> Infer<R> for oxidized::aast::Fun_<(), ()> {
    type Params = ();
    type Typed = tast::Fun_<R>;

    fn infer(&self, env: &mut TEnv<R>, _params: ()) -> Result<Self::Typed> {
        rupro_todo_assert!(self.tparams.is_empty(), AST);
        rupro_todo_assert!(self.where_constraints.is_empty(), AST);
        rupro_todo_assert!(self.user_attributes.is_empty(), AST);
        rupro_todo_mark!(Coeffects);
        rupro_todo_mark!(Dynamic);
        rupro_todo_mark!(Variance);
        rupro_todo_mark!(Memoization);
        rupro_todo_mark!(Wellformedness);
        rupro_todo_mark!(GenericParameters, "localize");
        rupro_todo_mark!(BindThis);
        rupro_todo_mark!(Disposable);
        rupro_todo_mark!(DisableTypecheckerInternalAttribute);

        let fun_pos = R::Pos::from(&self.name.0);
        let fun_name = Symbol::new(&self.name.1);
        let return_ty = TCReturnTypeHint(&self.ret).infer(
            env,
            TCReturnTypeHintParams {
                is_method: false,
                fun_name,
                fun_pos,
                localize_env: LocalizeEnv::no_subst(),
            },
        )?;
        env.set_return(TypingReturn::make_info(return_ty.clone()));

        let typed_params = self.params.infer(env, ())?;
        let tb = self.body.infer(env, ())?;

        rupro_todo_mark!(
            MissingError,
            "expecting return type hint, should go in TCReturnTypeHint"
        );

        rupro_todo_assert!(env.solve_all_unsolved_tyvars()?.is_none(), MissingError);
        let res = oxidized::aast::Fun_ {
            annotation: env.save(()),
            readonly_this: self.readonly_this.clone(),
            span: self.span.clone(),
            readonly_ret: self.readonly_ret.clone(),
            ret: oxidized::aast::TypeHint(return_ty, self.ret.1.clone()),
            name: self.name.clone(),
            tparams: vec![],
            where_constraints: vec![],
            params: typed_params,
            ctxs: self.ctxs.clone(),
            unsafe_ctxs: self.unsafe_ctxs.clone(),
            fun_kind: self.fun_kind.clone(),
            user_attributes: vec![],
            body: tb,
            external: self.external.clone(),
            doc_comment: self.doc_comment.clone(),
        };
        Ok(res)
    }
}
