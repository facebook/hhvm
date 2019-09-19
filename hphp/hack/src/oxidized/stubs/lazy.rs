// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlpool_rust::ocamlvalue::*;
use ocamlrep::OcamlRep;

#[derive(Clone, Debug)]
pub struct Lazy<T>(T);

impl<T: OcamlRep> OcamlRep for Lazy<T> {
    fn into_ocamlrep<'a>(self, _arena: &ocamlrep::Arena<'a>) -> ocamlrep::Value<'a> {
        unimplemented!()
    }

    fn from_ocamlrep(_value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        unimplemented!()
    }
}

impl<T> Ocamlvalue for Lazy<T> {
    fn ocamlvalue(&self) -> ocaml::core::mlvalues::Value {
        unimplemented!()
    }
}
