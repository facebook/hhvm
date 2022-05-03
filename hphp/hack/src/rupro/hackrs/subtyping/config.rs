// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[derive(Debug, Copy, Clone, Default, PartialEq, Eq)]
pub struct Config {
    pub ignore_generic_params: bool,
}
