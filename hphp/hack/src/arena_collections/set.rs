// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::{Deserialize, Serialize};

use crate::map::{Map, MapIter};
use arena_trait::{Arena, TrivialDrop};
use ocamlrep::{FromOcamlRepIn, ToOcamlRep};

/// An arena-allocated map.
///
/// See `Map` for more info.
#[derive(Debug, Deserialize, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
#[serde(bound(deserialize = "K: 'de + arena_deserializer::DeserializeInArena<'de>"))]
#[must_use]
pub struct Set<'a, K>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] Map<'a, K, ()>,
);

impl<'a, K> Clone for Set<'a, K> {
    fn clone(&self) -> Self {
        Set(self.0.clone())
    }
}
arena_deserializer::impl_deserialize_in_arena!(Set<'arena, K>);

impl<'a, K> Copy for Set<'a, K> {}

impl<'a, K> TrivialDrop for Set<'a, K> {}

impl<K> Default for Set<'_, K> {
    fn default() -> Self {
        Set(Map::default())
    }
}

impl<K: ToOcamlRep + Ord> ToOcamlRep for Set<'_, K> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&self, alloc: &'a A) -> ocamlrep::OpaqueValue<'a> {
        let len = self.count();
        let mut iter = self.iter();
        let (value, _) = ocamlrep::sorted_iter_to_ocaml_set(&mut iter, alloc, len);
        value
    }
}

impl<'a, K> FromOcamlRepIn<'a> for Set<'a, K>
where
    K: FromOcamlRepIn<'a> + Ord + TrivialDrop + Clone,
{
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        // TODO: This is a bit wasteful. If we had iter_from_ocaml_set_in
        // instead, we wouldn't need the extra vec.
        let mut elements = bumpalo::collections::Vec::new_in(alloc);
        ocamlrep::vec_from_ocaml_set_in(value, &mut elements, alloc)?;
        let mut set = Set::empty();
        for element in elements {
            set = set.add(alloc, element);
        }
        Ok(set)
    }
}

#[macro_export]
macro_rules! set {
  ( ) => ({ Set::empty() });
  ( $arena:expr; $($x:expr),* ) => ({
      let mut temp_map = Set::empty();
      $(
          temp_map = temp_map.add($arena, $x);
      )*
      temp_map
  });
}

impl<'a, K: Ord> Set<'a, K> {
    pub fn mem(self, x: &K) -> bool {
        self.0.mem(x)
    }

    pub fn intersection(self, other: Self) -> Intersection<'a, K> {
        Intersection {
            iter: self.iter(),
            other,
        }
    }
}

impl<'a, K> Set<'a, K> {
    pub const fn empty() -> Self {
        Set(Map::empty())
    }

    pub fn count(self) -> usize {
        self.0.count()
    }
}

impl<'a, K: Ord> Set<'a, K> {
    pub fn is_empty(self) -> bool {
        self.0.is_empty()
    }
}

impl<'a, K: TrivialDrop + Clone + Ord> Set<'a, K> {
    pub fn singleton<A: Arena>(arena: &'a A, x: K) -> Self {
        Set(Map::singleton(arena, x, ()))
    }

    pub fn from<A: Arena, I>(arena: &'a A, i: I) -> Self
    where
        I: IntoIterator<Item = K>,
    {
        let mut s = Self::empty();

        for k in i {
            s = s.add(arena, k);
        }

        return s;
    }

    pub fn add<A: Arena>(self, arena: &'a A, x: K) -> Self {
        Set(self.0.add(arena, x, ()))
    }

    pub fn remove<A: Arena>(self, arena: &'a A, x: &K) -> Self {
        Set(self.0.remove(arena, x))
    }

    pub fn min_entry(self) -> Option<&'a K> {
        let v = self.0.min_entry();
        v.map(|(k, _)| k)
    }

    pub fn remove_min_entry<A: Arena>(self, arena: &'a A) -> Self {
        Set(self.0.remove_min_entry(arena))
    }

    pub fn max_entry(self) -> Option<&'a K> {
        let v = self.0.max_entry();
        v.map(|(k, _)| k)
    }

    /// Remove the maximum key-value entry.
    pub fn remove_max_entry<A: Arena>(self, arena: &'a A) -> Self {
        Set(self.0.remove_max_entry(arena))
    }

    pub fn diff<A: Arena>(self, arena: &'a A, other: Self) -> Self {
        Set(self.0.diff(arena, other.0))
    }
}

impl<'a, T> Set<'a, T> {
    pub fn iter(&self) -> SetIter<'a, T> {
        SetIter {
            iter: self.0.iter(),
        }
    }
}

/// Iterator state for set.
pub struct SetIter<'a, K> {
    iter: MapIter<'a, K, ()>,
}

impl<'a, K> IntoIterator for &Set<'a, K> {
    type Item = &'a K;
    type IntoIter = SetIter<'a, K>;

    fn into_iter(self) -> Self::IntoIter {
        SetIter {
            iter: self.0.into_iter(),
        }
    }
}

impl<'a, K> Iterator for SetIter<'a, K> {
    type Item = &'a K;

    fn next(&mut self) -> Option<Self::Item> {
        match self.iter.next() {
            None => None,
            Some((k, _)) => Some(k),
        }
    }
}

pub struct Intersection<'a, T> {
    iter: SetIter<'a, T>,
    other: Set<'a, T>,
}

impl<'a, T: Ord> Iterator for Intersection<'a, T> {
    type Item = &'a T;

    fn next(&mut self) -> Option<Self::Item> {
        while let Some(x) = self.iter.next() {
            if self.other.mem(x) {
                return Some(x);
            }
        }
        None
    }
}

#[cfg(test)]
pub mod tests_macro {
    use super::*;
    use bumpalo::Bump;

    #[test]
    fn test_empty() {
        assert_eq!(set![], Set::<i32>::empty());
    }

    #[test]
    fn test_non_empty() {
        let a = Bump::new();
        assert_eq!(set![&a; 5, 3, 9, 4], Set::<i32>::from(&a, vec![3, 4, 5, 9]));
    }
}

#[cfg(test)]
pub mod tests_iter {
    use super::*;
    use bumpalo::Bump;

    #[test]
    fn test_empty() {
        let s: Set<i32> = set![];
        assert_eq!(s.into_iter().map(|k| *k).eq(vec![].into_iter()), true);
    }

    #[test]
    fn test_non_empty() {
        let a = Bump::new();
        let s: Set<i32> = set![&a; 6, 4, 5];
        assert_eq!(
            s.into_iter().map(|k| *k).eq(vec![4, 5, 6].into_iter()),
            true
        );
    }
}
