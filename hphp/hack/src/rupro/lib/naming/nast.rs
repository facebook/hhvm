// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::naming::{FileId, FileInfo};

pub struct Nast;

impl Nast {
    fn get_defs_impl(ast: &oxidized::aast::Program<(), ()>, fi: &mut FileInfo) {
        use oxidized::aast::Def;
        fn of_id(id: &oxidized::ast_defs::Id) -> FileId {
            FileId::with_full_pos(id.0.clone(), id.1.clone(), None)
        }
        for def in ast {
            match def {
                Def::Fun(f) => fi.funs.push(of_id(&f.fun.name)),
                Def::Class(c) => fi.classes.push(of_id(&c.name)),
                Def::Typedef(t) => fi.typedefs.push(of_id(&t.name)),
                Def::Constant(cst) => fi.consts.push(of_id(&cst.name)),
                Def::Namespace(_) => Self::get_defs_impl(ast, fi),
                Def::Stmt(_)
                | Def::Module(_)
                | Def::NamespaceUse(_)
                | Def::SetNamespaceEnv(_)
                | Def::FileAttributes(_) => {}
            }
        }
    }

    pub fn get_defs(ast: &oxidized::aast::Program<(), ()>) -> FileInfo {
        let mut fi = FileInfo::empty();
        Self::get_defs_impl(ast, &mut fi);
        fi
    }
}
