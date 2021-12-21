// Copyright (c) 2021, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;
use serde_json::json;

use ocamlrep_ocamlpool::ocaml_ffi_with_arena;
use oxidized_by_ref::typing_defs_core;

ocaml_ffi_with_arena! {
    fn ty_to_json<'a>(
        arena: &'a Bump,
        ty: typing_defs_core::Ty<'_>,
    ) -> String {
        let j = json!(ty);
        serde_json::to_string(&j).unwrap().to_string()
    }
}
