// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use nast::ClassConcreteTypeconst;
use nast::ClassTypeconstDef;
use nast::Expr_;
use nast::Hint;
use nast::Hint_;

use crate::prelude::*;
#[derive(Copy, Clone, Default)]
pub struct ValidateHintHrefinementPass {
    context: Option<RfmtCtxt>,
    tc_is_context: bool,
}

impl Pass for ValidateHintHrefinementPass {
    fn on_ty_expr__top_down(&mut self, _env: &Env, elem: &mut Expr_) -> ControlFlow<()> {
        match elem {
            Expr_::Is(..) => self.context = Some(RfmtCtxt::IsExpr),
            Expr_::As(..) => self.context = Some(RfmtCtxt::AsExpr),
            _ => self.context = None,
        }
        Continue(())
    }

    fn on_ty_hint_bottom_up(&mut self, env: &Env, elem: &mut Hint) -> ControlFlow<()> {
        if let Some(ctxt) = self.context
            && matches!(&*elem.1, Hint_::Hrefinement(..))
        {
            env.emit_error(NastCheckError::RefinementInTypestruct {
                pos: elem.0.clone(),
                kind: ctxt.to_string(),
            })
        }
        Continue(())
    }

    fn on_ty_class_typeconst_def_top_down(
        &mut self,
        _: &Env,
        elem: &mut ClassTypeconstDef,
    ) -> ControlFlow<()> {
        self.tc_is_context = elem.is_ctx;
        Continue(())
    }

    fn on_fld_class_abstract_typeconst_default_top_down(
        &mut self,
        _: &Env,
        _: &mut Option<Hint>,
    ) -> ControlFlow<()> {
        self.context = Some(if self.tc_is_context {
            RfmtCtxt::CtxtConst
        } else {
            RfmtCtxt::TyConst
        });
        Continue(())
    }

    fn on_ty_class_concrete_typeconst_top_down(
        &mut self,
        _: &Env,
        _: &mut ClassConcreteTypeconst,
    ) -> ControlFlow<()> {
        self.context = Some(if self.tc_is_context {
            RfmtCtxt::CtxtConst
        } else {
            RfmtCtxt::TyConst
        });
        Continue(())
    }

    fn on_fld_typedef_kind_top_down(&mut self, _: &Env, _: &mut Hint) -> ControlFlow<()> {
        self.context = Some(RfmtCtxt::TyAlias);
        Continue(())
    }
}

#[derive(Copy, Clone)]
enum RfmtCtxt {
    IsExpr,
    AsExpr,
    CtxtConst,
    TyConst,
    TyAlias,
}

impl ToString for RfmtCtxt {
    fn to_string(&self) -> String {
        match self {
            RfmtCtxt::IsExpr => "an is-expression".to_string(),
            RfmtCtxt::AsExpr => "an as-expression".to_string(),
            RfmtCtxt::CtxtConst => "a context constant".to_string(),
            RfmtCtxt::TyConst => "a type constant".to_string(),
            RfmtCtxt::TyAlias => "a type alias".to_string(),
        }
    }
}
