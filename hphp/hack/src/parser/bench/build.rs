// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

fn main() {
    cc::Build::new()
        .file(
            Path::new("../../../utils/ocaml_ffi_mock/ocaml.c")
                .canonicalize()
                .unwrap(),
        )
        .compile("ocaml_stubs");
}
