// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use md5::{Digest, Md5};
use oxidized::file_info::{Id, Mode};
use std::default::Default;

use oxidized::naming_types::KindOfType;
use oxidized::relative_path::Prefix;
use rusqlite::types::FromSql;
use rusqlite::types::{FromSqlError, FromSqlResult, ValueRef};
use std::convert::TryFrom;
use std::path::PathBuf;

pub(crate) struct SqlitePrefix {
    pub value: Prefix,
}

pub(crate) struct SqlitePathBuf {
    pub value: PathBuf,
}

pub(crate) struct SqliteKindOfType {
    pub value: KindOfType,
}

impl FromSql for SqlitePrefix {
    fn column_result(value: ValueRef) -> FromSqlResult<Self> {
        match value {
            ValueRef::Integer(i) => match Prefix::try_from(i as usize) {
                Ok(value) => Ok(SqlitePrefix { value }),
                Err(_) => Err(FromSqlError::OutOfRange(i)),
            },
            _ => Err(FromSqlError::InvalidType),
        }
    }
}

impl FromSql for SqlitePathBuf {
    fn column_result(value: ValueRef) -> FromSqlResult<Self> {
        match value.as_str() {
            Ok(s) => Ok(SqlitePathBuf {
                value: PathBuf::from(s),
            }),
            _ => Err(FromSqlError::InvalidType),
        }
    }
}

impl FromSql for SqliteKindOfType {
    fn column_result(value: ValueRef) -> FromSqlResult<Self> {
        match value {
            ValueRef::Integer(i) => match KindOfType::try_from(i) {
                Ok(value) => Ok(Self { value }),
                Err(_) => Err(FromSqlError::OutOfRange(i)),
            },
            _ => Err(FromSqlError::InvalidType),
        }
    }
}

pub(crate) mod convert {
    use super::*;

    pub fn ids_to_string(ids: &[Id]) -> String {
        let ids = ids
            .into_iter()
            .map(|id| id.1.clone())
            .collect::<Vec<String>>();
        ids.join("|")
    }

    pub fn mode_to_i64(mode: Option<Mode>) -> Option<i64> {
        match mode {
            Some(mode) => Some(match mode {
                Mode::Mphp => 0,
                Mode::Mdecl => 1,
                Mode::Mstrict => 2,
                Mode::Mpartial => 3,
            }),
            None => None,
        }
    }

    pub fn prefix_to_i64(prefix: Prefix) -> i64 {
        match prefix {
            Prefix::Root => 0,
            Prefix::Hhi => 1,
            Prefix::Dummy => 2,
            Prefix::Tmp => 3,
        }
    }

    pub fn name_to_hash(name: &str) -> i64 {
        let mut digest = Md5::new();
        digest.input(name);

        let bytes = digest.result();

        // Chop off extra bytes
        let mut b: [u8; 8] = Default::default();
        b.copy_from_slice(&bytes[..8]);
        return i64::from_ne_bytes(b);
    }
}
