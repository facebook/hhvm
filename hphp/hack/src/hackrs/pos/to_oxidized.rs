// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use indexmap::IndexMap;
use indexmap::IndexSet;
use ocamlrep::ToOcamlRep;
use oxidized::s_map::SMap;
use oxidized::s_set::SSet;

pub trait ToOxidized {
    type Output: Clone + ToOcamlRep;

    fn to_oxidized(self) -> Self::Output;
}

impl<T: ToOxidized> ToOxidized for Box<[T]> {
    type Output = Vec<T::Output>;

    fn to_oxidized(self) -> Self::Output {
        self.into_vec()
            .into_iter()
            .map(|x: T| x.to_oxidized())
            .collect()
    }
}

impl<T1: ToOxidized, T2: ToOxidized> ToOxidized for (T1, T2) {
    type Output = (T1::Output, T2::Output);

    fn to_oxidized(self) -> Self::Output {
        (self.0.to_oxidized(), self.1.to_oxidized())
    }
}

impl<V: ToOxidized> ToOxidized for Option<V> {
    type Output = Option<V::Output>;

    fn to_oxidized(self) -> Self::Output {
        self.map(|x| x.to_oxidized())
    }
}

impl<K: ToOxidized<Output = String>, V: ToOxidized> ToOxidized for BTreeMap<K, V> {
    type Output = SMap<V::Output>;

    fn to_oxidized(self) -> Self::Output {
        let mut m = SMap::new();
        for (k, v) in self.into_iter() {
            m.insert(k.to_oxidized(), v.to_oxidized());
        }
        m
    }
}

impl<T: ToOxidized<Output = String>, S> ToOxidized for IndexSet<T, S> {
    type Output = SSet;

    fn to_oxidized(self) -> Self::Output {
        let mut m = SSet::new();
        for s in self.into_iter() {
            m.insert(s.to_oxidized());
        }
        m
    }
}

impl<K: ToOxidized<Output = String>, V: ToOxidized, S> ToOxidized for IndexMap<K, V, S> {
    type Output = SMap<V::Output>;

    fn to_oxidized(self) -> Self::Output {
        let mut m = SMap::new();
        for (k, v) in self.into_iter() {
            m.insert(k.to_oxidized(), v.to_oxidized());
        }
        m
    }
}
