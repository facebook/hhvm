// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::Allocator;
use crate::reason::Reason;
use pos::{
    ClassConstName, FilePos, MethodName, ModuleName, Pos, Positioned, PropName, Symbol,
    TypeConstName, TypeName,
};

impl<R: Reason> Allocator<R> {
    pub fn pos_from_ast(&self, pos: &oxidized::pos::Pos) -> R::Pos {
        R::Pos::mk(|| {
            let pos_file = self.relative_path_from_ast(pos.filename());
            let ((start_lnum, start_bol, start_cnum), (end_lnum, end_bol, end_cnum)) =
                pos.to_start_and_end_lnum_bol_offset();
            let pos_start = FilePos {
                lnum: start_lnum as u64,
                bol: start_bol as u64,
                cnum: start_cnum as u64,
            };
            let pos_end = FilePos {
                lnum: end_lnum as u64,
                bol: end_bol as u64,
                cnum: end_cnum as u64,
            };
            (pos_file, pos_start, pos_end)
        })
    }

    pub fn pos_from_decl(&self, pos: &oxidized_by_ref::pos::Pos<'_>) -> R::Pos {
        R::Pos::mk(|| {
            let pos_file = self.relative_path_from_decl(pos.filename());
            let ((start_lnum, start_bol, start_cnum), (end_lnum, end_bol, end_cnum)) =
                pos.to_start_and_end_lnum_bol_offset();
            let pos_start = FilePos {
                lnum: start_lnum as u64,
                bol: start_bol as u64,
                cnum: start_cnum as u64,
            };
            let pos_end = FilePos {
                lnum: end_lnum as u64,
                bol: end_bol as u64,
                cnum: end_cnum as u64,
            };
            (pos_file, pos_start, pos_end)
        })
    }

    pub fn pos_classname_from_ast(
        &self,
        pos_id: &oxidized::ast_defs::Id,
    ) -> Positioned<TypeName, R::Pos> {
        Positioned::new(
            self.pos_from_ast(&pos_id.0),
            TypeName(self.symbol(&pos_id.1)),
        )
    }

    pub fn pos_classname_from_decl(
        &self,
        pos_id: oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> Positioned<TypeName, R::Pos> {
        Positioned::new(
            self.pos_from_decl(pos_id.0),
            TypeName(self.symbol(pos_id.1)),
        )
    }

    pub fn pos_id_from_decl(
        &self,
        pos_id: oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> Positioned<Symbol, R::Pos> {
        Positioned::new(self.pos_from_decl(pos_id.0), self.symbol(pos_id.1))
    }

    pub fn pos_module_from_ast_ref(
        &self,
        oxidized_by_ref::ast_defs::Id(pos, id): &oxidized_by_ref::ast_defs::Id<'_>,
    ) -> Positioned<ModuleName, R::Pos> {
        Positioned::new(self.pos_from_decl(pos), ModuleName(self.symbol(id)))
    }

    pub fn pos_class_const_from_decl(
        &self,
        (pos, id): oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> Positioned<ClassConstName, R::Pos> {
        Positioned::new(self.pos_from_decl(pos), ClassConstName(self.symbol(id)))
    }

    pub fn pos_type_const_from_decl(
        &self,
        (pos, id): oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> Positioned<TypeConstName, R::Pos> {
        Positioned::new(self.pos_from_decl(pos), TypeConstName(self.symbol(id)))
    }

    pub fn pos_method_from_decl(
        &self,
        (pos, id): oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> Positioned<MethodName, R::Pos> {
        Positioned::new(self.pos_from_decl(pos), MethodName(self.symbol(id)))
    }

    pub fn pos_prop_from_decl(
        &self,
        (pos, id): oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> Positioned<PropName, R::Pos> {
        Positioned::new(self.pos_from_decl(pos), PropName(self.symbol(id)))
    }
}
