// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::collections::BTreeSet;
use std::path::PathBuf;

use oxidized::file_info::Mode;
use oxidized::file_info::NameType;
use relative_path::Prefix;
use relative_path::RelativePath;
use rusqlite::types::FromSql;
use rusqlite::types::FromSqlError;
use rusqlite::types::FromSqlResult;
use rusqlite::types::ValueRef;

#[derive(Debug, Default)]
pub struct SaveResult {
    pub files_added: usize,
    pub symbols_added: usize,
    pub collisions: BTreeMap<(NameType, String), BTreeSet<RelativePath>>,
    pub checksum: hh24_types::Checksum,
}

impl SaveResult {
    pub fn add_collision(
        &mut self,
        kind: NameType,
        name: String,
        path1: &RelativePath,
        path2: &RelativePath,
    ) {
        let paths = self.collisions.entry((kind, name)).or_default();
        paths.insert(path1.clone());
        paths.insert(path2.clone());
    }
}

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

/// This uses NonZeroU64 instead of i64 (used elsewhere in sqlite) just as a bit-packing optimization
#[derive(Copy, Clone, Debug, Eq, Hash, PartialEq, PartialOrd, Ord)]
pub struct FileInfoId(std::num::NonZeroU64);

impl FileInfoId {
    pub fn last_insert_rowid(conn: &rusqlite::Connection) -> Self {
        Self::from_i64(conn.last_insert_rowid())
    }

    fn from_i64(i: i64) -> Self {
        Self(std::num::NonZeroU64::new(i as u64).expect("SQLite autoincrement indices start at 1"))
    }
}

impl rusqlite::ToSql for FileInfoId {
    fn to_sql(&self) -> rusqlite::Result<rusqlite::types::ToSqlOutput<'_>> {
        Ok(rusqlite::types::ToSqlOutput::from(self.0.get() as i64))
    }
}

impl FromSql for FileInfoId {
    fn column_result(value: ValueRef<'_>) -> FromSqlResult<Self> {
        Ok(Self::from_i64(value.as_i64()?))
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
