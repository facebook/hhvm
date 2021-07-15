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
pub enum VariableKind {
    Readonly,
    Mutable,
}

struct Context {
    locals: HashMap<String, bool>,
}

impl Context {
    fn new() -> Self {
        Self {
            locals: HashMap::new(),
        }
    }

    pub fn add_local(&mut self, var_name: &String, is_readonly: bool) {
        match self.locals.get(var_name) {
            Some(_) => {
                // per @jjwu, "...once a variable is assigned a value, it
                // can only be assigned a value of the same readonlyness
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

fn get_readonlyness(context: &mut Context, expr: &Expr_) -> VariableKind {
    match expr {
        aast::Expr_::ReadonlyExpr(_) => VariableKind::Readonly,
        aast::Expr_::ObjGet(og) => {
            let (obj, _member_name, _null_flavor, _reffiness) = &**og;
            get_readonlyness(context, &obj.2)
        }
        aast::Expr_::Lvar(id_orig) => {
            let var_name = local_id::get_name(&id_orig.1);
            // TODO(alnash) we need to handle $this separately because the readonlyness
            // comes from the function for this and not by the name
            let is_this = var_name == special_idents::THIS;
            if !is_this && context.is_readonly(var_name) {
                VariableKind::Readonly
            } else {
                VariableKind::Mutable
            }
        }
        _ => VariableKind::Mutable,
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
            // TODO(alnash) we need to handle $this separately because the readonlyness
            // comes from the function for this and not by the name
            if var_name != special_idents::THIS {
                let is_readonly = VariableKind::Readonly == get_readonlyness(context, &rhs.2);
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
        aast::Expr_::ObjGet(_) => match get_readonlyness(context, &lhs.2) {
            VariableKind::Readonly => {
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
        m.recurse(&mut Context::new(), self)
    }

    fn visit_fun_(
        &mut self,
        _context: &mut Context,
        f: &aast::Fun_<(), (), (), ()>,
    ) -> Result<(), ()> {
        f.recurse(&mut Context::new(), self)
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
}

pub fn check_program(program: &aast::Program<(), (), (), ()>) -> Vec<SyntaxError> {
    let mut checker = Checker::new();
    let mut context = Context::new();
    visit(&mut checker, &mut context, program).unwrap();
    checker.errors
}
