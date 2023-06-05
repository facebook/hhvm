// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a6d7a7156d962167936d1fa31a0bb7ac>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use core::*;

#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show, yojson)")]
pub type PattVar<'a> = str;
