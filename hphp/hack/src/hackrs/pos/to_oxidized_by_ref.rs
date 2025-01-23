// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;
use std::collections::HashMap;

use arena_trait::TrivialDrop;
use indexmap::IndexMap;
use indexmap::IndexSet;
use ocamlrep::ToOcamlRep;
use oxidized_by_ref::i_map::IMap;
use oxidized_by_ref::s_map::SMap;
use oxidized_by_ref::s_set::SSet;

pub trait ToOxidizedByRef<'a> {
    type Output: TrivialDrop + Clone + ToOcamlRep + 'a;

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output;
}

impl<'a, T: ToOxidizedByRef<'a>> ToOxidizedByRef<'a> for std::sync::Arc<T> {
    type Output = T::Output;

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        (**self).to_oxidized_by_ref(arena)
    }
}

impl<'a, T: ToOxidizedByRef<'a>> ToOxidizedByRef<'a> for Box<[T]> {
    type Output = &'a [T::Output];

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc_slice_fill_iter(self.iter().map(|x| x.to_oxidized_by_ref(arena)))
    }
}

impl<'a, T: ToOxidizedByRef<'a>> ToOxidizedByRef<'a> for [T] {
    type Output = &'a [T::Output];

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc_slice_fill_iter(self.iter().map(|x| x.to_oxidized_by_ref(arena)))
    }
}

impl<'a> ToOxidizedByRef<'a> for &str {
    type Output = &'a str;

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc_str(self)
    }
}

impl<'a, T1: ToOxidizedByRef<'a>, T2: ToOxidizedByRef<'a>> ToOxidizedByRef<'a> for (T1, T2) {
    type Output = &'a (T1::Output, T2::Output);

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc((
            self.0.to_oxidized_by_ref(arena),
            self.1.to_oxidized_by_ref(arena),
        ))
    }
}

impl<'a, V: ToOxidizedByRef<'a>> ToOxidizedByRef<'a> for Option<V> {
    type Output = Option<V::Output>;

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        self.as_ref().map(|x| x.to_oxidized_by_ref(arena))
    }
}

impl<'a, K: ToOxidizedByRef<'a, Output = &'a str>, V: ToOxidizedByRef<'a>> ToOxidizedByRef<'a>
    for BTreeMap<K, V>
{
    type Output = SMap<'a, V::Output>;

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        SMap::from(
            arena,
            self.iter()
                .map(|(k, v)| (k.to_oxidized_by_ref(arena), v.to_oxidized_by_ref(arena))),
        )
    }
}

impl<'a, T: ToOxidizedByRef<'a, Output = &'a str>, S> ToOxidizedByRef<'a> for IndexSet<T, S> {
    type Output = SSet<'a>;

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        SSet::from(arena, self.iter().map(|s| s.to_oxidized_by_ref(arena)))
    }
}

impl<'a, K: ToOxidizedByRef<'a, Output = &'a str>, V: ToOxidizedByRef<'a>, S> ToOxidizedByRef<'a>
    for IndexMap<K, V, S>
{
    type Output = SMap<'a, V::Output>;

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        SMap::from(
            arena,
            self.iter()
                .map(|(k, v)| (k.to_oxidized_by_ref(arena), v.to_oxidized_by_ref(arena))),
        )
    }
}

impl<'a, K: ToOxidizedByRef<'a, Output = &'a str>, V: ToOxidizedByRef<'a>, S> ToOxidizedByRef<'a>
    for HashMap<K, V, S>
{
    type Output = SMap<'a, V::Output>;

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        SMap::from(
            arena,
            self.iter()
                .map(|(k, v)| (k.to_oxidized_by_ref(arena), v.to_oxidized_by_ref(arena))),
        )
    }
}

impl<'a, K: ToOxidizedByRef<'a, Output = isize>, V: ToOxidizedByRef<'a>> ToOxidizedByRef<'a>
    for &im::HashMap<K, V>
{
    type Output = IMap<'a, &'a V::Output>;

    fn to_oxidized_by_ref(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        IMap::from(
            arena,
            self.iter().map(|(k, v)| {
                (
                    k.to_oxidized_by_ref(arena),
                    &*arena.alloc(v.to_oxidized_by_ref(arena)),
                )
            }),
        )
    }
}
