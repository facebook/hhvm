// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use ir::StringInterner;
use ir::UnitBytesId;

pub(crate) struct StringCache {
    pub interner: Arc<StringInterner>,
}

impl StringCache {
    pub fn new(interner: Arc<StringInterner>) -> Self {
        Self { interner }
    }

    pub fn intern(&self, id: UnitBytesId) -> Result<hhbc::StringId, std::str::Utf8Error> {
        Ok(hhbc::intern(std::str::from_utf8(
            &self.interner.lookup_bytes(id),
        )?))
    }
}
