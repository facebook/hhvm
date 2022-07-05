// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use eq_modulo_pos::EqModuloPos;
use pos::TypeName;
use serde::Deserialize;
use serde::Serialize;

#[derive(Clone, Debug, Eq, EqModuloPos, Hash, PartialEq, Serialize, Deserialize)]
pub enum DeclError<P> {
    CyclicClassDef(P, Vec<TypeName>),
    WrongExtendKind {
        parent_pos: P,
        parent_kind: oxidized::ast_defs::ClassishKind,
        parent_name: TypeName,
        pos: P,
        kind: oxidized::ast_defs::ClassishKind,
        name: TypeName,
    },
}
