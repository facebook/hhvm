// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Array-backed set types.
//!
//! At the moment, we are using the bumpalo allocator for arena allocation.
//! Because the stdlib types do not yet provide the ability to choose the
//! allocator used when they are allocated or resized, the bumpalo library
//! provides its own growable Vec and String types. Since bumpalo does not
//! provide its own map or set types, we must define our own if we want to
//! control where they are allocated.
//!
//! This module defines set types backed by bumpalo's Vec. It is useful for sets
//! which are built all at once, and never modified thereafter (e.g., sets in
//! ASTs). When immutable semantics are desired, but updating is necessary,
//! consider the `arena_collections::set` submodule instead, for a set type
//! backed by an immutable balanced binary tree. The Vec-backed sets in this
//! module may benefit from better cache efficiency, and so may outperform the
//! balanced tree implementation in some circumstances.

use std::borrow::Borrow;
use std::fmt::Debug;

use arena_trait::TrivialDrop;
use bumpalo::Bump;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Serialize;

use crate::AssocList;
use crate::AssocListMut;
use crate::SortedAssocList;

/// A readonly array-based multiset.
///
/// * Lookups run in linear time
/// * Duplicate elements are permitted
#[derive(Copy, Clone, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub struct MultiSet<'a, T> {
    list: AssocList<'a, T, ()>,
}

impl<'a, T: 'a> MultiSet<'a, T> {
    /// Returns `true` if the set contains a value.
    ///
    /// The value may be any borrowed form of the set's value type,
    /// but the ordering on the borrowed form *must* match the
    /// ordering on the value type.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSet;
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// set.insert(1);
    /// let set = MultiSet::from(set);
    /// assert!(set.contains(&1));
    /// ```
    pub fn contains<Q: ?Sized>(&self, value: &Q) -> bool
    where
        T: Borrow<Q>,
        Q: Ord,
    {
        self.list.contains_key(value)
    }

    /// Get an iterator over the elements of the set.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSet;
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// set.insert(3);
    /// set.insert(1);
    /// set.insert(2);
    /// let set = MultiSet::from(set);
    /// let mut set_iter = set.iter();
    /// assert_eq!(set_iter.next(), Some(&3));
    /// assert_eq!(set_iter.next(), Some(&1));
    /// assert_eq!(set_iter.next(), Some(&2));
    /// assert_eq!(set_iter.next(), None);
    /// ```
    pub fn iter(&self) -> impl Iterator<Item = &T> {
        self.list.keys()
    }

    /// Returns the number of elements in the set. Duplicate elements are
    /// counted.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSet;
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// set.insert(1);
    /// set.insert(1);
    /// let set = MultiSet::from(set);
    /// assert_eq!(set.len(), 2);
    /// ```
    pub fn len(&self) -> usize {
        self.list.len()
    }

    /// Returns `true` if the set contains no elements.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSet;
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// let set = MultiSet::from(set);
    /// assert_eq!(set.is_empty(), true);
    /// let mut set = MultiSetMut::new_in(&b);
    /// set.insert(1);
    /// let set = MultiSet::from(set);
    /// assert_eq!(set.is_empty(), false);
    /// ```
    pub fn is_empty(&self) -> bool {
        self.list.is_empty()
    }

    /// Make a new `MultiSet` containing the values in the given slice.
    ///
    /// Provided for the sake of creating empty const sets. Passing non-empty
    /// slices is not recommended.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocList;
    /// use arena_collections::MultiSet;
    ///
    /// const EMPTY_MULTISET: MultiSet<'_, i32> = MultiSet::from_slice(&[]);
    /// assert!(EMPTY_MULTISET.is_empty());
    /// ```
    pub const fn from_slice(slice: &'a [(T, ())]) -> Self {
        Self {
            list: AssocList::new(slice),
        }
    }
}

impl<T> TrivialDrop for MultiSet<'_, T> {}

impl<T: Debug> Debug for MultiSet<'_, T> {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fmt.debug_set().entries(self.iter()).finish()
    }
}

impl<'a, T> From<MultiSetMut<'a, T>> for MultiSet<'a, T> {
    #[inline]
    fn from(set: MultiSetMut<'a, T>) -> Self {
        MultiSet {
            list: set.list.into(),
        }
    }
}

/// A mutable array-based multiset, allocated in a given arena.
///
/// * Lookups and removals run in linear time
/// * Insertions run in constant time
/// * Duplicate elements are permitted
#[derive(Clone, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub struct MultiSetMut<'bump, T> {
    list: AssocListMut<'bump, T, ()>,
}

