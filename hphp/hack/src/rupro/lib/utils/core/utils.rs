// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
pub fn strip_ns(s: &str) -> &str {
    s.strip_prefix('\\').unwrap_or(s)
}

pub fn add_ns(s: &str) -> String {
    if s.strip_prefix('\\').is_some() {
        s.to_string()
    } else {
        format!("\\{}", s)
    }
}
