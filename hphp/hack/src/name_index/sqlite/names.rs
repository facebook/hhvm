// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use rusqlite::{Connection, OpenFlags};
use std::path::Path;

use oxidized::file_info::NameType;
use oxidized::naming_types::KindOfType;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::{direct_decl_parser::Decls, file_info};

use bumpalo::collections::Vec;

use no_pos_hash::position_insensitive_hash;

use crate::{consts, file_infos, funs, types, Result};
use oxidized_by_ref::file_info::FileInfo;

#[derive(Debug)]
pub struct Names {
    connection: Connection,
}

impl Names {
    pub fn readonly_from_file(path: impl AsRef<Path>) -> Result<Self> {
        let path = path.as_ref();
        let connection = Connection::open_with_flags(path, OpenFlags::SQLITE_OPEN_READ_ONLY)?;
        Ok(Self { connection })
    }

    pub fn new_in_memory() -> Result<Self> {
        let connection = Connection::open_in_memory()?;
        Self::create_tables(&connection)?;
        Ok(Self { connection })
    }

    pub fn create_tables(connection: &Connection) -> Result<()> {
        file_infos::create_table(connection)?;
        funs::create_table(connection)?;
        types::create_table(connection)?;
        consts::create_table(connection)?;
        Ok(())
    }

    fn iter_to_arena_slice<'a, T>(
        iter: impl Iterator<Item = T>,
        arena: &'a bumpalo::Bump,
    ) -> &'a [T] {
        let mut result = match iter.size_hint().1 {
            Some(upper_bound) => Vec::with_capacity_in(upper_bound, arena),
            None => Vec::new_in(arena),
        };
        for item in iter {
            result.push(item);
        }
        result.into_bump_slice()
    }

    pub fn decl_to_fileinfo<'a>(
        decls: Decls<'a>,
        file_mode: Option<file_info::Mode>,
        arena: &'a bumpalo::Bump,
    ) -> FileInfo<'a> {
        let classes = Names::iter_to_arena_slice(
            decls.classes().map(|(name, x)| {
                &*arena.alloc((oxidized_by_ref::file_info::Pos::Full(x.name.0), name))
            }),
            arena,
        );
        let funs = Names::iter_to_arena_slice(
            decls.funs().map(|(name, x)| {
                &*arena.alloc((oxidized_by_ref::file_info::Pos::Full(x.pos), name))
            }),
            arena,
        );
        let typedefs = Names::iter_to_arena_slice(
            decls.typedefs().map(|(name, x)| {
                &*arena.alloc((oxidized_by_ref::file_info::Pos::Full(x.pos), name))
            }),
            arena,
        );
        let consts = Names::iter_to_arena_slice(
            decls.consts().map(|(name, x)| {
                &*arena.alloc((oxidized_by_ref::file_info::Pos::Full(x.pos), name))
            }),
            arena,
        );
        let record_defs = Names::iter_to_arena_slice(
            decls.records().map(|(name, x)| {
                &*arena.alloc((oxidized_by_ref::file_info::Pos::Full(x.pos), name))
            }),
            arena,
        );
        let hash = arena.alloc(Some(position_insensitive_hash(&decls) as isize));

        FileInfo {
            hash,
            file_mode,
            funs,
            classes,
            record_defs,
            typedefs,
            consts,
            comments: None,
        }
    }

    pub fn save_fileinfo(conn: &Connection, path_rel: &RelativePath, fileinfo: &FileInfo) {
        let _ = file_infos::insert(conn, path_rel, fileinfo);
        // todo reset of tables
    }

    pub fn get_path(&self, kind: NameType, name: &str) -> Result<Option<RelativePath>> {
        let get_type_path_if_kind_matches = |expected_kind| match self.get_type_path(name)? {
            Some((path, kind)) if kind == expected_kind => Ok(Some(path)),
            Some(_) | None => Ok(None),
        };
        use NameType::*;
        match kind {
            Fun => self.get_fun_path(name),
            Class => get_type_path_if_kind_matches(KindOfType::TClass),
            RecordDef => get_type_path_if_kind_matches(KindOfType::TRecordDef),
            Typedef => get_type_path_if_kind_matches(KindOfType::TTypedef),
            Const => self.get_const_path(name),
        }
    }

    pub fn get_fun_path(&self, name: &str) -> Result<Option<RelativePath>> {
        funs::get_path(&self.connection, name)
    }

    pub fn get_type_path(&self, name: &str) -> Result<Option<(RelativePath, KindOfType)>> {
        types::get_path(&self.connection, name)
    }

    pub fn get_const_path(&self, name: &str) -> Result<Option<RelativePath>> {
        consts::get_path(&self.connection, name)
    }

    pub fn get_fun_path_case_insensitive(&self, name: String) -> Result<Option<RelativePath>> {
        funs::get_path_case_insensitive(&self.connection, name)
    }

    pub fn get_type_path_case_insensitive(&self, name: String) -> Result<Option<RelativePath>> {
        types::get_path_case_insensitive(&self.connection, name)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_non_existent_const() {
        let names = Names::new_in_memory().unwrap();

        let result = names.get_const_path("\\Foo").unwrap();

        match result {
            Some(path) => assert!(false, "Unexpected path: {:?}", path),
            None => assert!(true),
        }
    }
}
