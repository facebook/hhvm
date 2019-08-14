// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<d603e2cf84a70a66850a707149206492>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::IntoOcamlRep;

#[derive(Clone, Debug, IntoOcamlRep)]
pub enum Comment {
    CmtLine(String),
    CmtBlock(String),
    CmtMarkup(String),
}
