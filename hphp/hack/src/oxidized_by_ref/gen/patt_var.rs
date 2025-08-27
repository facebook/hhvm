// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<dffe08c710ece2594c449fb843bca791>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving (compare, eq, sexp, show)")]
pub type PattVar<'a> = str;
