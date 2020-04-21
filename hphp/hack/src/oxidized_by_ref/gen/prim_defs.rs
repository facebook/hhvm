// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<59450d071f835dfb24a6e96dfbc6a54a>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub enum Comment<'a> {
    CmtLine(&'a str),
    CmtBlock(&'a str),
}
