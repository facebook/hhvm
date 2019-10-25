// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<42ae59b1fcc6c70663faca8cd4023643>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;

#[derive(Clone, Debug, OcamlRep)]
pub enum Comment {
    CmtLine(String),
    CmtBlock(String),
}
