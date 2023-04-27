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

pub trait ToOxidized<'a> {
    type Output: TrivialDrop + Clone + ToOcamlRep + 'a;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output;

    fn to_oxidized_ref(&self, arena: &'a bumpalo::Bump) -> &'a Self::Output {
        &*arena.alloc(self.to_oxidized(arena))
    }
}

impl<'a, T: ToOxidized<'a>> ToOxidized<'a> for std::sync::Arc<T> {
    type Output = T::Output;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        (**self).to_oxidized(arena)
    }
}

impl<'a, T: ToOxidized<'a>> ToOxidized<'a> for Box<[T]> {
    type Output = &'a [T::Output];

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc_slice_fill_iter(self.iter().map(|x| x.to_oxidized(arena)))
    }
}

impl<'a, T: ToOxidized<'a>> ToOxidized<'a> for [T] {
    type Output = &'a [T::Output];

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc_slice_fill_iter(self.iter().map(|x| x.to_oxidized(arena)))
    }
}

impl<'a> ToOxidized<'a> for &str {
    type Output = &'a str;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc_str(self)
    }
}

impl<'a, T1: ToOxidized<'a>, T2: ToOxidized<'a>> ToOxidized<'a> for (T1, T2) {
    type Output = &'a (T1::Output, T2::Output);

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc((self.0.to_oxidized(arena), self.1.to_oxidized(arena)))
    }
}

impl<'a, V: ToOxidized<'a>> ToOxidized<'a> for Option<V> {
    type Output = Option<V::Output>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        self.as_ref().map(|x| x.to_oxidized(arena))
    }
}

impl<'a, K: ToOxidized<'a, Output = &'a str>, V: ToOxidized<'a>> ToOxidized<'a> for BTreeMap<K, V> {
    type Output = SMap<'a, V::Output>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        SMap::from(
            arena,
            self.iter()
                .map(|(k, v)| (k.to_oxidized(arena), v.to_oxidized(arena))),
        )
    }
}

impl<'a, T: ToOxidized<'a, Output = &'a str>, S> ToOxidized<'a> for IndexSet<T, S> {
    type Output = SSet<'a>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        SSet::from(arena, self.iter().map(|s| s.to_oxidized(arena)))
    }
}

impl<'a, K: ToOxidized<'a, Output = &'a str>, V: ToOxidized<'a>, S> ToOxidized<'a>
    for IndexMap<K, V, S>
{
    type Output = SMap<'a, V::Output>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        SMap::from(
            arena,
            self.iter()
                .map(|(k, v)| (k.to_oxidized(arena), v.to_oxidized(arena))),
        )
    }
}

impl<'a, K: ToOxidized<'a, Output = &'a str>, V: ToOxidized<'a>, S> ToOxidized<'a>
    for HashMap<K, V, S>
{
    type Output = SMap<'a, V::Output>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        SMap::from(
            arena,
            self.iter()
                .map(|(k, v)| (k.to_oxidized(arena), v.to_oxidized(arena))),
        )
    }
}

impl<'a, K: ToOxidized<'a, Output = isize>, V: ToOxidized<'a>> ToOxidized<'a>
    for &im::HashMap<K, V>
{
    type Output = IMap<'a, &'a V::Output>;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        IMap::from(
            arena,
            self.iter()
                .map(|(k, v)| (k.to_oxidized(arena), &*arena.alloc(v.to_oxidized(arena)))),
        )
    }
}
