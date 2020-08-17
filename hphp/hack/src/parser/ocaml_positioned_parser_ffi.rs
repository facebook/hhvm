// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocaml_positioned_parser::{ScState, SmartConstructors};
use smart_constructors;

rust_parser_ffi::parse!(
    ocaml_parse_positioned,
    SmartConstructors<'a>,
    ScState<'a>,
    ocaml_positioned_parser::parse_script
);
