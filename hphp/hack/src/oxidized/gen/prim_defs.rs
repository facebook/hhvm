// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<97c35dfbe72ed0e592ce0723b20367b8>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::IntoOcamlRep;
use ocamlvalue_macro::Ocamlvalue;

#[derive(Clone, Debug, IntoOcamlRep, Ocamlvalue)]
pub enum Comment {
    CmtLine(String),
    CmtBlock(String),
    CmtMarkup(String),
}
