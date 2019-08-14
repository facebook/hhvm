// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<82d073c5836b1a6fec5a01f877ff8269>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::IntoOcamlRep;

#[derive(Clone, Debug, IntoOcamlRep)]
pub enum Prefix {
    Root,
    Hhi,
    Dummy,
    Tmp,
}

#[derive(Clone, Debug, IntoOcamlRep)]
pub struct RelativePath(pub Prefix, pub String);
