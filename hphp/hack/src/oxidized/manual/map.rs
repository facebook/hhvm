// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ord;

use ocamlrep_derive::OcamlRep;
use ocamlvalue_macro::Ocamlvalue;

#[allow(dead_code)]
#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
enum MapImpl<K: Ord, V> {
    Empty,
    Node {
        l: Box<MapImpl<K, V>>,
        v: K,
        d: V,
        r: Box<MapImpl<K, V>>,
        h: usize,
    },
}

use MapImpl::*;

#[derive(Clone, Debug, OcamlRep, Ocamlvalue)]
pub struct Map<K: Ord, V>(MapImpl<K, V>);

impl<K: Ord, V> Map<K, V> {
    pub fn empty() -> Self {
        Map(Empty)
    }
}
