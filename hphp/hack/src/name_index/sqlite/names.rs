// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::consts::*;
use crate::file_infos::*;

use oxidized::relative_path::RelativePath;
use rusqlite::{Connection, OpenFlags};
use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex};

// TODO: file_infos is not used yet
#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct Names {
    consts: ConstsTable,
    file_infos: FileInfoTable,
    path: PathBuf,
}

impl Names {
    pub fn new(path: &Path) -> Self {
        let connection = if path.exists() {
            println!("Loading name index from '{:#?}' in read-only mode", path);
            Arc::new(Mutex::new(
                Connection::open_with_flags(path, OpenFlags::SQLITE_OPEN_READ_ONLY).unwrap(),
            ))
        } else {
            println!(
                "Path '{:#?}' does not point to a file on disk - creating an in-memory read-write database",
                path,
            );
            let connection = Arc::new(Mutex::new(Connection::open_in_memory().unwrap()));
            FileInfoTable::new(connection.clone()).create();
            ConstsTable::new(connection.clone()).create();

            connection
        };

        Names {
            consts: ConstsTable::new(connection.clone()),
            file_infos: FileInfoTable::new(connection.clone()),
            path: path.to_path_buf(),
        }
    }

    pub fn paths_of_consts(&self, names: &[&str]) -> Vec<Option<RelativePath>> {
        self.consts.map_names_to_paths(names)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_non_existent_const() {
        let names = Names::new(Path::new(""));

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
