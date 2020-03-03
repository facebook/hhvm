// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::typing_make_type::*;
use oxidized::{
    ast_defs::FunKind, relative_path::RelativePath, s_map, typing_defs::ValKind,
    typing_env_types::Genv,
};

pub fn empty_global_env(builder: &TypeBuilder, file: RelativePath) -> Genv {
    Genv {
        file,
        tcopt: oxidized::global_options::GlobalOptions::default(),
        fun_mutable: None,
        params: oxidized::local_id::map::Map::new(),
        return_: oxidized::typing_env_return_info::TypingEnvReturnInfo {
            explicit: false,
            mutable: false,
            void_to_rx: false,
            disposable: false,
            type_: oxidized::typing_defs::PossiblyEnforcedTy {
                enforced: false,
                type_: builder.nothing(oxidized::typing_reason::Reason::Rnone),
            },
        },
        static_: false,
        self_: None,
        parent: None,
        fun_kind: FunKind::FSync,
        val_kind: ValKind::Other,
        condition_types: s_map::SMap::new(),
    }
}
