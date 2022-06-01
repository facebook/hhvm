// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::{FileProvider, FileType};
use anyhow::Result;
use datastore::Store;
use pos::{RelativePath, RelativePathCtx};
use std::sync::Arc;

#[derive(Debug)]
pub struct PlainFileProvider {
    relative_path_ctx: Arc<RelativePathCtx>,
    store: Arc<dyn Store<RelativePath, FileType>>,
}

impl PlainFileProvider {
    pub fn new(
        relative_path_ctx: Arc<RelativePathCtx>,
        store: Arc<dyn Store<RelativePath, FileType>>,
    ) -> Self {
        Self {
            relative_path_ctx,
            store,
        }
    }

    pub fn with_no_cache(relative_path_ctx: Arc<RelativePathCtx>) -> Self {
        Self {
            relative_path_ctx,
            store: Arc::new(datastore::EmptyStore),
        }
    }

    fn read_file_contents_from_disk(&self, file: RelativePath) -> Result<bstr::BString> {
        let absolute_path = file.to_absolute(&self.relative_path_ctx);
        Ok((std::fs::read(&absolute_path))?.into())
    }
}

impl FileProvider for PlainFileProvider {
    fn get(&self, file: RelativePath) -> Result<Option<FileType>> {
        self.store.get(file)
    }

    fn get_contents(&self, file: RelativePath) -> Result<bstr::BString> {
        match self.get(file)? {
            Some(FileType::Ide(bytes)) => Ok(bytes),
            Some(FileType::Disk(bytes)) => Ok(bytes),
            None => self.read_file_contents_from_disk(file),
        }
    }

    fn provide_file_for_tests(&self, file: RelativePath, contents: bstr::BString) -> Result<()> {
        self.store.insert(file, FileType::Disk(contents))
    }

    fn provide_file_for_ide(&self, file: RelativePath, contents: bstr::BString) -> Result<()> {
        self.store.insert(file, FileType::Ide(contents))
    }

    fn provide_file_hint(&self, file: RelativePath, file_type: FileType) -> Result<()> {
        if let FileType::Ide(_) = file_type {
            self.store.insert(file, file_type)?;
        }
        Ok(())
    }

    fn remove_batch(&self, files: &std::collections::BTreeSet<RelativePath>) -> Result<()> {
        self.store.remove_batch(&mut files.iter().copied())
    }
}
