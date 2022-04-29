// Copyright (c) 2022, Meta, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::{
    aast,
    aast_visitor::{visit, AstParams, Node, Visitor},
    ast_defs,
    pos::Pos,
};
use parser_core_types::{
    syntax_error,
    syntax_error::{Error as ErrorMsg, SyntaxError},
};

struct Context {
    in_module: bool,
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

    fn check_in_module(&mut self, pos: &Pos, c: &Context) {
        if !c.in_module {
            self.add_error(pos, syntax_error::internal_outside_of_module)
        }
    }
}

impl<'ast> Visitor<'ast> for Checker {
    type Params = AstParams<Context, ()>;
    fn object(&mut self) -> &mut dyn Visitor<'ast, Params = Self::Params> {
        self
    }

    fn visit_class_(&mut self, c: &mut Context, p: &aast::Class_<(), ()>) -> Result<(), ()> {
        c.in_module = p.module.is_some();
        if p.internal {
            self.check_in_module(&p.name.0, c)
        }
        p.recurse(c, self.object())
    }

    fn visit_method_(&mut self, c: &mut Context, m: &aast::Method_<(), ()>) -> Result<(), ()> {
        if m.visibility == ast_defs::Visibility::Internal {
            self.check_in_module(&m.name.0, c)
        }
        m.recurse(c, self.object())
    }

    fn visit_fun_def(&mut self, c: &mut Context, f: &aast::FunDef<(), ()>) -> Result<(), ()> {
        c.in_module = f.module.is_some();
        if f.internal {
            self.check_in_module(&f.fun.name.0, c);
        }
        f.recurse(c, self.object())
    }

    fn visit_typedef(&mut self, c: &mut Context, t: &aast::Typedef<(), ()>) -> Result<(), ()> {
        c.in_module = t.module.is_some();
        if t.internal {
            self.check_in_module(&t.name.0, c);
        }
        t.recurse(c, self.object())
    }
}

/*
    Module membership declarations must be before any real declaration in the file
    You can have use namespace or file attributes in any order before it, but
    actual declarations must follow the module membership decl.

    This check also prevents having two module declarations in the file.
*/
fn check_module_declaration_first(checker: &mut Checker, program: &aast::Program<(), ()>) {
    let mut past_first_def = false;
    for def in program {
        match def {
            aast::Def::Stmt(_) /* Stmt is only used for Markup */
            | aast::Def::FileAttributes(_)
            /* We allow module declarations before module membership because they aren't affected
            by the module of a file, and it makes unit testing easier to write.
            In practice, we'll probably have different conventions/lint about these declarations.*/
            | aast::Def::Module(_)
            => {}
            aast::Def::SetModule(m) => {
                if past_first_def {
                    let oxidized::ast::Id(pos, name) = &**m;
                    checker.add_error(pos, syntax_error::module_first_in_file(name));
                }
                past_first_def = true
            }
            aast::Def::Fun(_)
            | aast::Def::Class(_)
            | aast::Def::Typedef(_)
            | aast::Def::Namespace(_)
            | aast::Def::SetNamespaceEnv(_)
            | aast::Def::NamespaceUse(_)
            | aast::Def::Constant(_)
           => past_first_def = true,
        }
    }
}

pub fn check_program(program: &aast::Program<(), ()>) -> Vec<SyntaxError> {
    let mut checker = Checker::new();
    let mut context = Context { in_module: false };
    check_module_declaration_first(&mut checker, program);
    visit(&mut checker, &mut context, program).unwrap();
    checker.errors
}
