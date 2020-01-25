// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use rusqlite::{Connection, OpenFlags};
use std::path::Path;

#[derive(Debug)]
pub struct Names {
    pub(crate) connection: Connection,
}

impl Names {
    pub fn readonly_from_file(path: impl AsRef<Path>) -> Self {
        let path = path.as_ref();
        assert!(path.exists());
        let connection =
            Connection::open_with_flags(path, OpenFlags::SQLITE_OPEN_READ_ONLY).unwrap();
        Names { connection }
    }

    pub fn new_in_memory() -> Self {
        let connection = Connection::open_in_memory().unwrap();
        Self::create_tables(&connection);
        Names { connection }
    }

    fn create_tables(connection: &Connection) {
        Self::create_file_info_table(connection);
        Self::create_funs_table(connection);
        Self::create_consts_table(connection);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_non_existent_const() {
        let names = Names::new_in_memory();

        let result = names.paths_of_consts(&["\\Foo"]);

        assert_eq!(
            1,
            result.len(),
            "The result vec must have exactly 1 element"
        );

        match result.first() {
            Some(Some(path)) => assert!(false, format!("Unexpected path: {:?}", path)),
            Some(None) => assert!(true),
            None => assert!(false, "Expected an element but got none"),
        }
    }
}
