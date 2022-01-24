// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::hcons::Conser;
use crate::pos::{FilePos, Pos, PosId, Prefix, RelativePath, Symbol};
use crate::reason::{Reason, ReasonImpl};

#[derive(Debug)]
pub struct PosProvider {
    symbols: Conser<String>,
}

impl PosProvider {
    pub fn new() -> Self {
        Self {
            symbols: Conser::new(),
        }
    }

    pub fn mk_symbol(&self, symbol: &str) -> Symbol {
        Symbol::from(self.symbols.mk(symbol))
    }


    pub fn mk_relative_path(
        &self,
        relative_path: &oxidized::relative_path::RelativePath,
    ) -> RelativePath {
        let prefix = Prefix::from(relative_path.prefix());
        let suffix = self.mk_symbol(relative_path.path_str());
        RelativePath::new(prefix, suffix)
    }

    pub fn mk_relative_path_of_ref(
        &self,
        relative_path: &oxidized_by_ref::relative_path::RelativePath<'_>,
    ) -> RelativePath {
        let prefix = Prefix::from(relative_path.prefix());
        let suffix = relative_path
            .path_str()
            .map(|path| self.mk_symbol(path))
            .unwrap_or_else(|| self.mk_symbol(&""));
        RelativePath::new(prefix, suffix)
    }


    pub fn mk_pos<R: Reason>(&self, pos: &oxidized::pos::Pos) -> R::Pos {
        R::Pos::mk(&|| {
            let pos_file = self.mk_relative_path(pos.filename());
            let ((start_lnum, start_bol, start_cnum), (end_lnum, end_bol, end_cnum)) =
                pos.to_start_and_end_lnum_bol_offset();
            let pos_start = FilePos {
                pos_lnum: start_lnum as u64,
                pos_bol: start_bol as u64,
                pos_cnum: start_cnum as u64,
            };
            let pos_end = FilePos {
                pos_lnum: end_lnum as u64,
                pos_bol: end_bol as u64,
                pos_cnum: end_cnum as u64,
            };
            (pos_file, pos_start, pos_end)
        })
    }

    pub fn mk_pos_of_ref<R: Reason>(&self, pos: &oxidized_by_ref::pos::Pos<'_>) -> R::Pos {
        R::Pos::mk(&|| {
            let pos_file = self.mk_relative_path_of_ref(pos.filename());
            let ((start_lnum, start_bol, start_cnum), (end_lnum, end_bol, end_cnum)) =
                pos.to_start_and_end_lnum_bol_offset();
            let pos_start = FilePos {
                pos_lnum: start_lnum as u64,
                pos_bol: start_bol as u64,
                pos_cnum: start_cnum as u64,
            };
            let pos_end = FilePos {
                pos_lnum: end_lnum as u64,
                pos_bol: end_bol as u64,
                pos_cnum: end_cnum as u64,
            };
            (pos_file, pos_start, pos_end)
        })
    }

    pub fn mk_pos_id<R: Reason>(&self, pos_id: &oxidized::ast_defs::Id) -> PosId<R::Pos> {
        let pos = self.mk_pos::<R>(&pos_id.0);
        let def = self.mk_symbol(&pos_id.1);
        PosId::new(pos, def)
    }

    pub fn mk_pos_id_of_ref<R: Reason>(
        &self,
        pos_id: oxidized_by_ref::typing_defs::PosId<'_>,
    ) -> PosId<R::Pos> {
        let pos = self.mk_pos_of_ref::<R>(pos_id.0);
        let def = self.mk_symbol(pos_id.1);
        PosId::new(pos, def)
    }

    pub fn mk_reason<R: Reason>(&self, reason: &oxidized_by_ref::typing_reason::T_<'_>) -> R {
        R::mk(&|| {
            use oxidized_by_ref::typing_reason::T_ as OR;
            use ReasonImpl as RI;
            match reason {
                OR::Rnone => RI::Rnone,
                OR::Rwitness(pos) => RI::Rwitness(self.mk_pos_of_ref::<R>(pos)),
                OR::RwitnessFromDecl(pos) => RI::RwitnessFromDecl(self.mk_pos_of_ref::<R>(pos)),
                OR::Rhint(pos) => RI::Rhint(self.mk_pos_of_ref::<R>(pos)),
                r => unimplemented!("mk_reason: {:?}", r),
            }
        })
    }
}
