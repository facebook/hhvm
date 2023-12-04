// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust::coeffects;
use naming_special_names_rust::special_idents;
use oxidized::aast;
use oxidized::aast_visitor::visit;
use oxidized::aast_visitor::AstParams;
use oxidized::aast_visitor::Node;
use oxidized::aast_visitor::Visitor;
use oxidized::ast;
use oxidized::ast_defs;
use oxidized::pos::Pos;
use parser_core_types::syntax_error;
use parser_core_types::syntax_error::Error as ErrorMsg;
use parser_core_types::syntax_error::SyntaxError;
use parser_core_types::syntax_error::SyntaxQuickfix;

/// Does this hint look like `Awaitable<Foo>` or `<<__Soft>> Awaitable<Foo>`?
///
/// If it does, return the type arguments present.
fn awaitable_type_args(hint: &ast::Hint) -> Option<&[ast::Hint]> {
    use oxidized::aast_defs::Hint_::*;
    match &*hint.1 {
        // Since `Awaitable` is an autoloaded class, we look for its
        // literal name. Defining a class named `Awaitable` in the
        // current namespace does not affect us.
        Happly(ast::Id(_, s), args)
            if s == "\\HH\\Awaitable" || s == "HH\\Awaitable" || s == "Awaitable" =>
        {
            Some(args)
        }
        Hsoft(h) => awaitable_type_args(h),
        _ => None,
    }
}

fn is_any_local(ctxs: Option<&ast::Contexts>) -> bool {
    match ctxs {
        Some(c) => {
            for hint in &c.1 {
                if let aast::Hint_::Happly(ast_defs::Id(_, id), _) = &*hint.1 {
                    if id.as_str() == coeffects::ZONED_LOCAL
                        || id.as_str() == coeffects::LEAK_SAFE_LOCAL
                        || id.as_str() == coeffects::RX_LOCAL
                    {
                        return true;
                    }
                }
            }
            false
        }
        None => false,
    }
}

struct Context {
    in_classish: bool,
    in_static_methodish: bool,
    is_any_local_fun: bool,
    is_typechecker: bool,
}

struct Checker {
    errors: Vec<SyntaxError>,
}

impl Checker {
    fn new() -> Self {
        Self { errors: vec![] }
    }

    fn add_error(&mut self, pos: &Pos, msg: ErrorMsg) {
        let (start_offset, end_offset) = pos.info_raw();
        self.errors
            .push(SyntaxError::make(start_offset, end_offset, msg, vec![]));
    }

    fn add_error_with_quickfix(
        &mut self,
        start_offset: usize,
        end_offset: usize,
        msg: ErrorMsg,
        quickfix_title: &str,
        edits: Vec<(usize, usize, String)>,
    ) {
        let quickfixes = vec![SyntaxQuickfix {
            title: quickfix_title.into(),
            edits,
        }];
        self.errors
            .push(SyntaxError::make(start_offset, end_offset, msg, quickfixes));
    }

    fn name_eq_this_and_in_static_method(c: &Context, name: impl AsRef<str>) -> bool {
        c.in_classish
            && c.in_static_methodish
            && name.as_ref().eq_ignore_ascii_case(special_idents::THIS)
    }

    fn check_async_ret_hint<Hi>(&mut self, h: &aast::TypeHint<Hi>) {
        match &h.1 {
            Some(hint) => match awaitable_type_args(hint) {
                Some(typeargs) => {
                    if typeargs.len() > 1 {
                        self.add_error(&hint.0, syntax_error::invalid_awaitable_arity)
                    }
                }
                None => {
                    let (start_offset, end_offset) = hint.0.info_raw();
                    let edits = vec![
                        (start_offset, start_offset, "Awaitable<".into()),
                        (end_offset, end_offset, ">".into()),
                    ];
                    self.add_error_with_quickfix(
                        start_offset,
                        end_offset,
                        syntax_error::invalid_async_return_hint,
                        "Make return type Awaitable",
                        edits,
                    );
                }
            },
            None => {}
        }
    }
}

