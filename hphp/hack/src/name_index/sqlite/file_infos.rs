// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::datatypes::*;
use crate::Result;

use ocamlrep::rc::RcOc;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::file_info::FileInfo;
use rusqlite::{params, Connection};

#[derive(Debug)]
pub(crate) struct FileInfoItem<'a> {
    path: RcOc<RelativePath>,
    file_info: FileInfo<'a>,
}

pub fn create_table(connection: &Connection) -> Result<()> {
    let statement = "
        CREATE TABLE IF NOT EXISTS NAMING_FILE_INFO (
            FILE_INFO_ID INTEGER PRIMARY KEY AUTOINCREMENT,
            PATH_PREFIX_TYPE INTEGER NOT NULL,
            PATH_SUFFIX TEXT NOT NULL,
            TYPE_CHECKER_MODE INTEGER,
            DECL_HASH TEXT,
            CLASSES TEXT,
            CONSTS TEXT,
            FUNS TEXT,
            RECS TEXT,
            TYPEDEFS TEXT
        );";

    connection.execute(&statement, params![])?;
    Ok(())
}

pub fn insert(connection: &Connection, path_rel: &RelativePath, fileinfo: &FileInfo) -> Result<()> {
    let prefix_type = path_rel.prefix() as u8; // verify this makes sense
    let suffix = path_rel.path().to_str().unwrap();
    let type_checker_mode = convert::mode_to_i64(fileinfo.file_mode);
    let hash = fileinfo.hash;

    let insert_statement = "
        INSERT INTO NAMING_FILE_INFO(
            PATH_PREFIX_TYPE,
            PATH_SUFFIX,
            TYPE_CHECKER_MODE,
            DECL_HASH,
            CLASSES,
            CONSTS,
            FUNS,
            RECS,
            TYPEDEFS
        )
        VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9);
      ";

    let mut insert_statement = connection.prepare_cached(&insert_statement)?;
    let ids_to_string = |ids: &[&oxidized_by_ref::file_info::Id]| {
        let mut s = String::new();
        for id in ids {
            s.push_str(id.1);
            s.push('|');
        }
        s.pop(); // Remove trailing pipe character
        s
    };
    insert_statement.execute(params![
        prefix_type,
        suffix,
        type_checker_mode,
        hash,
        ids_to_string(fileinfo.classes),
        ids_to_string(fileinfo.consts),
        ids_to_string(fileinfo.funs),
        ids_to_string(fileinfo.record_defs),
        ids_to_string(fileinfo.typedefs),
    ])?;
    Ok(())
}
