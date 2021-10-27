// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust::{members, special_idents, user_attributes};
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

fn is_pure(ctxs: &Option<ast::Contexts>) -> bool {
    match ctxs {
        Some(c) => c.1.len() == 0,
        None => false,
    }
}

fn is_mutating_unop(unop: &ast_defs::Uop) -> bool {
    use ast_defs::Uop::*;
    match unop {
        Upincr | Uincr | Updecr | Udecr => true,
        _ => false,
    }
}

// This distinguishes between specified not-pure contexts and unspecified contexts (both of which are not pure).
// The latter is important because for e.g. lambdas without contexts, they should inherit from enclosing functions.
fn is_pure_with_inherited_val(c: &Context, ctxs: &Option<ast::Contexts>) -> bool {
    match ctxs {
        Some(_) => is_pure(ctxs),
        None => c.is_pure,
    }
}

fn is_constructor(s: &ast_defs::Id) -> bool {
    let ast_defs::Id(_, name) = s;
    return name == members::__CONSTRUCT;
}

fn has_ignore_coeffect_local_errors_attr(attrs: &Vec<aast::UserAttribute<(), ()>>) -> bool {
    for attr in attrs.iter() {
        let ast_defs::Id(_, name) = &attr.name;
        if user_attributes::ignore_coeffect_local_errors(name) {
            return true;
        }
    }
    false
}

struct Context {
    in_methodish: bool,
    in_classish: bool,
    in_static_methodish: bool,
    is_pure: bool,
    is_constructor: bool,
    ignore_coeffect_local_errors: bool,
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

    fn check_pure_fn_contexts(&mut self, c: &mut Context, e: &aast::Expr<(), ()>) {
        if !c.is_pure || c.is_constructor || c.ignore_coeffect_local_errors {
            return;
        }
        if let Some((bop, lhs, _)) = e.2.as_binop() {
            if let ast_defs::Bop::Eq(_) = bop {
                self.check_is_obj_property_write_expr(&e, &lhs);
            }
        }
        if let Some((unop, expr)) = e.2.as_unop() {
            if is_mutating_unop(&unop) {
                self.check_is_obj_property_write_expr(&e, &expr);
            }
        }
    }

    fn check_is_obj_property_write_expr(
        &mut self,
        top_level_expr: &aast::Expr<(), ()>,
        expr: &aast::Expr<(), ()>,
    ) {
        if let Some(_) = expr.2.as_obj_get() {
            self.add_error(
                &top_level_expr.1,
                syntax_error::object_property_write_in_pure_fn,
            );
            return;
        } else if let Some((arr, _)) = expr.2.as_array_get() {
            self.check_is_obj_property_write_expr(top_level_expr, &arr);
        }
    }
}

impl<'ast> Visitor<'ast> for Checker {
    type P = AstParams<Context, ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
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
                is_pure: is_pure(&m.ctxs),
                is_constructor: is_constructor(&m.name),
                ignore_coeffect_local_errors: has_ignore_coeffect_local_errors_attr(
                    &m.user_attributes,
                ),
                ..*c
            },
            self,
        )
    }

    fn visit_fun_def(&mut self, c: &mut Context, d: &aast::FunDef<(), ()>) -> Result<(), ()> {
        d.recurse(
            &mut Context {
                is_pure: is_pure(&d.fun.ctxs),
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
                is_pure: is_pure_with_inherited_val(c, &f.ctxs),
                is_constructor: is_constructor(&f.name),
                ignore_coeffect_local_errors: has_ignore_coeffect_local_errors_attr(
                    &f.user_attributes,
                ),
                ..*c
            },
            self,
        )
    }

    fn visit_expr(&mut self, c: &mut Context, p: &aast::Expr<(), ()>) -> Result<(), ()> {
        use aast::{ClassId, ClassId_::*, Expr, Expr_::*, Lid};

        if c.in_methodish {
            self.check_pure_fn_contexts(c, &p);
        }
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
        }
        p.recurse(c, self)
    }
}

pub fn check_program(program: &aast::Program<(), ()>) -> Vec<SyntaxError> {
    let mut checker = Checker::new();
    let mut context = Context {
        in_methodish: false,
        in_classish: false,
        in_static_methodish: false,
        is_pure: false,
        is_constructor: false,
        ignore_coeffect_local_errors: false,
    };
    visit(&mut checker, &mut context, program).unwrap();
    checker.errors
}
