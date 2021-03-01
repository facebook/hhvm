// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// This is OCaml FFI entry point.
rust_parser_ffi::parse!(parse_positioned, positioned_parser::parse_script);
// The function generated is `parse_positioned` and its implementation
// calls into `positioned_parser::parse_script`.
