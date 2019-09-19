// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use minimal_parser::MinimalSyntaxParser;
use parser_core_types::minimal_syntax::MinimalSyntax;

rust_parser_ffi::parse!(parse_minimal, MinimalSyntaxParser, MinimalSyntax);
