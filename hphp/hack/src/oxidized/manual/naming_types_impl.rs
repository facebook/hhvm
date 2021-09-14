// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::convert::TryFrom;

use crate::gen::naming_types::{KindOfType, NameKind};

impl TryFrom<i64> for NameKind {
    type Error = String;

    fn try_from(kind: i64) -> Result<Self, String> {
        // Must be kept in sync with naming_types.ml and namekind_to_i64
        match kind {
            0 => Ok(NameKind::TypeKind(KindOfType::TClass)),
            1 => Ok(NameKind::TypeKind(KindOfType::TTypedef)),
            2 => Ok(NameKind::TypeKind(KindOfType::TRecordDef)),
            3 => Ok(NameKind::FunKind),
            4 => Ok(NameKind::ConstKind),
            _ => Err(format!("Out of range for KindOfType: {}", kind)),
        }
    }
}
