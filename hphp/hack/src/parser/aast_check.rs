// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust::special_idents;
use oxidized::{
    aast,
    aast_visitor::{visit, AstParams, Node, Visitor},
    pos::Pos,
};
use parser_core_types::{
    syntax_error,
    syntax_error::{Error as ErrorMsg, SyntaxError},
};

struct Context {
    in_methodish: bool,
    in_classish: bool,
    in_static_methodish: bool,
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
}

impl Visitor for Checker {
    type P = AstParams<Context, ()>;

    fn object(&mut self) -> &mut dyn Visitor<P = Self::P> {
        self
    }

    fn visit_class_(
        &mut self,
        c: &mut Context,
        p: &aast::Class_<Pos, (), (), ()>,
    ) -> Result<(), ()> {
        p.recurse(
            &mut Context {
                in_classish: true,
                ..*c
            },
            self,
        )
    }

    fn visit_method_(
        &mut self,
        c: &mut Context,
        p: &aast::Method_<Pos, (), (), ()>,
    ) -> Result<(), ()> {
        p.recurse(
            &mut Context {
                in_methodish: true,
                in_static_methodish: p.static_,
                ..*c
            },
            self,
        )
    }

    fn visit_fun_(&mut self, c: &mut Context, p: &aast::Fun_<Pos, (), (), ()>) -> Result<(), ()> {
        p.recurse(
            &mut Context {
                in_methodish: true,
                in_static_methodish: p.static_ || c.in_static_methodish,
                ..*c
            },
            self,
        )
    }

    fn visit_expr(&mut self, c: &mut Context, p: &aast::Expr<Pos, (), (), ()>) -> Result<(), ()> {
        use aast::{ClassId, ClassId_::*, Expr, Expr_::*, Lid};

        if let Await(_) = p.1 {
            if !c.in_methodish {
                self.add_error(&p.0, syntax_error::toplevel_await_use)
            }
        } else if let Some((_, Expr(_, f), ..)) = p.1.as_call() {
            if let Some((ClassId(_, CIexpr(Expr(pos, Id(id)))), ..)) = f.as_class_const() {
                if Self::name_eq_this_and_in_static_method(c, &id.1) {
                    self.add_error(&pos, syntax_error::this_in_static);
                }
            }
        } else if let Some(Lid(pos, (_, name))) = p.1.as_lvar() {
            if Self::name_eq_this_and_in_static_method(c, name) {
                self.add_error(pos, syntax_error::this_in_static);
            }
        }
        p.recurse(c, self)
    }
}

pub fn check_program(program: &aast::Program<Pos, (), (), ()>) -> Vec<SyntaxError> {
    let mut checker = Checker::new();
    let mut context = Context {
        in_methodish: false,
        in_classish: false,
        in_static_methodish: false,
    };
    visit(&mut checker, &mut context, program).unwrap();
    checker.errors
}
