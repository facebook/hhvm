// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::rc::RcOc;
use oxidized::ast;
use oxidized::namespace_env::Env;
use oxidized::nast;

fn env() -> RcOc<Env> {
    let is_codegen = false;
    let disable_xhp_element_mangling = false;
    RcOc::new(Env::empty(vec![], is_codegen, disable_xhp_element_mangling))
}

ocamlrep_ocamlpool::ocaml_ffi! {
    fn hh_elaborate_program(program: ast::Program) -> nast::Program {
        let mut program = program;
        elaborate_namespaces_visitor::elaborate_program(env(), &mut program);
        program
    }

    fn hh_elaborate_fun_def(fd: ast::FunDef) -> nast::FunDef {
        let mut fd = fd;
        elaborate_namespaces_visitor::elaborate_fun_def(env(), &mut fd);
        fd
    }

    fn hh_elaborate_class_(c: ast::Class_) -> nast::Class_ {
        let mut c = c;
        elaborate_namespaces_visitor::elaborate_class_(env(), &mut c);
        c
    }

    fn hh_elaborate_module_def(m: ast::ModuleDef) -> nast::ModuleDef {
        let mut m = m;
        elaborate_namespaces_visitor::elaborate_module_def(env(), &mut m);
        m
    }

    fn hh_elaborate_gconst(cst: ast::Gconst) -> nast::Gconst {
        let mut cst = cst;
        elaborate_namespaces_visitor::elaborate_gconst(env(), &mut cst);
        cst
    }

    fn hh_elaborate_typedef(td: ast::Typedef) -> nast::Typedef {
        let mut td = td;
        elaborate_namespaces_visitor::elaborate_typedef(env(), &mut td);
        td
    }
}
