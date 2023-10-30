// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use eq_modulo_pos::EqModuloPos;
use pos::TypeName;
use serde::Deserialize;
use serde::Serialize;

use crate::reason::Reason;

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
#[serde(bound = "R: Reason")]
pub enum Primary<R: Reason> {
    Subtype,
    OccursCheck,
    InvalidTypeHint(R::Pos),
    ExpectingTypeHint(R::Pos),
    ExpectingReturnTypeHint(R::Pos),
    CyclicClassDef(R::Pos, Vec<TypeName>),
    WrongExtendKind {
        parent_pos: R::Pos,
        parent_kind: oxidized::ast_defs::ClassishKind,
        parent_name: TypeName,
        pos: R::Pos,
        kind: oxidized::ast_defs::ClassishKind,
        name: TypeName,
    },
    WrongUseKind {
        parent_pos: R::Pos,
        parent_name: TypeName,
        pos: R::Pos,
        name: TypeName,
    },
}
