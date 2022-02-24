// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_trait::TrivialDrop;
use ocamlrep::ToOcamlRep;

pub trait ToOxidized<'a> {
    type Output: TrivialDrop + Clone + ToOcamlRep + 'a;

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output;
}

impl<'a, T: ToOxidized<'a>> ToOxidized<'a> for Box<[T]> {
    type Output = &'a [T::Output];

    fn to_oxidized(&self, arena: &'a bumpalo::Bump) -> Self::Output {
        arena.alloc_slice_fill_iter(self.iter().map(|x| x.to_oxidized(arena)))
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
