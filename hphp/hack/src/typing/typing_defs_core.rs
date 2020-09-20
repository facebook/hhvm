// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::{Allocator, OpaqueValue, ToOcamlRep};

pub use oxidized_by_ref::typing_defs_core::*;

pub type PrimKind<'a> = oxidized_by_ref::aast_defs::Tprim<'a>;

#[derive(Debug)]
pub struct SavedEnv;

impl ToOcamlRep for SavedEnv {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        oxidized_by_ref::tast::SavedEnv::default().to_ocamlrep(alloc)
    }
}
