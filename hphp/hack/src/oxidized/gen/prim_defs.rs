// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<8008c2bed20ca09b35dac73a73b07921>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use ocamlvalue_macro::Ocamlvalue;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub enum Comment {
    CmtLine(String),
    CmtBlock(String),
}
