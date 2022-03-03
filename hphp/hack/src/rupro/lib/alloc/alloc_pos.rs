// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::Allocator;
use crate::reason::Reason;
use oxidized::pos_span_raw::PosSpanRaw;
use pos::{
    ClassConstName, MethodName, ModuleName, Pos, Positioned, PropName, RelativePath, Symbol,
    TypeConstName, TypeName,
};

impl<R: Reason> Allocator<R> {
    pub fn pos_from_ast(&self, pos: &oxidized::pos::Pos) -> R::Pos {
        R::Pos::mk(|| {
            let PosSpanRaw { start, end } = pos.to_raw_span();
            (RelativePath::from(pos.filename()), start, end)
        })
    }

    pub fn pos_from_decl(&self, pos: &oxidized_by_ref::pos::Pos<'_>) -> R::Pos {
        R::Pos::mk(|| {
            let PosSpanRaw { start, end } = pos.to_raw_span();
            (RelativePath::from(pos.filename()), start, end)
        })
    }

    pub fn pos_type_from_ast(
        &self,
        pos_id: &oxidized::ast_defs::Id,
    ) -> Positioned<TypeName, R::Pos> {
        Positioned::new(
            self.pos_from_ast(&pos_id.0),
            TypeName(Symbol::new(&pos_id.1)),
        )
    }

    pub fn pos_type_from_decl(
        &self,
        pos_id: oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> Positioned<TypeName, R::Pos> {
        Positioned::new(
            self.pos_from_decl(pos_id.0),
            TypeName(Symbol::new(pos_id.1)),
        )
    }

    pub fn pos_module_from_ast_ref(
        &self,
        oxidized_by_ref::ast_defs::Id(pos, id): &oxidized_by_ref::ast_defs::Id<'_>,
    ) -> Positioned<ModuleName, R::Pos> {
        Positioned::new(self.pos_from_decl(pos), ModuleName(Symbol::new(*id)))
    }

    pub fn pos_class_const_from_decl(
        &self,
        (pos, id): oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> Positioned<ClassConstName, R::Pos> {
        Positioned::new(self.pos_from_decl(pos), ClassConstName(Symbol::new(id)))
    }

    pub fn pos_type_const_from_decl(
        &self,
        (pos, id): oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> Positioned<TypeConstName, R::Pos> {
        Positioned::new(self.pos_from_decl(pos), TypeConstName(Symbol::new(id)))
    }

    pub fn pos_method_from_decl(
        &self,
        (pos, id): oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> Positioned<MethodName, R::Pos> {
        Positioned::new(self.pos_from_decl(pos), MethodName(Symbol::new(id)))
    }

    pub fn pos_prop_from_decl(
        &self,
        (pos, id): oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> Positioned<PropName, R::Pos> {
        Positioned::new(self.pos_from_decl(pos), PropName(Symbol::new(id)))
    }
}
