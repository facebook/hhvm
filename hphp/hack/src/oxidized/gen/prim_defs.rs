// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<26e025d687af99d3c382253848a694f2>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#[derive(Clone, Debug)]
pub enum Comment {
    CmtLine(String),
    CmtBlock(String),
    CmtMarkup(String),
}
