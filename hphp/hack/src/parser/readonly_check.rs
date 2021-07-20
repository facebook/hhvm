// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use naming_special_names_rust::special_idents;
use oxidized::{
    aast,
    aast_visitor::{visit, AstParams, Node, Visitor},
    ast::*,
    local_id,
    pos::Pos,
};
use parser_core_types::{
    syntax_error,
    syntax_error::{Error as ErrorMsg, SyntaxError},
};
use std::collections::HashMap;

#[derive(PartialEq)]
pub enum Rty {
    Readonly,
    Mutable,
}

struct Context {
    locals: HashMap<String, bool>,
    readonly_return: Rty,
}

impl Context {
    fn new(readonly_ret: Rty) -> Self {
        Self {
            locals: HashMap::new(),
            readonly_return: readonly_ret,
        }
    }

    pub fn add_local(&mut self, var_name: &String, is_readonly: bool) {
        match self.locals.get(var_name) {
            Some(_) => {
                // per @jjwu, "...once a variable is assigned a value, it
                // can only be assigned a value of the same Rty
                // for the rest of the function." See D29320968 for more context.
            }
            None => {
                self.locals.insert(var_name.clone(), is_readonly);
            }
        }
    }

    pub fn is_new_local(&self, var_name: &String) -> bool {
        !self.locals.contains_key(var_name)
    }

    pub fn is_readonly<S: ?Sized>(&self, var_name: &S) -> bool
    where
        String: std::borrow::Borrow<S>,
        S: std::hash::Hash + Eq,
    {
        match self.locals.get(var_name) {
            Some(&x) => x,
            None => false,
        }
    }
}

fn ro_expr_list(context: &mut Context, exprs: &Vec<Expr>) -> Rty {
    if exprs.iter().any(|e| rty_expr(context, &e) == Rty::Readonly) {
        Rty::Readonly
    } else {
        Rty::Mutable
    }
}

fn ro_expr_list2<T>(context: &mut Context, exprs: &Vec<(T, Expr)>) -> Rty {
    if exprs
        .iter()
        .any(|e| rty_expr(context, &e.1) == Rty::Readonly)
    {
        Rty::Readonly
    } else {
        Rty::Mutable
    }
}

fn rty_expr(context: &mut Context, expr: &Expr) -> Rty {
    let aast::Expr(_, _, exp) = &*expr;
    match exp {
        aast::Expr_::ReadonlyExpr(_) => Rty::Readonly,
        aast::Expr_::ObjGet(og) => {
            let (obj, _member_name, _null_flavor, _reffiness) = &**og;
            rty_expr(context, &obj)
        }
        aast::Expr_::Lvar(id_orig) => {
            let var_name = local_id::get_name(&id_orig.1);
            // TODO(alnash) we need to handle $this separately because the Rty
            // comes from the function for this and not by the name
            let is_this = var_name == special_idents::THIS;
            if !is_this && context.is_readonly(var_name) {
                Rty::Readonly
            } else {
                Rty::Mutable
            }
        }
        aast::Expr_::Darray(d) => {
            let (_, exprs) = &**d;
            ro_expr_list2(context, exprs)
        }
        aast::Expr_::Varray(v) => {
            let (_, exprs) = &**v;
            ro_expr_list(context, exprs)
        }
        aast::Expr_::Shape(fields) => ro_expr_list2(context, fields),
        aast::Expr_::ValCollection(v) => {
            let (_, _, exprs) = &**v;
            ro_expr_list(context, exprs)
        }
        aast::Expr_::KeyValCollection(kv) => {
            let (_, _, fields) = &**kv;
            if fields
                .iter()
                .any(|f| rty_expr(context, &f.1) == Rty::Readonly)
            {
                Rty::Readonly
            } else {
                Rty::Mutable
            }
        }
        aast::Expr_::Collection(c) => {
            let (_, _, fields) = &**c;
            if fields.iter().any(|f| match f {
                aast::Afield::AFvalue(e) => rty_expr(context, &e) == Rty::Readonly,
                aast::Afield::AFkvalue(_, e) => rty_expr(context, &e) == Rty::Readonly,
            }) {
                Rty::Readonly
            } else {
                Rty::Mutable
            }
        }
        _ => Rty::Mutable,
    }
}

