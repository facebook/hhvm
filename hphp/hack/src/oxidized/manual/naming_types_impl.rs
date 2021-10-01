// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::convert::TryFrom;

use crate::file_info::NameType;
use crate::gen::naming_types::{KindOfType, NameKind};

impl TryFrom<i64> for NameKind {
    type Error = String;

    fn try_from(kind: i64) -> Result<Self, String> {
        // Must be kept in sync with naming_types.ml and namekind_to_i64
        // name_type.ml is where the numerical values are defined, and NameType in rust is
        // generated from it, and all other places including namkind_to_i64 just use it.
        if kind == (NameType::Class as i64) {
            Ok(NameKind::TypeKind(KindOfType::TClass))
        } else if kind == (NameType::Typedef as i64) {
            Ok(NameKind::TypeKind(KindOfType::TTypedef))
        } else if kind == (NameType::RecordDef as i64) {
            Ok(NameKind::TypeKind(KindOfType::TRecordDef))
        } else if kind == (NameType::Fun as i64) {
            Ok(NameKind::FunKind)
        } else if kind == (NameType::Const as i64) {
            Ok(NameKind::ConstKind)
        } else {
            Err("oops".to_owned())
        }
    }
}
