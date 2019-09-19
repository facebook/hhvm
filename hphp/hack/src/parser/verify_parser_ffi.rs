// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::positioned_syntax::PositionedSyntax;
use verify_parser::VerifyParser;

rust_parser_ffi::parse!(
    parse_positioned_with_verify_sc,
    VerifyParser,
    PositionedSyntax
);
