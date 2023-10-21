// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use anyhow::Result;
use bstr::BString;
use pos::RelativePath;
use pos::RelativePathCtx;
use tempfile::TempDir;

#[derive(Debug)]
pub struct DiskProvider {
    relative_path_ctx: Arc<RelativePathCtx>,
    // Drop the tempdir when the disk provider goes out of scope
    _hhi_root: Option<TempDir>,
}

impl DiskProvider {
    pub fn new(relative_path_ctx: Arc<RelativePathCtx>, hhi_root: Option<TempDir>) -> Self {
        Self {
            relative_path_ctx,
            _hhi_root: hhi_root,
        }
    }

    pub fn read(&self, file: RelativePath) -> std::io::Result<BString> {
        let absolute_path = file.to_absolute(&self.relative_path_ctx);
        Ok(std::fs::read(absolute_path)?.into())
    }
}

impl super::FileProvider for DiskProvider {
    fn get(&self, file: RelativePath) -> Result<BString> {
        Ok(self.read(file)?)
    }
}
