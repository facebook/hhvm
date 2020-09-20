// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Write;

pub type Result<T> = std::result::Result<T, Box<dyn std::error::Error>>;

pub fn to_snake(s: &str) -> String {
    let mut r = String::new();
    let chars: Vec<char> = s.chars().collect();
    for i in 0..chars.len() {
        if chars[i].is_ascii_uppercase() {
            if i != 0
                && chars[i - 1].is_ascii_lowercase()
                && (i + 1 == chars.len() || chars[i + 1].is_ascii_lowercase())
            {
                r.push('_');
            }
            r.push(chars[i].to_ascii_lowercase());
        } else {
            r.push(chars[i])
        }
    }
    r
}

pub fn insert_header(s: &str, command: &str) -> Result<String> {
    let mut content = String::new();
    write!(
        &mut content,
        "
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @{} <<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>
//
// To regenerate this file, run:
//   {}

{}
",
        "generated", command, s
    )?;
    Ok(content)
}
