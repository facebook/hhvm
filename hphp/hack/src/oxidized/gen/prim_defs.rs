// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<338e8754b75150c5150d97939ba75366>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum Comment {
    CmtLine(String),
    CmtBlock(String),
}
