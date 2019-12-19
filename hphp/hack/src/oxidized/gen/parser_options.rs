// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<7b19f88a89782818d48e254ec30fc075>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;

use crate::global_options;

pub type ParserOptions = global_options::GlobalOptions;

#[derive(Clone, Debug, OcamlRep)]
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
);
