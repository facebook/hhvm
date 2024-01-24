// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust::autoimported_functions;
use naming_special_names_rust::pseudo_functions;
use naming_special_names_rust::user_attributes;
use nast::CallExpr;
use nast::Expr;
use nast::Expr_;
use nast::Fun_;
use nast::Id;
use nast::Method_;
use nast::ParamKind;
use nast::Stmt;
use nast::Stmt_;
use oxidized::ast::Pos;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ElabCrossPackagePass;

impl Pass for ElabCrossPackagePass {
    fn on_ty_method__bottom_up(&mut self, _env: &Env, elem: &mut Method_) -> ControlFlow<()> {
        for ua in &elem.user_attributes {
            if let user_attributes::CROSS_PACKAGE = ua.name.name() {
                ua.params
                    .iter()
                    .for_each(|pkg| elem.body.fb_ast.insert(0, assert_package_is_loaded(pkg)));
                break;
            }
        }
        ControlFlow::Continue(())
    }

    fn on_ty_fun__bottom_up(&mut self, _env: &Env, elem: &mut Fun_) -> ControlFlow<()> {
        for ua in &elem.user_attributes {
            if let user_attributes::CROSS_PACKAGE = ua.name.name() {
                ua.params
                    .iter()
                    .for_each(|pkg| elem.body.fb_ast.insert(0, assert_package_is_loaded(pkg)));
                break;
            }
        }
        ControlFlow::Continue(())
    }
}

fn assert_package_is_loaded(pkg: &Expr) -> Stmt {
    let pos = || Pos::NONE;
    let msg = format!("Package {} should be loaded", pkg.2.as_string().unwrap());

    // HH\invariant(HH\package_exists($pkg), "Package $pkg should be loaded")
    let invariant_call = Expr(
        (),
        pos(),
        Expr_::Call(Box::new(CallExpr {
            func: Expr(
                (),
                pos(),
                Expr_::Id(Box::new(Id(
                    pos(),
                    autoimported_functions::INVARIANT.into(),
                ))),
            ),
            targs: vec![],
            args: vec![
                (
                    ParamKind::Pnormal,
                    Expr(
                        (),
                        pos(),
                        Expr_::Call(Box::new(CallExpr {
                            func: Expr(
                                (),
                                pos(),
                                Expr_::Id(Box::new(Id(
                                    pos(),
                                    pseudo_functions::PACKAGE_EXISTS.into(),
                                ))),
                            ),
                            targs: vec![],
                            args: vec![(ParamKind::Pnormal, { pkg.clone() })],
                            unpacked_arg: None,
                        })),
                    ),
                ),
                (ParamKind::Pnormal, {
                    Expr((), pos(), Expr_::String(msg.into()))
                }),
            ],
            unpacked_arg: None,
        })),
    );
    Stmt(pos(), Stmt_::Expr(Box::new(invariant_call)))
}
