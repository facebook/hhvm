// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlpool_rust::ocamlvalue::*;

#[derive(Clone, Debug)]
pub struct Lazy<T>(T);

impl<T: Into<ocamlrep::Value>> Into<ocamlrep::Value> for Lazy<T> {
    fn into(self) -> ocamlrep::Value {
        ().into()
    }
}

impl<T> Ocamlvalue for Lazy<T> {
    fn ocamlvalue(&self) -> ocaml::core::mlvalues::Value {
        panic!("Not implemented")
    }
}
