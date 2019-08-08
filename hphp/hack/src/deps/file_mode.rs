// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[derive(Debug)]
pub enum FileMode {
    Mphp,          // Do the best you can to support legacy PHP
    Mdecl,         // just declare signatures, don't check anything
    Mstrict,       // check everything!
    Mpartial,      // Don't fail if you see a function/class you don't know
    Mexperimental, // Strict mode + experimental features
}

impl FileMode {
    pub fn from_string(s: &str) -> Option<Self> {
        match s {
            "strict" | "" => Some(FileMode::Mstrict),
            "partial" => Some(FileMode::Mpartial),
            "experimental" => Some(FileMode::Mexperimental),
            _ => None,
        }
    }
}
