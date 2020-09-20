// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use minimal_parser::{ScState, SmartConstructors};

rust_parser_ffi::parse!(
    parse_minimal,
    SmartConstructors,
    ScState,
    minimal_parser::parse_script
);