impl<'ast> Visitor<'ast> for Checker {
    type Params = AstParams<Context, ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
        self
    }

    fn visit_class_(&mut self, c: &mut Context, p: &aast::Class_<(), ()>) -> Result<(), ()> {
        p.recurse(
            &mut Context {
                in_classish: true,
                ..*c
            },
            self,
        )
    }

    fn visit_method_(&mut self, c: &mut Context, m: &aast::Method_<(), ()>) -> Result<(), ()> {
        if m.fun_kind == ast::FunKind::FAsync {
            self.check_async_ret_hint(&m.ret);
        }
        m.recurse(
            &mut Context {
                in_static_methodish: m.static_,
                is_any_local_fun: is_any_local(m.ctxs.as_ref()),
                ..*c
            },
            self,
        )
    }

    fn visit_fun_(&mut self, c: &mut Context, f: &aast::Fun_<(), ()>) -> Result<(), ()> {
        if f.fun_kind == ast::FunKind::FAsync {
            self.check_async_ret_hint(&f.ret);
        }

        f.recurse(
            &mut Context {
                in_static_methodish: c.in_static_methodish,
                is_any_local_fun: is_any_local(f.ctxs.as_ref()),
                ..*c
            },
            self,
        )
    }

    fn visit_stmt(&mut self, c: &mut Context, p: &aast::Stmt<(), ()>) -> Result<(), ()> {
        p.recurse(c, self)
    }

    fn visit_expr(&mut self, c: &mut Context, p: &aast::Expr<(), ()>) -> Result<(), ()> {
        use aast::CallExpr;
        use aast::ClassId;
        use aast::ClassId_::*;
        use aast::Expr;
        use aast::Expr_::*;
        use aast::Lid;

        if let Some(CallExpr {
            func: Expr(_, _, f),
            args,
            ..
        }) = p.2.as_call()
        {
            if let Some((ClassId(_, _, CIexpr(Expr(_, pos, Id(id)))), ..)) = f.as_class_const() {
                if Self::name_eq_this_and_in_static_method(c, &id.1) {
                    self.add_error(pos, syntax_error::this_in_static);
                }
            } else if let Some(ast::Id(_, fun_name)) = f.as_id() {
                if (fun_name == "\\HH\\meth_caller"
                    || fun_name == "HH\\meth_caller"
                    || fun_name == "meth_caller")
                    && args.len() == 2
                {
                    if let (_, Expr(_, pos, String(classname))) = &args[0] {
                        if classname.contains(&b'$') {
                            self.add_error(pos, syntax_error::dollar_sign_in_meth_caller_argument);
                        }
                    }
                    if let (_, Expr(_, pos, String(method_name))) = &args[1] {
                        if method_name.contains(&b'$') {
                            self.add_error(pos, syntax_error::dollar_sign_in_meth_caller_argument);
                        }
                    }
                }
            }
        } else if let Some(Lid(pos, (_, name))) = p.2.as_lvar() {
            if Self::name_eq_this_and_in_static_method(c, name) {
                self.add_error(pos, syntax_error::this_in_static);
            }
        } else if let Some(efun) = p.2.as_efun() {
            match efun.fun.ctxs {
                None if c.is_any_local_fun && c.is_typechecker => {
                    self.add_error(&efun.fun.span, syntax_error::closure_in_local_context)
                }
                _ => {}
            }
        } else if let Some((f, ..)) = p.2.as_lfun() {
            match f.ctxs {
                None if c.is_any_local_fun && c.is_typechecker => {
                    self.add_error(&f.span, syntax_error::closure_in_local_context)
                }
                _ => {}
            }
        }
        p.recurse(c, self)
    }
}

pub fn check_program(program: &aast::Program<(), ()>, is_typechecker: bool) -> Vec<SyntaxError> {
    let mut checker = Checker::new();
    let mut context = Context {
        in_classish: false,
        in_static_methodish: false,
        is_any_local_fun: false,
        is_typechecker,
    };
    visit(&mut checker, &mut context, program).unwrap();
    checker.errors
}
