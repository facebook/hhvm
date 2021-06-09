// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

use decl_provider::DeclProvider;
use hhbc_by_ref_ast_class_expr::ClassExpr;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhas_body::HhasBodyEnv;
use oxidized::{ast, ast_defs, pos::Pos};

pub trait SpecialClassResolver {
    fn resolve<'a>(&self, env: Option<&'a HhasBodyEnv>, id: &'a str) -> Cow<'a, str>;
}

impl<'arena, 'decl, D: DeclProvider<'decl>> SpecialClassResolver for Emitter<'arena, 'decl, D> {
    fn resolve<'a>(&self, env: Option<&'a HhasBodyEnv>, id: &'a str) -> Cow<'a, str> {
        let class_expr = match env {
            None => ClassExpr::expr_to_class_expr_(
                self,
                true,
                true,
                None,
                None,
                ast::Expr(
                    Pos::make_none(),
                    ast::Expr_::mk_id(ast_defs::Id(Pos::make_none(), id.into())),
                ),
            ),
            Some(body_env) => ClassExpr::expr_to_class_expr_(
                self,
                true,
                true,
                body_env
                    .class_info
                    .as_ref()
                    .map(|(k, s)| (k.clone(), s.as_str())),
                body_env.parent_name.clone(),
                ast::Expr(
                    Pos::make_none(),
                    ast::Expr_::mk_id(ast_defs::Id(Pos::make_none(), id.into())),
                ),
            ),
        };
        match class_expr {
            ClassExpr::Id(ast_defs::Id(_, name)) => Cow::Owned(name),
            _ => Cow::Borrowed(id),
        }
    }
}
