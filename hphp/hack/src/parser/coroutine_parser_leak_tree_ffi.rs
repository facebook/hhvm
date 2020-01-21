// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use coroutine_parser_leak_tree::{ScState, SmartConstructors};

rust_parser_ffi::parse!(
    parse_positioned_with_coroutine_sc_leak_tree,
    SmartConstructors<'a>,
    ScState<'a>,
    coroutine_parser_leak_tree::parse_script,
);
