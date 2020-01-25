// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::datatypes::*;
use crate::Names;
use crate::Result;

use oxidized::relative_path::RelativePath;
use rusqlite::{params, Connection, OptionalExtension};

#[derive(Clone, Debug)]
pub(crate) struct ConstItem {
    name: String,
    file_info_id: i64,
}

// TODO: some functions is only used in unit tests for now
#[allow(dead_code)]
impl Names {
    pub(crate) fn create_consts_table(connection: &Connection) -> Result<()> {
        let statement = "
            CREATE TABLE IF NOT EXISTS NAMING_CONSTS (
                HASH INTEGER PRIMARY KEY NOT NULL,
                FILE_INFO_ID INTEGER NOT NULL
            );";

        connection.execute(&statement, params![])?;
        Ok(())
    }

    fn insert_consts(&self, items: &[ConstItem]) -> Result<()> {
        let insert_statement = "
            INSERT INTO NAMING_CONSTS (
                HASH,
                FILE_INFO_ID
            ) VALUES (
                ?, ?
            );";

        let connection = &self.connection;
        let mut insert_statement = connection.prepare(&insert_statement)?;

        for item in items {
            let hash = convert::name_to_hash(&item.name);

            insert_statement.execute(params![hash, item.file_info_id])?;
        }
        Ok(())
    }

    pub fn paths_of_consts(&self, names: &[&str]) -> Result<Vec<Option<RelativePath>>> {
        let select_statement = "
            SELECT
                NAMING_FILE_INFO.PATH_PREFIX_TYPE,
                NAMING_FILE_INFO.PATH_SUFFIX
            FROM
                NAMING_CONSTS
            LEFT JOIN
                NAMING_FILE_INFO
            ON
                NAMING_CONSTS.FILE_INFO_ID = NAMING_FILE_INFO.FILE_INFO_ID
            WHERE
                NAMING_CONSTS.HASH = ?
            ";

        let connection = &self.connection;
        let mut select_statement = connection.prepare(&select_statement)?;
        names
            .into_iter()
            .map(|name| {
                let hash = convert::name_to_hash(&name);
                select_statement
                    .query_row::<RelativePath, _, _>(params![hash], |row| {
                        let prefix: SqlitePrefix = row.get(0)?;
                        let suffix: SqlitePathBuf = row.get(1)?;
                        Ok(RelativePath::make(prefix.value, suffix.value))
                    })
                    .optional()
            })
            .collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_add_const() {
        let names = Names::new_in_memory().unwrap();
        let consts = [ConstItem {
            name: "Foo".to_string(),
            file_info_id: 123,
        }];
        names.insert_consts(&consts).unwrap();
    }
}