fn check_assignment_validity(
    context: &mut Context,
    checker: &mut Checker,
    pos: &Pos,
    lhs: &Expr,
    rhs: &Expr,
) {
    match &lhs.2 {
        aast::Expr_::Lvar(id_orig) => {
            let var_name = local_id::get_name(&id_orig.1).to_string();
            // TODO(alnash) we need to handle $this separately because the Rty
            // comes from the function for this and not by the name
            if var_name != special_idents::THIS {
                let is_readonly = Rty::Readonly == rty_expr(context, &rhs);
                if context.is_new_local(&var_name) {
                    context.add_local(&var_name, is_readonly);
                } else if context.is_readonly(&var_name) != is_readonly {
                    checker.add_error(
                        &pos,
                        syntax_error::redefined_assignment_different_mutability(&var_name),
                    );
                }
            }
        }
        aast::Expr_::ObjGet(_) => match rty_expr(context, &lhs) {
            Rty::Readonly => {
                checker.add_error(&pos, syntax_error::assignment_to_readonly);
            }
            _ => {}
        },
        _ => {}
    }
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

    fn subtype(&mut self, pos: &Pos, r_sub: &Rty, r_sup: &Rty, reason: &str) {
        use Rty::*;
        match (r_sub, r_sup) {
            (Readonly, Mutable) => self.add_error(
                &pos,
                syntax_error::invalid_readonly("readonly", "mutable", &reason),
            ),
            _ => {}
        }
    }
}

impl<'ast> Visitor<'ast> for Checker {
    type P = AstParams<Context, ()>;

    fn object(&mut self) -> &mut dyn Visitor<'ast, P = Self::P> {
        self
    }

    fn visit_method_(
        &mut self,
        _context: &mut Context,
        m: &aast::Method_<(), (), (), ()>,
    ) -> Result<(), ()> {
        let readonly_return = if let Some(_) = m.readonly_ret {
            Rty::Readonly
        } else {
            Rty::Mutable
        };
        m.recurse(&mut Context::new(readonly_return), self)
    }

    fn visit_fun_(
        &mut self,
        _context: &mut Context,
        f: &aast::Fun_<(), (), (), ()>,
    ) -> Result<(), ()> {
        let readonly_return = if let Some(_) = f.readonly_ret {
            Rty::Readonly
        } else {
            Rty::Mutable
        };
        f.recurse(&mut Context::new(readonly_return), self)
    }

    fn visit_expr(
        &mut self,
        context: &mut Context,
        p: &aast::Expr<(), (), (), ()>,
    ) -> Result<(), ()> {
        if let aast::Expr_::Binop(x) = &p.2 {
            x.recurse(context, self.object())?;
            let (bop, e_lhs, e_rhs) = x.as_ref();
            if let Bop::Eq(None) = bop {
                check_assignment_validity(context, self, &p.1, e_lhs, e_rhs);
            }
        }

        p.recurse(context, self)
    }

    fn visit_stmt(
        &mut self,
        context: &mut Context,
        s: &aast::Stmt<(), (), (), ()>,
    ) -> std::result::Result<(), ()> {
        if let aast::Stmt_::Return(r) = &s.1 {
            r.recurse(context, self.object())?;
            if let Some(expr) = r.as_ref() {
                self.subtype(&expr.1, &rty_expr(context, &expr), &context.readonly_return, "this function does not return readonly. Please mark it to return readonly if needed.")
            }
        }
        s.recurse(context, self)
    }
}

pub fn check_program(program: &aast::Program<(), (), (), ()>) -> Vec<SyntaxError> {
    let mut checker = Checker::new();
    let mut context = Context::new(Rty::Mutable);
    visit(&mut checker, &mut context, program).unwrap();
    checker.errors
}
