// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::datatypes::*;

use oxidized::relative_path::RelativePath;
use rusqlite::{params, Connection};
use std::sync::{Arc, Mutex};

#[derive(Clone, Debug)]
pub(crate) struct ConstsTable {
    connection: Arc<Mutex<Connection>>,
}

#[derive(Clone, Debug)]
pub(crate) struct ConstItem {
    name: String,
    file_info_id: i64,
}

// TODO: some functions is only used in unit tests for now
#[allow(dead_code)]
impl ConstsTable {
    pub fn new(connection: Arc<Mutex<Connection>>) -> Self {
        ConstsTable {
            connection: connection.clone(),
        }
    }

    pub fn create(&self) {
        let statement = "
            CREATE TABLE IF NOT EXISTS NAMING_CONSTS (
                HASH INTEGER PRIMARY KEY NOT NULL,
                FILE_INFO_ID INTEGER NOT NULL
            );";

        self.connection
            .lock()
            .unwrap()
            .execute(&statement, params![])
            .unwrap();
    }

    pub fn insert(self, items: &[ConstItem]) {
        let insert_statement = "
            INSERT INTO NAMING_CONSTS (
                HASH,
                FILE_INFO_ID
            ) VALUES (
                ?, ?
            );";

        let connection = self.connection.lock().unwrap();
        let mut insert_statement = connection.prepare(&insert_statement).unwrap();

        for item in items {
            let hash = Convert::name_to_hash(&item.name);

            let result = insert_statement.execute(params![hash, item.file_info_id]);

            match result {
                Ok(_v) => println!("Inserted row OK"),
                Err(e) => println!("Error: {:?}", e),
            }
        }
    }

    pub fn map_names_to_paths(self, names: &[&str]) -> Vec<RelativePath> {
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

        let connection = self.connection.lock().unwrap();
        let mut select_statement = connection.prepare(&select_statement).unwrap();
        names
            .into_iter()
            .map(|name| {
                let hash = Convert::name_to_hash(&name);
                let path = &select_statement
                    .query_row::<RelativePath, _, _>(params![hash], |row| {
                        let prefix: SqlitePrefix = row.get(0).unwrap();
                        let suffix: SqlitePathBuf = row.get(1).unwrap();
                        Ok(RelativePath::make(prefix.value, suffix.value))
                    })
                    .unwrap();

                path.clone()
            })
            .collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_add_const() {
        let const_table =
            ConstsTable::new(Arc::new(Mutex::new(Connection::open_in_memory().unwrap())));
        const_table.create();
        let consts = [ConstItem {
            name: "Foo".to_string(),
            file_info_id: 123,
        }];
        const_table.insert(&consts);

        assert!(true)
    }
}
