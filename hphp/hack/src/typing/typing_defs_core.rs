// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::Allocator;
use ocamlrep::OpaqueValue;
use ocamlrep::ToOcamlRep;
pub use oxidized_by_ref::typing_defs_core::*;

pub type PrimKind = oxidized_by_ref::aast_defs::Tprim;

#[derive(Debug)]
pub struct SavedEnv;

impl ToOcamlRep for SavedEnv {
    fn to_ocamlrep<'a, A: Allocator>(&'a self, alloc: &'a A) -> OpaqueValue<'a> {
        let saved_env = oxidized_by_ref::tast::SavedEnv::default();
        // SAFETY: Transmute away the lifetime to allow the stack-allocated
        // value to be converted to OCaml. Won't break type safety in Rust, but
        // may produce broken OCaml values if used with an invocation of
        // `ocamlrep::Allocator::add_root` higher up.
        let saved_env = unsafe {
            std::mem::transmute::<
                &'_ oxidized_by_ref::tast::SavedEnv<'_>,
                &'a oxidized_by_ref::tast::SavedEnv<'a>,
            >(&saved_env)
        };
        saved_env.to_ocamlrep(alloc)
    }
}
