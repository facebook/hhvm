// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use ir::StringInterner;

pub(crate) struct StringCache {
    pub interner: Arc<StringInterner>,
}

impl StringCache {
    pub fn new(interner: Arc<StringInterner>) -> Self {
        Self { interner }
    }
}
