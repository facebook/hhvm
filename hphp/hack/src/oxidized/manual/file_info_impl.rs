// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::file_info::Mode;
use crate::gen::file_info::NameType;
use crate::gen::naming_types::KindOfType;
use rusqlite::types::FromSql;
use rusqlite::types::{FromSqlError, FromSqlResult, ValueRef};

impl Mode {
    pub fn from_string(s: &str) -> Option<Self> {
        match s {
            "strict" | "" => Some(Mode::Mstrict),
            "partial" => Some(Mode::Mpartial),
            _ => None,
        }
    }
}

impl From<KindOfType> for NameType {
    fn from(kind: KindOfType) -> Self {
        match kind {
            KindOfType::TClass => NameType::Class,
            KindOfType::TTypedef => NameType::Typedef,
            KindOfType::TRecordDef => NameType::RecordDef,
        }
    }
}

impl FromSql for NameType {
    fn column_result(value: ValueRef<'_>) -> FromSqlResult<Self> {
        match value {
            ValueRef::Integer(i) => {
                if i == NameType::Fun as i64 {
                    Ok(NameType::Fun)
                } else if i == NameType::Const as i64 {
                    Ok(NameType::Const)
                } else if i == NameType::Class as i64 {
                    Ok(NameType::Class)
                } else if i == NameType::Typedef as i64 {
                    Ok(NameType::Typedef)
                } else if i == NameType::RecordDef as i64 {
                    Ok(NameType::RecordDef)
                } else {
                    Err(FromSqlError::OutOfRange(i))
                }
            }
            _ => Err(FromSqlError::InvalidType),
        }
    }
}
