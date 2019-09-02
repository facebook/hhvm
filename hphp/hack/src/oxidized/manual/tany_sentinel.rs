// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::IntoOcamlRep;

#[derive(Clone, Debug, IntoOcamlRep)]
pub struct TanySentinel;
