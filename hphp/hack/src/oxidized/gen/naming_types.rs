// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<814b4855c018c39d76fc804c74903d1e>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum KindOfType {
    TClass = 0,
    TTypedef = 1,
    TRecordDef = 2,
}
