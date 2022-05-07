// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{Error, FileProvider, FileType, Result};
use dashmap::DashMap;
use parking_lot::RwLock;
use pos::{RelativePath, RelativePathCtx};
use std::sync::Arc;

#[derive(Debug)]
pub struct PlainFileProvider {
    curr: DashMap<RelativePath, Option<FileType>>,
    prev: RwLock<Vec<DashMap<RelativePath, Option<FileType>>>>,
    relative_path_ctx: Arc<RelativePathCtx>,
}

impl PlainFileProvider {
    pub fn new(relative_path_ctx: Arc<RelativePathCtx>) -> Self {
        PlainFileProvider {
            curr: DashMap::new(),
            prev: RwLock::new(Vec::new()),
            relative_path_ctx,
        }
    }

    fn read_file_contents_from_disk(&self, file: RelativePath) -> Result<bstr::BString> {
        let absolute_path = file.to_absolute(&self.relative_path_ctx);
        Ok((std::fs::read(&absolute_path).map_err(Error::IoError))?.into())
    }
}

impl FileProvider for PlainFileProvider {
    fn get(&self, file: RelativePath) -> Option<FileType> {
        // Check current.
        if let Some(kv) = self.curr.get(&file) {
            if let Some(ft) = kv.value() {
                return Some(ft.clone());
            }
        }
        // Check ancestors.
        let scopes = self.prev.read();
        for scope in scopes.iter().rev() {
            if let Some(kv) = scope.get(&file) {
                if let Some(ft) = kv.value() {
                    return Some(ft.clone());
                }
            }
        }
        // Not found.
        None
    }

    fn get_contents(&self, file: RelativePath) -> Result<bstr::BString> {
        match self.get(file) {
            Some(FileType::Ide(bytes)) => Ok(bytes),
            Some(FileType::Disk(bytes)) => Ok(bytes),
            None => self.read_file_contents_from_disk(file),
        }
    }

    fn provide_file_for_tests(&self, file: RelativePath, contents: bstr::BString) {
        self.curr.insert(file, Some(FileType::Disk(contents)));
    }

    fn provide_file_for_ide(&self, file: RelativePath, contents: bstr::BString) {
        self.curr.insert(file, Some(FileType::Ide(contents)));
    }

    fn provide_file_hint(&self, file: RelativePath, file_type: FileType) {
        if let FileType::Ide(_) = file_type {
            self.curr.insert(file, Some(file_type));
        }
    }

    fn remove_batch(&self, files: &std::collections::BTreeSet<RelativePath>) {
        for file in files {
            self.curr.insert(*file, None);
        }
    }

    fn push_local_changes(&self) {
        let mut lck = self.prev.write();
        let prev = &mut *lck;
        prev.push(self.curr.clone());
        self.curr.clear()
    }

    fn pop_local_changes(&self) {
        self.curr.clear()
    }
}
