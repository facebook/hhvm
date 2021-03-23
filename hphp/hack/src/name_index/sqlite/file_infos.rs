// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::datatypes::*;
use crate::Result;

use ocamlrep::rc::RcOc;
use oxidized::file_info::FileInfo;
use oxidized::relative_path::RelativePath;
use rusqlite::{params, Connection};

#[derive(Debug)]
pub(crate) struct FileInfoItem {
    path: RcOc<RelativePath>,
    file_info: FileInfo,
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
