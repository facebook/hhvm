// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<dd3d4038c4d17bd5a207143582c4a75d>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use error_hash_set::*;

#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving (ord, show)")]
pub type ErrorHash = ocamlrep::OCamlInt;
