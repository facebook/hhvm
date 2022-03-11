// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use crate::reason::Reason;
use pos::TypeName;
use serde::{Deserialize, Serialize};

#[derive(Clone, Debug, Eq, Hash, PartialEq, Serialize, Deserialize)]
#[serde(bound = "R: Reason")]
pub enum Primary<R: Reason> {
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
}
