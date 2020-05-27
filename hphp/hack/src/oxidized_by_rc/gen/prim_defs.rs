// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<20b63a8cbb5e712cc5b226d85a275ed3>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_rc/regen.sh

use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Comment {
    CmtLine(std::rc::Rc<String>),
    CmtBlock(std::rc::Rc<String>),
}
