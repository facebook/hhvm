// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ty::reason::Reason;

use crate::tast;
use crate::typing::ast::typing_expr::TCExprParams;
use crate::typing::ast::typing_localize::LocalizeEnv;
use crate::typing::ast::typing_trait::Infer;
use crate::typing::env::typing_env::TEnv;
use crate::typing::hint_utils::HintUtils;
use crate::typing::typing_error::Result;

impl<R: Reason> Infer<R> for oxidized::aast::ClassVar<(), ()> {
    type Params = ();
    type Typed = tast::ClassVar<R>;

    fn infer(&self, env: &mut TEnv<R>, _params: ()) -> Result<Self::Typed> {
        rupro_todo_assert!(self.user_attributes.is_empty(), AST);
        rupro_todo_mark!(MissingError, "missing hint");
        rupro_todo_mark!(Dynamic);

        let cty = HintUtils::type_hint(&self.type_)
            .map(|decl_cty| decl_cty.infer(env, LocalizeEnv::no_subst()))
            .transpose()?
            .unwrap();
        let typed_cv_expr = self
            .expr
            .as_ref()
            .map(|e| {
                rupro_todo_mark!(BidirectionalTC);
                e.infer(env, TCExprParams::empty_locals())
            })
            .transpose()?;
        let res = tast::ClassVar {
            final_: self.final_,
            xhp_attr: self.xhp_attr.clone(),
            abstract_: self.abstract_,
            readonly: self.readonly,
            visibility: self.visibility.clone(),
            type_: oxidized::aast::TypeHint(cty, self.type_.1.clone()),
            id: self.id.clone(),
            expr: typed_cv_expr,
            user_attributes: vec![],
            doc_comment: self.doc_comment.clone(),
            is_promoted_variadic: self.is_promoted_variadic,
            is_static: self.is_static,
            span: self.span.clone(),
        };
        Ok(res)
    }
}
