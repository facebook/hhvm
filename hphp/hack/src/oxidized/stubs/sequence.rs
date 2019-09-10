// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::OcamlRep;

#[derive(Debug)]
pub struct Sequence<T>(T);

impl<T: OcamlRep> OcamlRep for Sequence<T> {
    fn into_ocamlrep<'a>(self, arena: &ocamlrep::Arena<'a>) -> ocamlrep::Value<'a> {
        ().into_ocamlrep(arena)
    }
}
