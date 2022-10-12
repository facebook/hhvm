// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Took this from https://docs.rs/once_cell/latest/once_cell/index.html#lazily-compiled-regex
/// with this macro we can avoid re-initializing regexes, which are expensive to do
#[macro_export]
macro_rules! regex {
    ($re:literal $(,)?) => {{
        static RE: once_cell::sync::OnceCell<Regex> = once_cell::sync::OnceCell::new();
        RE.get_or_init(|| Regex::new($re).unwrap())
    }};
}
