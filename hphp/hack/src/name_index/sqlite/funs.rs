// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::datatypes::*;
use crate::Result;

use oxidized::relative_path::RelativePath;
use rusqlite::{params, Connection, OptionalExtension};

#[derive(Clone, Debug)]
pub(crate) struct FunItem {
    name: String,
    file_info_id: i64,
}

pub fn create_table(connection: &Connection) -> Result<()> {
    let statement = "
        CREATE TABLE IF NOT EXISTS NAMING_FUNS (
            HASH INTEGER PRIMARY KEY NOT NULL,
            CANON_HASH INTEGER NOT NULL,
            FILE_INFO_ID INTEGER NOT NULL
        );";

    connection.execute(&statement, params![])?;
    Ok(())
}

// Used only in tests for now.
#[cfg(test)]
fn insert(connection: &Connection, items: Vec<FunItem>) -> Result<()> {
    let insert_statement = "
        INSERT INTO NAMING_FUNS (
            HASH,
            CANON_HASH,
            FILE_INFO_ID
        ) VALUES (
            ?, ?, ?
        );";

    let mut insert_statement = connection.prepare(&insert_statement)?;

    for mut item in items.into_iter() {
        let hash = convert::name_to_hash(typing_deps_hash::DepType::Fun, &item.name);
        item.name.make_ascii_lowercase();
        let canon_hash = convert::name_to_hash(typing_deps_hash::DepType::Fun, &item.name);

        insert_statement.execute(params![hash, canon_hash, item.file_info_id])?;
    }
    Ok(())
}

pub fn get_path(connection: &Connection, name: &str) -> Result<Option<RelativePath>> {
    let select_statement = "
        SELECT
            NAMING_FILE_INFO.PATH_PREFIX_TYPE,
            NAMING_FILE_INFO.PATH_SUFFIX
        FROM
            NAMING_FUNS
        LEFT JOIN
            NAMING_FILE_INFO
        ON
            NAMING_FUNS.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE
            NAMING_FUNS.HASH = ?
        ";

    let mut select_statement = connection.prepare_cached(&select_statement)?;
    let hash = convert::name_to_hash(typing_deps_hash::DepType::Fun, &name);
    select_statement
        .query_row::<RelativePath, _, _>(params![hash], |row| {
            let prefix: SqlitePrefix = row.get(0)?;
            let suffix: SqlitePathBuf = row.get(1)?;
            Ok(RelativePath::make(prefix.value, suffix.value))
        })
        .optional()
}

pub fn get_path_case_insensitive(
    connection: &Connection,
    mut name: String,
) -> Result<Option<RelativePath>> {
    let select_statement = "
        SELECT
            NAMING_FILE_INFO.PATH_PREFIX_TYPE,
            NAMING_FILE_INFO.PATH_SUFFIX
        FROM
            NAMING_FUNS
        LEFT JOIN
            NAMING_FILE_INFO
        ON
            NAMING_FUNS.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
        WHERE
            NAMING_FUNS.CANON_HASH = ?
        ";

    let mut select_statement = connection.prepare_cached(&select_statement)?;
    name.make_ascii_lowercase();
    let hash = convert::name_to_hash(typing_deps_hash::DepType::Fun, &name);
    select_statement
        .query_row::<RelativePath, _, _>(params![hash], |row| {
            let prefix: SqlitePrefix = row.get(0)?;
            let suffix: SqlitePathBuf = row.get(1)?;
            Ok(RelativePath::make(prefix.value, suffix.value))
        })
        .optional()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_add_fun() {
        let conn = Connection::open_in_memory().unwrap();
        create_table(&conn).unwrap();
        let funs = vec![FunItem {
            name: "Foo".to_string(),
            file_info_id: 123,
        }];
        insert(&conn, funs).unwrap();
    }
}
