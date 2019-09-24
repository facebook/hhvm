// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::gen::file_info::Mode;

impl Mode {
    pub unsafe fn from_ocamlvalue(value: ocaml::Value) -> Self {
        let mode_raw = value.i32_val() as u32;
        match mode_raw {
            0 => Mode::Mphp,
            1 => Mode::Mdecl,
            2 => Mode::Mstrict,
            3 => Mode::Mpartial,
            4 => Mode::Mexperimental,
            _ => panic!("mode {} is not defined", mode_raw.to_string()),
        }
    }

    pub fn from_string(s: &str) -> Option<Self> {
        match s {
            "strict" | "" => Some(Mode::Mstrict),
            "partial" => Some(Mode::Mpartial),
            "experimental" => Some(Mode::Mexperimental),
            _ => None,
        }
    }
}
