// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ee85dc8fa2a5a393660689c5acfcf8ca>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum InferMissing {
    Deactivated,
    InferReturn,
    InferParams,
    InferGlobal,
}
