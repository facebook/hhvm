// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::OcamlRep;

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum InferMissing {
    Deactivated,
    InferReturn,
    InferParams,
}
