// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<12f8d711e40df734ff966642092aefdc>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::IntoOcamlRep;
use ocamlvalue_macro::Ocamlvalue;

#[derive(Clone, Debug, IntoOcamlRep, Ocamlvalue)]
pub enum Prefix {
    Root,
    Hhi,
    Dummy,
    Tmp,
}

#[derive(Clone, Debug, IntoOcamlRep, Ocamlvalue)]
pub struct RelativePath(pub Prefix, pub String);
