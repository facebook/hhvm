// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::file_info::Mode;
use crate::full_fidelity_parser_env::FullFidelityParserEnv;

impl FullFidelityParserEnv {
    pub fn is_experimental_mode(&self) -> bool {
        self.mode == Some(Mode::Mexperimental)
    }
}
