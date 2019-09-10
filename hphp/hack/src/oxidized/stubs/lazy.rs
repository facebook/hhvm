// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlpool_rust::ocamlvalue::*;
use ocamlrep::OcamlRep;

#[derive(Clone, Debug)]
pub struct Lazy<T>(T);

impl<T: OcamlRep> OcamlRep for Lazy<T> {
    fn into_ocamlrep<'a>(self, arena: &ocamlrep::Arena<'a>) -> ocamlrep::Value<'a> {
        ().into_ocamlrep(arena)
    }
}

impl<T> Ocamlvalue for Lazy<T> {
    fn ocamlvalue(&self) -> ocaml::core::mlvalues::Value {
        panic!("Not implemented")
    }
}
