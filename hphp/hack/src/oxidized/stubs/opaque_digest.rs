// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::OcamlRep;
use ocamlvalue_macro::Ocamlvalue;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct OpaqueDigest(());
