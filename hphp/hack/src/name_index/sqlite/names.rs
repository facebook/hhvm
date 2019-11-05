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
        // TODO: check if the path exists, and create an in-memory representation if it doesn't
        let connection = Arc::new(Mutex::new(
            Connection::open_with_flags(path, OpenFlags::SQLITE_OPEN_READ_ONLY).unwrap(),
        ));

        Names {
            consts: ConstsTable::new(connection.clone()),
            file_infos: FileInfoTable::new(connection.clone()),
            path: path.to_path_buf(),
        }
    }

    pub fn paths_of_consts(self, names: &[&str]) -> Vec<RelativePath> {
        self.consts.map_names_to_paths(names)
    }
}