impl<'bump, T> MultiSetMut<'bump, T> {
    /// Constructs a new, empty `MultiSetMut`.
    ///
    /// The set will not allocate until an element is inserted.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set: MultiSetMut<i32> = MultiSetMut::new_in(&b);
    /// ```
    #[inline]
    pub fn new_in(bump: &'bump Bump) -> Self {
        MultiSetMut {
            list: AssocListMut::new_in(bump),
        }
    }

    /// Constructs a new, empty `MultiSetMut` with the specified capacity.
    ///
    /// The set will be able to hold exactly `capacity` elements without
    /// reallocating. If `capacity` is 0, the set will not allocate.
    ///
    /// It is important to note that although the returned set has the
    /// *capacity* specified, the set will have a zero *length*.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    ///
    /// let mut set = MultiSetMut::with_capacity_in(10, &b);
    ///
    /// // The set contains no items, even though it has capacity for more
    /// assert_eq!(set.len(), 0);
    ///
    /// // These are all done without reallocating...
    /// for i in 0..10 {
    ///     set.insert(i);
    /// }
    ///
    /// // ...but this may make the set reallocate
    /// set.insert(11);
    /// ```
    #[inline]
    pub fn with_capacity_in(capacity: usize, bump: &'bump Bump) -> Self {
        MultiSetMut {
            list: AssocListMut::with_capacity_in(capacity, bump),
        }
    }

    /// Returns `true` if the set contains a value.
    ///
    /// The value may be any borrowed form of the set's value type,
    /// but the ordering on the borrowed form *must* match the
    /// ordering on the value type.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// set.insert(1);
    /// assert!(set.contains(&1));
    /// ```
    pub fn contains<Q: ?Sized>(&self, value: &Q) -> bool
    where
        T: Borrow<Q>,
        Q: Ord,
    {
        self.list.contains_key(value)
    }

    /// Add a value to the set.
    ///
    /// The value is added even if the set already contains the given value.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// assert_eq!(set.contains(&1), false);
    /// set.insert(1);
    /// assert_eq!(set.contains(&1), true);
    /// ```
    pub fn insert(&mut self, value: T) {
        self.list.insert(value, ());
    }

    /// Removes a value from the set. Returns true if the value was present in
    /// the set.
    ///
    /// If the set contains multiple values equal to the given value, only the
    /// most recently inserted is removed.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    ///
    /// set.insert(2);
    /// assert_eq!(set.remove(&2), true);
    /// assert_eq!(set.remove(&2), false);
    /// ```
    pub fn remove<Q: ?Sized>(&mut self, value: &Q) -> bool
    where
        T: Borrow<Q>,
        Q: Ord,
    {
        self.list.remove(value).is_some()
    }

    /// Removes all values equal to the given value from the set. Returns true
    /// if any values were removed.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    ///
    /// set.insert(2);
    /// set.insert(2);
    /// assert_eq!(set.remove_all(&2), true);
    /// assert_eq!(set.remove_all(&2), false);
    /// ```
    pub fn remove_all<Q: ?Sized>(&mut self, value: &Q) -> bool
    where
        T: Borrow<Q>,
        Q: Ord,
    {
        self.list.remove_all(value)
    }

    /// Get an iterator over the elements of the set.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSet;
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// set.insert(3);
    /// set.insert(1);
    /// set.insert(2);
    /// let mut set_iter = set.iter();
    /// assert_eq!(set_iter.next(), Some(&3));
    /// assert_eq!(set_iter.next(), Some(&1));
    /// assert_eq!(set_iter.next(), Some(&2));
    /// assert_eq!(set_iter.next(), None);
    /// ```
    pub fn iter(&self) -> impl Iterator<Item = &T> {
        self.list.keys()
    }

    /// Returns the number of elements in the set. Duplicate elements are
    /// counted.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSet;
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// assert_eq!(set.len(), 0);
    /// set.insert(1);
    /// assert_eq!(set.len(), 1);
    /// set.insert(1);
    /// assert_eq!(set.len(), 2);
    /// set.insert(2);
    /// assert_eq!(set.len(), 3);
    /// ```
    pub fn len(&self) -> usize {
        self.list.len()
    }

    /// Returns `true` if the set contains no elements.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSet;
    /// use arena_collections::MultiSetMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// let set = MultiSet::from(set);
    /// assert_eq!(set.is_empty(), true);
    /// let mut set = MultiSetMut::new_in(&b);
    /// set.insert(1);
    /// let set = MultiSet::from(set);
    /// assert_eq!(set.is_empty(), false);
    /// ```
    pub fn is_empty(&self) -> bool {
        self.list.is_empty()
    }
}

impl<T: Debug> Debug for MultiSetMut<'_, T> {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fmt.debug_set().entries(self.iter()).finish()
    }
}

