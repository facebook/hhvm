// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::positioned_syntax::PositionedValue;
use positioned_coroutine_parser::ocaml_syntax::OcamlSyntax;
use positioned_coroutine_parser::CoroutineParser;

rust_parser_ffi::parse!(
    parse_positioned_with_coroutine_sc,
    CoroutineParser,
    OcamlSyntax<PositionedValue>
);
