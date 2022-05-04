// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use oxidized::file_info::Mode;

use hh24_types::u64_hash_wrapper_impls;
use oxidized::relative_path::Prefix;
use rusqlite::types::FromSql;
use rusqlite::types::{FromSqlError, FromSqlResult, ValueRef};
use std::path::PathBuf;

pub(crate) struct SqlitePrefix {
    pub value: Prefix,
}

pub(crate) struct SqlitePathBuf {
    pub value: PathBuf,
}

impl FromSql for SqlitePrefix {
    fn column_result(value: ValueRef<'_>) -> FromSqlResult<Self> {
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
    fn column_result(value: ValueRef<'_>) -> FromSqlResult<Self> {
        match value.as_str() {
            Ok(s) => Ok(SqlitePathBuf {
                value: PathBuf::from(s),
            }),
            _ => Err(FromSqlError::InvalidType),
        }
    }
}

#[derive(Copy, Clone, Debug, Eq, Hash, PartialEq, PartialOrd, Ord)]
pub struct FileInfoId(std::num::NonZeroU64);

impl nohash_hasher::IsEnabled for FileInfoId {}

impl rusqlite::ToSql for FileInfoId {
    fn to_sql(&self) -> rusqlite::Result<rusqlite::types::ToSqlOutput<'_>> {
        Ok(rusqlite::types::ToSqlOutput::from(self.0.get() as i64))
    }
}

impl FromSql for FileInfoId {
    fn column_result(value: ValueRef<'_>) -> FromSqlResult<Self> {
        Ok(Self(
            std::num::NonZeroU64::new(value.as_i64()? as u64)
                .expect("SQLite autoincrement indices start at 1"),
        ))
    }
}

pub(crate) mod convert {
    use super::*;

    #[allow(unused)]
    pub fn mode_to_i64(mode: Option<Mode>) -> Option<i64> {
        mode.map(|mode| match mode {
            Mode::Mhhi => 0,
            Mode::Mstrict => 1,
        })
    }

    #[allow(unused)]
    pub fn prefix_to_i64(prefix: Prefix) -> i64 {
        match prefix {
            Prefix::Root => 0,
            Prefix::Hhi => 1,
            Prefix::Dummy => 2,
            Prefix::Tmp => 3,
        }
    }

    #[allow(unused)]
    pub fn name_to_hash(dep_type: typing_deps_hash::DepType, name: &str) -> i64 {
        // It happens that hash1 only ever returns 64bit numbers with the high bit 0
        // (for convenience of interop with ocaml). Therefore, u64/i64 will both
        // interpret the same bit pattern as the same positive integer. When we store
        // in sqlite, it will also be stored as a positive integer.
        typing_deps_hash::hash1(dep_type, name.as_bytes()) as i64
    }
}

/// The hash of a lowercased toplevel symbol name. See also hh24_types::TopLevelSymbolHash
/// which is used throughout the codebase; this CanonSymbolHash by contrast is solely
/// internal to sqlite.
#[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord)]
#[derive(derive_more::UpperHex, derive_more::LowerHex)]
pub struct ToplevelCanonSymbolHash(u64);
u64_hash_wrapper_impls! { ToplevelCanonSymbolHash }

impl ToplevelCanonSymbolHash {
    fn from(dep_type: typing_deps_hash::DepType, mut symbol: String) -> Self {
        symbol.make_ascii_lowercase();
        Self(typing_deps_hash::hash1(dep_type, symbol.as_bytes()))
    }

    pub fn from_type(symbol: String) -> Self {
        Self::from(typing_deps_hash::DepType::Type, symbol)
    }

    pub fn from_fun(symbol: String) -> Self {
        Self::from(typing_deps_hash::DepType::Fun, symbol)
    }
}

/// The flags associated with types. Used to denote whether a type is a TypeDef or Class
#[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord)]
#[derive(derive_more::UpperHex, derive_more::LowerHex)]
pub struct ToplevelSymbolFlags(u64);
u64_hash_wrapper_impls! { ToplevelSymbolFlags }
