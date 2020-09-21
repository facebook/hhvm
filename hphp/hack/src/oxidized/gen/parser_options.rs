// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<1c8cee3400ea1d45cbbd5df3af250ed1>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub type ParserOptions = global_options::GlobalOptions;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct FfiT(
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
    pub bool,
);
