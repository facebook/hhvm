// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use positioned_coroutine_parser::{ScState, SmartConstructors};

rust_parser_ffi::parse!(
    parse_positioned_with_coroutine_sc,
    SmartConstructors<'a>,
    ScState<'a>,
    positioned_coroutine_parser::parse_script,
);
