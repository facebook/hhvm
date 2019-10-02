// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<8a2fdfab68ab9c4afde11a2b85a88152>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use ocamlvalue_macro::Ocamlvalue;

use crate::global_options;

pub type ParserOptions = global_options::GlobalOptions;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
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
);
