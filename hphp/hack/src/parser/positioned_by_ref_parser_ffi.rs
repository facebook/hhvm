// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

rust_parser_ffi::parse_with_arena!(
    parse_positioned_by_ref,
    positioned_by_ref_parser::parse_script
);
