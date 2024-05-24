// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<77ea09bd2b7d0c1a09829199c565c7d7>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use error_hash_set::*;

#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving (ord, show)")]
pub type ErrorHash = isize;
