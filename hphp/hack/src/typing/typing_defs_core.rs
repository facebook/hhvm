// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::{Allocator, ToOcamlRep, Value};
pub use oxidized_by_ref::typing_defs_core::*;

pub type PrimKind<'a> = oxidized_by_ref::aast_defs::Tprim<'a>;

// Rust versions of oxidized tast uses dummy placeholders for FuncBodyAnn and SavedEnv, which are converted
// to similarly dummy (using Default trait) OCaml representations. This is OK for now, since we use this conversion
// only to call into OCaml tast printer, and the simple types we are printing doesn't use annotations or saved envs.
#[derive(Debug)]
pub struct FuncBodyAnn;

impl ToOcamlRep for FuncBodyAnn {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        oxidized_by_ref::tast::FuncBodyAnn::default().to_ocamlrep(alloc)
    }
}

#[derive(Debug)]
pub struct SavedEnv;

impl ToOcamlRep for SavedEnv {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> Value<'a> {
        oxidized_by_ref::tast::SavedEnv::default().to_ocamlrep(alloc)
    }
}
