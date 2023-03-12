// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::rc::RcOc;
use oxidized::ast;
use oxidized::namespace_env::Env;
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::nast;
use oxidized::typechecker_options::TypecheckerOptions;
use relative_path::RelativePath;

fn env() -> RcOc<Env> {
    let is_codegen = false;
    let disable_xhp_element_mangling = false;
    RcOc::new(Env::empty(vec![], is_codegen, disable_xhp_element_mangling))
}

ocamlrep_ocamlpool::ocaml_ffi! {
    fn hh_elab_ns_program(program: ast::Program) -> nast::Program {
        let mut program = program;
        elaborate_namespaces_visitor::elaborate_program(env(), &mut program);
        program
    }

    fn hh_elab_ns_fun_def(fd: ast::FunDef) -> nast::FunDef {
        let mut fd = fd;
        elaborate_namespaces_visitor::elaborate_fun_def(env(), &mut fd);
        fd
    }

    fn hh_elab_ns_class_(c: ast::Class_) -> nast::Class_ {
        let mut c = c;
        elaborate_namespaces_visitor::elaborate_class_(env(), &mut c);
        c
    }

    fn hh_elab_ns_module_def(m: ast::ModuleDef) -> nast::ModuleDef {
        let mut m = m;
        elaborate_namespaces_visitor::elaborate_module_def(env(), &mut m);
        m
    }

    fn hh_elab_ns_gconst(cst: ast::Gconst) -> nast::Gconst {
        let mut cst = cst;
        elaborate_namespaces_visitor::elaborate_gconst(env(), &mut cst);
        cst
    }

    fn hh_elab_ns_typedef(td: ast::Typedef) -> nast::Typedef {
        let mut td = td;
        elaborate_namespaces_visitor::elaborate_typedef(env(), &mut td);
        td
    }

    fn hh_elab_program(
        tco: TypecheckerOptions,
        path: RelativePath,
        program: nast::Program,
    ) -> (nast::Program, Vec<NamingPhaseError>) {
        let mut program = program;
        let errs = elab::elaborate_program(&tco, &path, &mut program);
        (program, errs)
    }

    fn hh_elab_fun_def(
        tco: TypecheckerOptions,
        path: RelativePath,
        fd: nast::FunDef,
    ) -> (nast::FunDef, Vec<NamingPhaseError>) {
        let mut fd = fd;
        let errs = elab::elaborate_fun_def(&tco, &path, &mut fd);
        (fd, errs)
    }

    fn hh_elab_class_(
        tco: TypecheckerOptions,
        path: RelativePath,
        c: nast::Class_,
    ) -> (nast::Class_, Vec<NamingPhaseError>) {
        let mut c = c;
        let errs = elab::elaborate_class_(&tco, &path, &mut c);
        (c, errs)
    }

    fn hh_elab_module_def(
        tco: TypecheckerOptions,
        path: RelativePath,
        m: nast::ModuleDef,
    ) -> (nast::ModuleDef, Vec<NamingPhaseError>) {
        let mut m = m;
        let errs = elab::elaborate_module_def(&tco, &path, &mut m);
        (m, errs)
    }

    fn hh_elab_gconst(
        tco: TypecheckerOptions,
        path: RelativePath,
        cst: nast::Gconst,
    ) -> (nast::Gconst, Vec<NamingPhaseError>) {
        let mut cst = cst;
        let errs = elab::elaborate_gconst(&tco, &path, &mut cst);
        (cst, errs)
    }

    fn hh_elab_typedef(
        tco: TypecheckerOptions,
        path: RelativePath,
        td: nast::Typedef,
    ) -> (nast::Typedef, Vec<NamingPhaseError>) {
        let mut td = td;
        let errs = elab::elaborate_typedef(&tco, &path, &mut td);
        (td, errs)
    }
}
