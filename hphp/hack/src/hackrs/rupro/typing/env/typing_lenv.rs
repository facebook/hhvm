// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ty::reason::Reason;

use crate::typing::env::typing_per_cont_env::PerContEnv;

/// The local environment, where things might change often.
#[derive(Debug)]
pub struct TLEnv<R: Reason> {
    pub per_cont_env: PerContEnv<R>,
}

impl<R: Reason> TLEnv<R> {
    /// Initialize a new local environment.
    ///
    /// A local environment contains often changing information.
    pub fn new() -> Self {
        Self {
            per_cont_env: PerContEnv::new(),
        }
    }
}
