// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use anyhow::Result;
use bstr::BString;
use pos::RelativePath;
use pos::RelativePathCtx;

#[derive(Debug)]
pub struct DiskProvider {
    relative_path_ctx: Arc<RelativePathCtx>,
}

impl DiskProvider {
    pub fn new(relative_path_ctx: Arc<RelativePathCtx>) -> Self {
        Self { relative_path_ctx }
    }

    pub fn read(&self, file: RelativePath) -> std::io::Result<BString> {
        let absolute_path = file.to_absolute(&self.relative_path_ctx);
        Ok(std::fs::read(&absolute_path)?.into())
    }
}

impl super::FileProvider for DiskProvider {
    fn get(&self, file: RelativePath) -> Result<BString> {
        Ok(self.read(file)?)
    }
}
