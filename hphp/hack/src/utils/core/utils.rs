// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

/**
 * A\B\C -> \A\B\C
 */
pub fn add_ns(s: &str) -> Cow<str> {
    if s.starts_with("\\") {
        let mut new_str = String::with_capacity(1 + s.len());
        new_str.push_str("\\");
        new_str.push_str(s);
        Cow::Owned(new_str)
    } else {
        Cow::Borrowed(s)
    }
}

/**
 * \A\B\C -> A\B\C
 */
pub fn strip_ns(s: &str) -> &str {
    if s.is_empty() || !s.starts_with("\\") {
        return s;
    } else {
        return &s[1..];
    }
}
