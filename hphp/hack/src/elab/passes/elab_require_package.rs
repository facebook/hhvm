// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_string_utils::strip_global_ns;
use intern::string::StringId;
use intern::string::intern;
use naming_special_names_rust::autoimported_functions;
use naming_special_names_rust::pseudo_functions;
use naming_special_names_rust::user_attributes;
use nast::Block;
use nast::CallExpr;
use nast::Class_;
use nast::Expr;
use nast::Expr_;
use nast::FunDef;
use nast::Id;
use nast::Method_;
use nast::Stmt;
use nast::Stmt_;
use oxidized::ast;

use crate::prelude::*;

#[derive(Copy, Clone, Default)]
pub struct ElabRequirePackagePass {
    in_interface: bool,
    class_name: Option<StringId>,
}

impl Pass for ElabRequirePackagePass {
    fn on_ty_class__top_down(&mut self, _env: &Env, class: &mut Class_) -> ControlFlow<()> {
        self.in_interface = class.kind.is_cinterface();
        self.class_name = Some(intern(strip_global_ns(class.name.name())));
        Continue(())
    }

    fn on_ty_method__top_down(&mut self, _env: &Env, elem: &mut Method_) -> ControlFlow<()> {
        if elem.abstract_ || self.in_interface {
            return Continue(());
        }

        for ua in &elem.user_attributes {
            if user_attributes::is_require_package(ua.name.name()) {
                elem.body
                    .fb_ast
                    .insert(0, assert_package_is_loaded(&ua.params[0]));
                break;
            } else if user_attributes::is_soft_require_package(ua.name.name()) {
                elem.body.fb_ast.insert(
                    0,
                    log_if_package_unloaded(
                        ua,
                        format!("{}:{}", self.class_name.unwrap().as_str(), elem.name.name()),
                    ),
                );
                break;
            }
        }
        Continue(())
    }

    fn on_ty_fun_def_top_down(&mut self, _env: &Env, fun_def: &mut FunDef) -> ControlFlow<()> {
        let elem = &mut fun_def.fun;
        for ua in &elem.user_attributes {
            if user_attributes::is_require_package(ua.name.name()) {
                elem.body
                    .fb_ast
                    .insert(0, assert_package_is_loaded(&ua.params[0]));
                break;
            } else if user_attributes::is_soft_require_package(ua.name.name()) {
                elem.body.fb_ast.insert(
                    0,
                    log_if_package_unloaded(ua, strip_global_ns(fun_def.name.name()).to_string()),
                );
                break;
            }
        }
        Continue(())
    }
}

fn assert_package_is_loaded(pkg: &Expr) -> Stmt {
    let pos = || pkg.pos().clone();
    let msg = format!("Package {} should be loaded", pkg.2.as_string().unwrap());

    // HH\invariant(HH\package_exists($pkg), "Package $pkg should be loaded")
    let invariant_call = Expr(
        (),
        pos(),
        Expr_::mk_call(CallExpr {
            func: Expr(
                (),
                pos(),
                Expr_::mk_id(Id(pos(), autoimported_functions::INVARIANT.into())),
            ),
            targs: vec![],
            args: vec![
                ast::Argument::Anormal(call_package_exists(pkg)),
                ast::Argument::Anormal(Expr((), pos(), Expr_::String(msg.into()))),
            ],
            unpacked_arg: None,
        }),
    );
    Stmt(pos(), Stmt_::mk_expr(invariant_call))
}

fn log_if_package_unloaded(srp_attr: &ast::UserAttribute, fn_name: String) -> Stmt {
    let pkg = &srp_attr.params[0];
    let pos = || pkg.pos().clone();

    let sample_rate = if srp_attr.params.len() < 2 {
        Expr((), pos(), Expr_::Int("1".into()))
    } else {
        srp_attr.params[1].clone()
    };

    let msg = format!(
        "Package {} SoftRequired but not loaded for {}",
        pkg.2.as_string().unwrap(),
        fn_name.as_str(),
    );

    let call_log = Expr(
        (),
        pos(),
        Expr_::mk_call(CallExpr {
            func: Expr(
                (),
                pos(),
                Expr_::mk_id(Id(
                    pos(),
                    sn::std_lib_functions::TRIGGER_SAMPLED_ERROR.to_string(),
                )),
            ),
            targs: vec![],
            args: vec![
                ast::Argument::Anormal(Expr((), pos(), Expr_::mk_string(msg.into()))),
                ast::Argument::Anormal(sample_rate),
            ],
            unpacked_arg: None,
        }),
    );

    Stmt(
        pos(),
        Stmt_::mk_if(
            call_package_exists(pkg),
            Block(vec![]),
            Block(vec![Stmt(pos(), Stmt_::mk_expr(call_log))]),
        ),
    )
}

fn call_package_exists(pkg: &Expr) -> Expr {
    let pos = || pkg.pos().clone();
    Expr(
        (),
        pos(),
        Expr_::mk_call(CallExpr {
            func: Expr(
                (),
                pos(),
                Expr_::mk_id(Id(pos(), pseudo_functions::PACKAGE_EXISTS.into())),
            ),
            targs: vec![],
            args: vec![ast::Argument::Anormal(pkg.clone())],
            unpacked_arg: None,
        }),
    )
}
