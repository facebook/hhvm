// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{FileProvider, FileType};
use dashmap::DashMap;
use parking_lot::RwLock;
use pos::RelativePath;

#[derive(Debug)]
pub struct PlainFileProvider {
    #[allow(dead_code)]
    curr: DashMap<RelativePath, Option<FileType>>,
    #[allow(dead_code)]
    prev: RwLock<Vec<DashMap<RelativePath, Option<FileType>>>>,
}

impl PlainFileProvider {
    pub fn new() -> Self {
        PlainFileProvider {
            curr: DashMap::new(),
            prev: RwLock::new(Vec::new()),
        }
    }
}

impl FileProvider for PlainFileProvider {
    fn get(&self, _file: RelativePath) -> Option<FileType> {
        todo!()
        /*
        // Check current.
        if let Some(kv) = self.curr.get(&file) {
            if let Some(ft) = kv.value() {
                return Some(ft.clone())
            }
        }
        // Check ancestors.
        let scopes = self.prev.read();
        for scope in scopes.iter().rev() {
            if let Some(kv) = scope.get(&file) {
                if let Some(ft) = kv.value() {
                    return Some(ft.clone())
                }
            }
        }
        // Not found.
        None
         */
    }

    fn get_contents(&self, _file: RelativePath) -> bstr::BString {
        todo!()
    }

    fn provide_file_for_tests(&self, _file: RelativePath, _contents: bstr::BString) {
        todo!()
    }

    fn provide_file_for_ide(&self, _file: RelativePath, _contents: bstr::BString) {
        todo!()
    }

    fn provide_file_hint(&self, _file: RelativePath, _file_type: FileType) {
        todo!()
    }

    fn remove_batch<I: Iterator<Item = RelativePath>>(&self, _files: I) {
        todo!()
    }

    fn push_local_changes(&self) {
        todo!()
    }

    fn pop_local_changes(&self) {
        todo!()
    }
}