/// A readonly array-based set.
///
/// * Lookups run in log(n) time
/// * Duplicate elements are not permitted. When constructing a `SortedSet` from
///   a `MultiSetMut`, elements will be deduplicated.
#[derive(Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub struct SortedSet<'a, T> {
    list: SortedAssocList<'a, T, ()>,
}

impl<T> TrivialDrop for SortedSet<'_, T> {}
impl<T> Copy for SortedSet<'_, T> {}
impl<T> Clone for SortedSet<'_, T> {
    fn clone(&self) -> Self {
        *self
    }
}

impl<T> Default for SortedSet<'_, T> {
    fn default() -> Self {
        Self::from_slice(&[])
    }
}

impl<'a, T> SortedSet<'a, T> {
    /// Returns the empty set
    pub fn empty() -> Self {
        SortedSet {
            list: SortedAssocList::empty(),
        }
    }

    /// Returns `true` if the set contains a value.
    ///
    /// The value may be any borrowed form of the set's value type,
    /// but the ordering on the borrowed form *must* match the
    /// ordering on the value type.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSetMut;
    /// use arena_collections::SortedSet;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// set.insert(1);
    /// let set = SortedSet::from(set);
    /// assert!(set.contains(&1));
    /// ```
    pub fn contains<Q: ?Sized>(&self, value: &Q) -> bool
    where
        T: Borrow<Q>,
        Q: Ord,
    {
        self.list.contains_key(value)
    }

    /// Get an iterator over the elements of the set in ascending order.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSetMut;
    /// use arena_collections::SortedSet;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// set.insert(3);
    /// set.insert(1);
    /// set.insert(2);
    /// let set = SortedSet::from(set);
    /// let mut set_iter = set.iter();
    /// assert_eq!(set_iter.next(), Some(&1));
    /// assert_eq!(set_iter.next(), Some(&2));
    /// assert_eq!(set_iter.next(), Some(&3));
    /// assert_eq!(set_iter.next(), None);
    /// ```
    pub fn iter(&self) -> impl Iterator<Item = &T> {
        self.list.keys()
    }

    /// Returns the number of elements in the set.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSetMut;
    /// use arena_collections::SortedSet;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// set.insert(1);
    /// set.insert(2);
    /// set.insert(1);
    /// let set = SortedSet::from(set);
    /// assert_eq!(set.len(), 2);
    /// ```
    pub fn len(&self) -> usize {
        self.list.len()
    }

    /// Returns `true` if the set contains no elements.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::MultiSetMut;
    /// use arena_collections::SortedSet;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut set = MultiSetMut::new_in(&b);
    /// let set = SortedSet::from(set);
    /// assert_eq!(set.is_empty(), true);
    /// let mut set = MultiSetMut::new_in(&b);
    /// set.insert(1);
    /// let set = SortedSet::from(set);
    /// assert_eq!(set.is_empty(), false);
    /// ```
    pub fn is_empty(&self) -> bool {
        self.list.is_empty()
    }

    /// Make a new `SortedSet` containing the values in the given slice.
    ///
    /// Provided for the sake of creating empty const sets. Passing non-empty
    /// slices is not recommended.
    ///
    /// The values in the slice must be in ascending sorted order (by `T`'s
    /// implementation of `Ord`). There must be no duplicate values in the
    /// slice.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::SortedSet;
    ///
    /// const EMPTY_SORTED_SET: SortedSet<'_, i32> = SortedSet::from_slice(&[]);
    /// assert!(EMPTY_SORTED_SET.is_empty());
    /// ```
    pub const fn from_slice(list: &'a [(T, ())]) -> Self {
        Self {
            list: SortedAssocList::from_slice(list),
        }
    }
}

impl<T: Debug> Debug for SortedSet<'_, T> {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fmt.debug_set().entries(self.iter()).finish()
    }
}

impl<'a, T: Ord> From<MultiSetMut<'a, T>> for SortedSet<'a, T> {
    #[inline]
    fn from(set: MultiSetMut<'a, T>) -> Self {
        SortedSet {
            list: set.list.into(),
        }
    }
}

impl<T: ToOcamlRep + Ord> ToOcamlRep for SortedSet<'_, T> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        let len = self.len();
        let mut iter = self.iter().map(|x| x.to_ocamlrep(alloc));
        let (value, _) = ocamlrep::sorted_iter_to_ocaml_set(&mut iter, alloc, len);
        value
    }
}

impl<'a, T: FromOcamlRepIn<'a> + Ord> FromOcamlRepIn<'a> for SortedSet<'a, T> {
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        let mut list = bumpalo::collections::Vec::new_in(alloc);
        ocamlrep::vec_from_ocaml_set_in(value, &mut list, alloc)?;
        let list = SortedAssocList::from_slice(list.into_bump_slice());
        Ok(Self { list })
    }
}
