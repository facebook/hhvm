// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<27e15e62167bf134e953f3622603eb22>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

pub use core::*;

#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show, yojson)")]
pub type PattVar = String;
