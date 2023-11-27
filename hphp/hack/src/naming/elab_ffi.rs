// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::ast;
use oxidized::naming_phase_error::NamingPhaseError;
use oxidized::nast;
use oxidized::typechecker_options::TypecheckerOptions;
use relative_path::RelativePath;

ocamlrep_ocamlpool::ocaml_ffi! {
    fn hh_elab_program(
        tco: TypecheckerOptions,
        path: RelativePath,
        program: ast::Program,
    ) -> (nast::Program, Vec<NamingPhaseError>) {
        let mut program = program;
        let errs = elab::elaborate_program(&tco, &path, &mut program);
        (program, errs)
    }

    fn hh_elab_fun_def(
        tco: TypecheckerOptions,
        path: RelativePath,
        fd: ast::FunDef,
    ) -> (nast::FunDef, Vec<NamingPhaseError>) {
        let mut fd = fd;
        let errs = elab::elaborate_fun_def(&tco, &path, &mut fd);
        (fd, errs)
    }

    fn hh_elab_class_(
        tco: TypecheckerOptions,
        path: RelativePath,
        c: ast::Class_,
    ) -> (nast::Class_, Vec<NamingPhaseError>) {
        let mut c = c;
        let errs = elab::elaborate_class_(&tco, &path, &mut c);
        (c, errs)
    }

    fn hh_elab_module_def(
        tco: TypecheckerOptions,
        path: RelativePath,
        m: ast::ModuleDef,
    ) -> (nast::ModuleDef, Vec<NamingPhaseError>) {
        let mut m = m;
        let errs = elab::elaborate_module_def(&tco, &path, &mut m);
        (m, errs)
    }

    fn hh_elab_gconst(
        tco: TypecheckerOptions,
        path: RelativePath,
        cst: ast::Gconst,
    ) -> (nast::Gconst, Vec<NamingPhaseError>) {
        let mut cst = cst;
        let errs = elab::elaborate_gconst(&tco, &path, &mut cst);
        (cst, errs)
    }

    fn hh_elab_typedef(
        tco: TypecheckerOptions,
        path: RelativePath,
        td: ast::Typedef,
    ) -> (nast::Typedef, Vec<NamingPhaseError>) {
        let mut td = td;
        let errs = elab::elaborate_typedef(&tco, &path, &mut td);
        (td, errs)
    }

    fn hh_elab_stmt(
        tco: TypecheckerOptions,
        path: RelativePath,
        stmt: ast::Stmt,
    ) -> (nast::Stmt, Vec<NamingPhaseError>) {
        let mut stmt = stmt;
        let errs = elab::elaborate_stmt(&tco, &path, &mut stmt);
        (stmt, errs)
    }
}
