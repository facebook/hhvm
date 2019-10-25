// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::file_info::Mode;

impl Mode {
    pub fn from_string(s: &str) -> Option<Self> {
        match s {
            "strict" | "" => Some(Mode::Mstrict),
            "partial" => Some(Mode::Mpartial),
            "experimental" => Some(Mode::Mexperimental),
            _ => None,
        }
    }
}
