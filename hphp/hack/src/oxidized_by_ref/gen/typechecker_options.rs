// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<b097b84fd6af979c5e02acc45fa7366c>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving (eq, show)")]
pub type TypecheckerOptions<'a> = global_options::GlobalOptions<'a>;
