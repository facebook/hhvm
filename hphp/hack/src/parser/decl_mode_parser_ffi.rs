// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_mode_parser::DeclModeParser;
use parser_rust::positioned_syntax::PositionedSyntax;

rust_parser_ffi::parse!(
    parse_positioned_with_decl_mode_sc,
    DeclModeParser,
    PositionedSyntax
);
