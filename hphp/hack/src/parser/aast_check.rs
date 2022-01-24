// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust::coeffects;
use naming_special_names_rust::special_idents;

use oxidized::{
    aast,
    aast_visitor::{visit, AstParams, Node, Visitor},
    ast, ast_defs,
    pos::Pos,
};
use parser_core_types::{
    syntax_error,
    syntax_error::{Error as ErrorMsg, SyntaxError},
};

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
        Hsoft(h) => awaitable_type_args(&h),
        _ => None,
    }
}

fn is_policied_local(ctxs: &Option<ast::Contexts>) -> bool {
    match ctxs {
        Some(c) => {
            for hint in &c.1 {
                if let aast::Hint_::Happly(ast_defs::Id(_, id), _) = &*hint.1 {
                    if id.as_str() == coeffects::POLICIED_LOCAL
                        || id.as_str() == coeffects::ZONED_LOCAL
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
    in_methodish: bool,
    in_classish: bool,
    in_static_methodish: bool,
    is_policied_local_fun: bool,
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
            .push(SyntaxError::make(start_offset, end_offset, msg));
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
                None => self.add_error(&hint.0, syntax_error::invalid_async_return_hint),
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
                in_methodish: true,
                in_static_methodish: m.static_,
                is_policied_local_fun: is_policied_local(&m.ctxs),
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
                in_methodish: true,
                in_static_methodish: c.in_static_methodish,
                is_policied_local_fun: is_policied_local(&f.ctxs),
                ..*c
            },
            self,
        )
    }

    fn visit_expr(&mut self, c: &mut Context, p: &aast::Expr<(), ()>) -> Result<(), ()> {
        use aast::{ClassId, ClassId_::*, Expr, Expr_::*, Lid};

        if let Await(_) = p.2 {
            if !c.in_methodish {
                self.add_error(&p.1, syntax_error::toplevel_await_use);
            }
        } else if let Some((Expr(_, _, f), ..)) = p.2.as_call() {
            if let Some((ClassId(_, _, CIexpr(Expr(_, pos, Id(id)))), ..)) = f.as_class_const() {
                if Self::name_eq_this_and_in_static_method(c, &id.1) {
                    self.add_error(&pos, syntax_error::this_in_static);
                }
            }
        } else if let Some(Lid(pos, (_, name))) = p.2.as_lvar() {
            if Self::name_eq_this_and_in_static_method(c, name) {
                self.add_error(pos, syntax_error::this_in_static);
            }
        } else if let Some((f, ..)) = p.2.as_efun() {
            match f.ctxs {
                None if c.is_policied_local_fun && c.is_typechecker => {
                    self.add_error(&f.span, syntax_error::closure_in_local_context)
                }
                _ => {}
            }
        } else if let Some((f, ..)) = p.2.as_lfun() {
            match f.ctxs {
                None if c.is_policied_local_fun && c.is_typechecker => {
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
        in_methodish: false,
        in_classish: false,
        in_static_methodish: false,
        is_policied_local_fun: false,
        is_typechecker,
    };
    visit(&mut checker, &mut context, program).unwrap();
    checker.errors
}
