// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use super::Allocator;
use crate::reason::Reason;
use crate::utils::core::LocalId;

impl<R: Reason> Allocator<R> {
    pub fn local_id_from_ast(&self, local_id: &oxidized::local_id::LocalId) -> LocalId {
        LocalId::new(local_id.0.try_into().unwrap(), self.symbol(&local_id.1))
    }
}
