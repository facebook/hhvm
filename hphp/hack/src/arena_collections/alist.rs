// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Associative array types.
//!
//! At the moment, we are using the bumpalo allocator for arena allocation.
//! Because the stdlib types do not yet provide the ability to choose the
//! allocator used when they are allocated or resized, the bumpalo library
//! provides its own growable Vec and String types. Since bumpalo does not
//! provide its own map or set types, we must define our own if we want to
//! control where they are allocated.
//!
//! This module defines map types backed by bumpalo's Vec. It is useful for maps
//! which are built all at once, and never modified thereafter (e.g., maps in
//! ASTs). When immutable semantics are desired, but updating is necessary,
//! consider the `arena_collections::map` submodule instead, for an immutable
//! balanced binary tree. The Vec-backed maps in this module may benefit from
//! better cache efficiency, and so may outperform the balanced tree
//! implementation in some circumstances.

use std::borrow::Borrow;
use std::fmt::Debug;

use arena_trait::TrivialDrop;
use bumpalo::Bump;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

/// Perform a linear search for the last entry in the slice with the given key.
#[inline(always)]
fn get_last_entry<'a, K, V, Q: ?Sized>(entries: &'a [(K, V)], key: &Q) -> Option<&'a (K, V)>
where
    K: Borrow<Q>,
    Q: PartialEq,
{
    entries.iter().rev().find(|(k, _)| key == k.borrow())
}

/// Perform a linear search for the last entry in the slice with the given key
/// and return its index in the slice.
#[inline(always)]
fn get_last_index<'a, K, V, Q: ?Sized>(entries: &'a [(K, V)], key: &Q) -> Option<usize>
where
    K: Borrow<Q>,
    Q: PartialEq,
{
    entries
        .iter()
        .enumerate()
        .rev()
        .find(|(_, (k, _))| key == k.borrow())
        .map(|(idx, _)| idx)
}

/// A readonly associative array.
///
/// * Lookups run in linear time
/// * Entries with duplicate keys are permitted (but only observable using iterators)
#[derive(Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub struct AssocList<'a, K, V> {
    entries: &'a [(K, V)],
}

impl<'a, K, V> AssocList<'a, K, V> {
    /// Make a new `AssocList` containing the given key-value pairs.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocList;
    ///
    /// let entries = [(1, "a")];
    /// let alist = AssocList::new(&entries[..]);
    /// ```
    #[inline]
    pub const fn new(entries: &'a [(K, V)]) -> Self {
        Self { entries }
    }

    /// Returns a reference to the value corresponding to the key.
    ///
    /// The key may be any borrowed form of the list's key type, but the
    /// ordering on the borrowed form *must* match the ordering on the key type.
    ///
    /// If multiple entries in the map have keys equal to the given key, the
    /// value corresponding to the last entry (the one with the larger index in
    /// the slice passed to `AssocList::new`) will be returned.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocList;
    ///
    /// let entries = [(1, "a")];
    /// let alist = AssocList::new(&entries[..]);
    /// assert_eq!(alist.get(&1), Some(&"a"));
    /// assert_eq!(alist.get(&2), None);
    /// ```
    pub fn get<Q: ?Sized>(&self, key: &Q) -> Option<&V>
    where
        K: Borrow<Q>,
        Q: PartialEq,
    {
        get_last_entry(self.entries, key).map(|(_, v)| v)
    }

    /// Returns the key-value pair corresponding to the supplied key.
    ///
    /// The key may be any borrowed form of the list's key type, but the
    /// ordering on the borrowed form *must* match the ordering on the key type.
    ///
    /// If multiple entries in the map have keys equal to the given key, the
    /// last entry (the one with the larger index in the slice passed to
    /// `AssocList::new`) will be returned.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocList;
    ///
    /// let entries = [(1, "a")];
    /// let alist = AssocList::new(&entries[..]);
    /// assert_eq!(alist.get_key_value(&1), Some((&1, &"a")));
    /// assert_eq!(alist.get_key_value(&2), None);
    /// ```
    pub fn get_key_value<Q: ?Sized>(&self, key: &Q) -> Option<(&K, &V)>
    where
        K: Borrow<Q>,
        Q: PartialEq,
    {
        get_last_entry(self.entries, key).map(|(k, v)| (k, v))
    }

    /// Returns `true` if the list contains a value for the specified key.
    ///
    /// The key may be any borrowed form of the list's key type, but the
    /// ordering on the borrowed form *must* match the ordering on the key type.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocList;
    ///
    /// let entries = [(1, "a")];
    /// let alist = AssocList::new(&entries[..]);
    /// assert_eq!(alist.contains_key(&1), true);
    /// assert_eq!(alist.contains_key(&2), false);
    /// ```
    pub fn contains_key<Q: ?Sized>(&self, key: &Q) -> bool
    where
        K: Borrow<Q>,
        Q: PartialEq,
    {
        get_last_entry(self.entries, key).is_some()
    }

    /// Gets an iterator over the entries of the association list.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocList;
    ///
    /// let entries = [(1, "a"), (2, "b")];
    /// let alist = AssocList::new(&entries[..]);
    ///
    /// for (key, value) in alist.iter() {
    ///     println!("{}: {}", key, value);
    /// }
    ///
    /// let (first_key, first_value) = alist.iter().next().unwrap();
    /// assert_eq!((*first_key, *first_value), (1, "a"));
    /// ```
    pub fn iter(&self) -> impl Iterator<Item = (&K, &V)> {
        self.entries.iter().map(|(k, v)| (k, v))
    }

    /// Gets an iterator over the keys of the association list.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocList;
    ///
    /// let entries = [(1, "a"), (2, "b")];
    /// let alist = AssocList::new(&entries[..]);
    ///
    /// let keys: Vec<_> = alist.keys().copied().collect();
    /// assert_eq!(keys, [1, 2]);
    /// ```
    pub fn keys(&self) -> impl Iterator<Item = &K> {
        self.entries.iter().map(|(k, _)| k)
    }

    /// Gets an iterator over the values of the association list.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocList;
    ///
    /// let entries = [(1, "hello"), (2, "goodbye")];
    /// let alist = AssocList::new(&entries[..]);
    ///
    /// let values: Vec<&str> = alist.values().copied().collect();
    /// assert_eq!(values, ["hello", "goodbye"]);
    /// ```
    pub fn values(&self) -> impl Iterator<Item = &V> {
        self.entries.iter().map(|(_, v)| v)
    }

    /// Returns the number of entries in the list.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocList;
    ///
    /// let entries = [(1, "a")];
    /// let alist = AssocList::new(&entries[0..entries]);
    /// assert_eq!(alist.len(), 0);
    /// let alist = AssocList::new(&entries[0..1]);
    /// assert_eq!(alist.len(), 1);
    /// ```
    pub fn len(&self) -> usize {
        self.entries.len()
    }

    /// Returns `true` if the list contains no entries.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocList;
    ///
    /// let entries = [(1, "a")];
    /// let alist = AssocList::new(&entries[0..entries]);
    /// assert!(alist.is_empty());
    /// let alist = AssocList::new(&entries[0..1]);
    /// assert!(!alist.is_empty());
    /// ```
    pub fn is_empty(&self) -> bool {
        self.entries.is_empty()
    }
}

impl<K, V> TrivialDrop for AssocList<'_, K, V> {}
impl<K, V> Copy for AssocList<'_, K, V> {}
impl<K, V> Clone for AssocList<'_, K, V> {
    fn clone(&self) -> Self {
        *self
    }
}

impl<K: Debug, V: Debug> Debug for AssocList<'_, K, V> {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fmt.debug_map().entries(self.iter()).finish()
    }
}

impl<'a, K, V> From<AssocListMut<'a, K, V>> for AssocList<'a, K, V> {
    #[inline]
    fn from(alist: AssocListMut<'a, K, V>) -> Self {
        AssocList::new(alist.entries.into_bump_slice())
    }
}

/// A mutable associative array, allocated in a given arena.
///
/// * Lookups, replacements, and removals run in linear time
/// * Insertions run in constant time
/// * Entries with duplicate keys are permitted (but only observable using iterators)
#[derive(Clone, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub struct AssocListMut<'bump, K, V> {
    entries: bumpalo::collections::Vec<'bump, (K, V)>,
}

impl<'bump, K, V> AssocListMut<'bump, K, V> {
    /// Constructs a new, empty `AssocListMut`.
    ///
    /// The list will not allocate until entries are inserted.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist: AssocListMut<i32> = AssocListMut::new_in(&b);
    /// ```
    #[inline]
    pub fn new_in(bump: &'bump Bump) -> Self {
        AssocListMut {
            entries: bumpalo::collections::Vec::new_in(bump),
        }
    }

    /// Constructs a new, empty `AssocListMut` with the specified capacity.
    ///
    /// The list will be able to hold exactly `capacity` elements without
    /// reallocating. If `capacity` is 0, the list will not allocate.
    ///
    /// It is important to note that although the returned list has the
    /// *capacity* specified, the list will have a zero *length*.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    ///
    /// let mut alist = AssocListMut::with_capacity_in(10, &b);
    ///
    /// // The list contains no items, even though it has capacity for more
    /// assert_eq!(alist.len(), 0);
    ///
    /// // These are all done without reallocating...
    /// for i in 0..10 {
    ///     alist.insert(i, i);
    /// }
    ///
    /// // ...but this may make the list reallocate
    /// alist.insert(11, 11);
    /// ```
    #[inline]
    pub fn with_capacity_in(capacity: usize, bump: &'bump Bump) -> Self {
        AssocListMut {
            entries: bumpalo::collections::Vec::with_capacity_in(capacity, bump),
        }
    }

    /// Insert the given key-value pair into the association list.
    ///
    /// If an entry with the given key already exists in the list, it will
    /// remain there, but future calls to `AssocListMut::get` will return the
    /// newly-inserted value instead of the extant one.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// assert_eq!(alist.get(&1), Some(&"a"));
    /// alist.insert(1, "b");
    /// assert_eq!(alist.get(&1), Some(&"b"));
    /// assert_eq!(alist.len(), 2);
    /// ```
    #[inline]
    pub fn insert(&mut self, key: K, value: V) {
        self.entries.push((key, value))
    }

    /// Insert the given key-value pair into the association list.
    ///
    /// If one entry with the given key already exists in the list, it will be
    /// removed, and its value will be returned. If multiple entries with the
    /// given key already exist in the list, only the most recently inserted one
    /// will be removed.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert_or_replace(1, "a");
    /// assert_eq!(alist.get(&1), Some(&"a"));
    /// alist.insert_or_replace(1, "b");
    /// assert_eq!(alist.get(&1), Some(&"b"));
    /// assert_eq!(alist.len(), 1);
    /// ```
    pub fn insert_or_replace(&mut self, key: K, value: V) -> Option<V>
    where
        K: PartialEq,
    {
        match get_last_index(&self.entries, &key) {
            None => {
                self.insert(key, value);
                None
            }
            Some(idx) => {
                let mut entry = (key, value);
                std::mem::swap(&mut self.entries[idx], &mut entry);
                Some(entry.1)
            }
        }
    }

    /// Remove a key from the list, returning the value at the key if the key
    /// was previously in the list.
    ///
    /// The key may be any borrowed form of the list's key type, but the
    /// ordering on the borrowed form *must* match the ordering on the key type.
    ///
    /// If multiple entries with the given key exist in the list, only the last
    /// will be removed (the one which would b returned by
    /// `AssocListMut::get`).
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// alist.insert(1, "b");
    /// assert_eq!(alist.get(&1), Some(&"b"));
    /// alist.remove(&1);
    /// assert_eq!(alist.get(&1), Some(&"a"));
    /// alist.remove(&1);
    /// assert_eq!(alist.get(&1), None);
    /// ```
    pub fn remove<Q: ?Sized>(&mut self, key: &Q) -> Option<V>
    where
        K: Borrow<Q>,
        Q: PartialEq,
    {
        match get_last_index(&self.entries, key) {
            None => None,
            Some(idx) => Some(self.entries.remove(idx).1),
        }
    }

    /// Removes all entries with the given key from the list. Returns true if
    /// any entries were removed.
    ///
    /// The key may be any borrowed form of the list's key type, but the
    /// ordering on the borrowed form *must* match the ordering on the key type.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// alist.insert(1, "b");
    /// assert_eq!(alist.get(&1), Some(&"b"));
    /// alist.remove_all(&1);
    /// assert_eq!(alist.get(&1), None);
    /// ```
    pub fn remove_all<Q: ?Sized>(&mut self, key: &Q) -> bool
    where
        K: Borrow<Q>,
        Q: PartialEq,
    {
        let len_before = self.len();
        self.entries.retain(|(k, _)| key != k.borrow());
        let len_after = self.len();
        len_before != len_after
    }

    /// Returns a reference to the value corresponding to the key.
    ///
    /// The key may be any borrowed form of the list's key type, but the
    /// ordering on the borrowed form *must* match the ordering on the key type.
    ///
    /// If multiple entries in the map have keys equal to the given key, the
    /// value in the most recently inserted entry will be returned.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// assert_eq!(alist.get(&1), Some(&"a"));
    /// assert_eq!(alist.get(&2), None);
    /// ```
    pub fn get<Q: ?Sized>(&self, key: &Q) -> Option<&V>
    where
        K: Borrow<Q>,
        Q: PartialEq,
    {
        get_last_entry(&self.entries, key).map(|(_, v)| v)
    }

    /// Returns the key-value pair corresponding to the supplied key.
    ///
    /// The key may be any borrowed form of the list's key type, but the
    /// ordering on the borrowed form *must* match the ordering on the key type.
    ///
    /// If multiple entries in the map have keys equal to the given key, the
    /// most recently inserted entry will be returned.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// assert_eq!(alist.get_key_value(&1), Some((&1, &"a")));
    /// assert_eq!(alist.get_key_value(&2), None);
    /// ```
    pub fn get_key_value<Q: ?Sized>(&self, key: &Q) -> Option<(&K, &V)>
    where
        K: Borrow<Q>,
        Q: PartialEq,
    {
        get_last_entry(&self.entries, key).map(|(k, v)| (k, v))
    }

    /// Returns `true` if the list contains a value for the specified key.
    ///
    /// The key may be any borrowed form of the list's key type, but the
    /// ordering on the borrowed form *must* match the ordering on the key type.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// assert_eq!(alist.contains_key(&1), true);
    /// assert_eq!(alist.contains_key(&2), false);
    /// ```
    pub fn contains_key<Q: ?Sized>(&self, key: &Q) -> bool
    where
        K: Borrow<Q>,
        Q: PartialEq,
    {
        self.get(key).is_some()
    }

    /// Gets an iterator over the entries of the association list.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// alist.insert(2, "b");
    ///
    /// for (key, value) in alist.iter() {
    ///     println!("{}: {}", key, value);
    /// }
    ///
    /// let (first_key, first_value) = alist.iter().next().unwrap();
    /// assert_eq!((*first_key, *first_value), (1, "a"));
    /// ```
    pub fn iter(&self) -> impl Iterator<Item = (&K, &V)> {
        self.entries.iter().map(|(k, v)| (k, v))
    }

    /// Gets an iterator over the keys of the association list.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// alist.insert(2, "b");
    ///
    /// let keys: Vec<_> = alist.keys().copied().collect();
    /// assert_eq!(keys, [1, 2]);
    /// ```
    pub fn keys(&self) -> impl Iterator<Item = &K> {
        self.entries.iter().map(|(k, _)| k)
    }

    /// Gets an iterator over the values of the association list.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "hello");
    /// alist.insert(2, "goodbye");
    ///
    /// let values: Vec<&str> = alist.values().copied().collect();
    /// assert_eq!(values, ["hello", "goodbye"]);
    /// ```
    pub fn values(&self) -> impl Iterator<Item = &V> {
        self.entries.iter().map(|(_, v)| v)
    }

    /// Returns the number of entries in the list.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// assert_eq!(alist.len(), 0);
    /// alist.insert(1, "a");
    /// assert_eq!(alist.len(), 1);
    /// ```
    pub fn len(&self) -> usize {
        self.entries.len()
    }

    /// Returns `true` if the list contains no entries.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// assert!(alist.is_empty());
    /// alist.insert(1, "a");
    /// assert!(!alist.is_empty());
    /// ```
    pub fn is_empty(&self) -> bool {
        self.entries.is_empty()
    }
}

impl<K: Debug, V: Debug> Debug for AssocListMut<'_, K, V> {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fmt.debug_map().entries(self.iter()).finish()
    }
}

/// Get an entry in the slice with the given key. Performs a linear search on
/// small slices, and a binary search otherwise.
#[inline(always)]
fn get_sorted_entry<'a, K, V, Q: ?Sized>(entries: &'a [(K, V)], key: &Q) -> Option<&'a (K, V)>
where
    K: Borrow<Q>,
    Q: Ord,
{
    // TODO: tune this threshold based on perf results
    const BINARY_SEARCH_LEN_THRESHOLD: usize = 32;

    if entries.len() < BINARY_SEARCH_LEN_THRESHOLD {
        entries.iter().find(|(k, _)| key == k.borrow())
    } else {
        let index = entries
            .binary_search_by(|(k, _)| k.borrow().cmp(key))
            .ok()?;
        Some(&entries[index])
    }
}

/// A readonly associative array, sorted by key, containing no duplicate keys.
///
/// * Lookups run in log(n) time
/// * Entries with duplicate keys are not permitted. When constructing a
///   `SortedAssocList` from an `AssocListMut`, entries will be deduplicated by
///   key, and only the most recently inserted entry for each key will be
///   retained.
#[derive(Deserialize, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
#[serde(bound(
    deserialize = "K: 'de + arena_deserializer::DeserializeInArena<'de>, V: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
pub struct SortedAssocList<'a, K, V> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    entries: &'a [(K, V)],
}

arena_deserializer::impl_deserialize_in_arena!(SortedAssocList<'arena, K, V>);

impl<'a, K, V> SortedAssocList<'a, K, V> {
    /// Returns the empty association list.
    pub fn empty() -> Self {
        SortedAssocList { entries: &[] }
    }

    /// Returns a reference to the value corresponding to the key.
    ///
    /// The key may be any borrowed form of the list's key type, but the
    /// ordering on the borrowed form *must* match the ordering on the key type.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use arena_collections::SortedAssocList;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// let alist = SortedAssocList::from(alist);
    /// assert_eq!(alist.get(&1), Some(&"a"));
    /// assert_eq!(alist.get(&2), None);
    /// ```
    pub fn get<Q: ?Sized>(&self, key: &Q) -> Option<&V>
    where
        K: Borrow<Q>,
        Q: Ord,
    {
        get_sorted_entry(self.entries, key).map(|(_, v)| v)
    }

    /// Returns the key-value pair corresponding to the supplied key.
    ///
    /// The key may be any borrowed form of the list's key type, but the
    /// ordering on the borrowed form *must* match the ordering on the key type.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use arena_collections::SortedAssocList;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// let alist = SortedAssocList::from(alist);
    /// assert_eq!(alist.get_key_value(&1), Some((&1, &"a")));
    /// assert_eq!(alist.get_key_value(&2), None);
    /// ```
    pub fn get_key_value<Q: ?Sized>(&self, key: &Q) -> Option<(&K, &V)>
    where
        K: Borrow<Q>,
        Q: Ord,
    {
        get_sorted_entry(self.entries, key).map(|(k, v)| (k, v))
    }

    /// Returns `true` if the list contains a value for the specified key.
    ///
    /// The key may be any borrowed form of the list's key type, but the
    /// ordering on the borrowed form *must* match the ordering on the key type.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use arena_collections::SortedAssocList;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// let alist = SortedAssocList::from(alist);
    /// assert_eq!(alist.contains_key(&1), true);
    /// assert_eq!(alist.contains_key(&2), false);
    /// ```
    pub fn contains_key<Q: ?Sized>(&self, key: &Q) -> bool
    where
        K: Borrow<Q>,
        Q: Ord,
    {
        get_sorted_entry(self.entries, key).is_some()
    }

    /// Gets an iterator over the entries of the association list, sorted by
    /// key.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use arena_collections::SortedAssocList;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    ///
    /// alist.insert(1, "a");
    /// alist.insert(2, "b");
    ///
    /// let alist = SortedAssocList::from(alist);
    ///
    /// for (key, value) in alist.iter() {
    ///     println!("{}: {}", key, value);
    /// }
    ///
    /// let (first_key, first_value) = alist.iter().next().unwrap();
    /// assert_eq!((*first_key, *first_value), (1, "a"));
    /// ```
    pub fn iter(&self) -> impl Iterator<Item = (&K, &V)> {
        self.entries.iter().map(|(k, v)| (k, v))
    }

    /// Gets an iterator over the keys of the association list, in sorted order.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use arena_collections::SortedAssocList;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    ///
    /// alist.insert(1, "a");
    /// alist.insert(2, "b");
    ///
    /// let alist = SortedAssocList::from(alist);
    /// let keys: Vec<_> = alist.keys().copied().collect();
    /// assert_eq!(keys, [1, 2]);
    /// ```
    pub fn keys(&self) -> impl Iterator<Item = &K> {
        self.entries.iter().map(|(k, _)| k)
    }

    /// Gets an iterator over the values of the association list, in order by
    /// key.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use arena_collections::SortedAssocList;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    ///
    /// alist.insert(1, "hello");
    /// alist.insert(2, "goodbye");
    ///
    /// let alist = SortedAssocList::from(alist);
    /// let values: Vec<&str> = alist.values().copied().collect();
    /// assert_eq!(values, ["hello", "goodbye"]);
    /// ```
    pub fn values(&self) -> impl Iterator<Item = &V> {
        self.entries.iter().map(|(_, v)| v)
    }

    /// Returns the number of entries in the list.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use arena_collections::SortedAssocList;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// alist.insert(1, "a");
    /// alist.insert(1, "b");
    /// let alist = SortedAssocList::from(alist);
    /// assert_eq!(alist.len(), 1);
    /// ```
    pub fn len(&self) -> usize {
        self.entries.len()
    }

    /// Returns `true` if the list contains no entries.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use arena_collections::SortedAssocList;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    /// let alist = SortedAssocList::from(alist);
    /// assert!(alist.is_empty());
    /// ```
    pub fn is_empty(&self) -> bool {
        self.entries.is_empty()
    }

    /// Make a new `SortedAssocList` containing the given key-value pairs.
    ///
    /// Provided for the sake of creating empty const lists. Passing non-empty
    /// slices is not recommended.
    ///
    /// The values in the slice must be in ascending sorted order (by `K`'s
    /// implementation of `Ord`). There must be no duplicate keys in the slice.
    ///
    /// # Examples
    ///
    /// ```
    /// use arena_collections::AssocListMut;
    /// use arena_collections::SortedAssocList;
    /// use bumpalo::Bump;
    ///
    /// let b = Bump::new();
    /// let mut alist = AssocListMut::new_in(&b);
    ///
    /// const EMPTY_ALIST: SortedAssocList<'_, usize> = SortedAssocList::from_slice(&[]);
    /// assert!(EMPTY_ALIST.is_empty());
    /// ```
    pub const fn from_slice(entries: &'a [(K, V)]) -> Self {
        Self { entries }
    }
}

impl<K, V> TrivialDrop for SortedAssocList<'_, K, V> {}
impl<K, V> Copy for SortedAssocList<'_, K, V> {}
impl<K, V> Clone for SortedAssocList<'_, K, V> {
    fn clone(&self) -> Self {
        *self
    }
}

impl<K, V> Default for SortedAssocList<'_, K, V> {
    fn default() -> Self {
        Self::from_slice(&[])
    }
}

impl<K: Debug, V: Debug> Debug for SortedAssocList<'_, K, V> {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        fmt.debug_map().entries(self.iter()).finish()
    }
}

impl<'a, K: Ord, V> From<AssocListMut<'a, K, V>> for SortedAssocList<'a, K, V> {
    #[inline]
    fn from(mut alist: AssocListMut<'a, K, V>) -> Self {
        let entries_mut = alist.entries.as_mut_slice();
        // Reverse the slice so the most recently inserted pairs appear first.
        entries_mut.reverse();
        // Keep recently-inserted pairs first with a stable sort. Allocates
        // temporary storage half the size of the slice using the global
        // allocator if the slice is larger than some threshold (20 elements at
        // time of writing).
        entries_mut.sort_by(|(k1, _), (k2, _)| k1.cmp(k2));
        // Remove all but the most recently inserted pair for each key.
        alist.entries.dedup_by(|(k1, _), (k2, _)| k1 == k2);
        SortedAssocList {
            entries: alist.entries.into_bump_slice(),
        }
    }
}

impl<K: ToOcamlRep + Ord, V: ToOcamlRep> ToOcamlRep for SortedAssocList<'_, K, V> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        let len = self.len();
        let mut iter = self
            .iter()
            .map(|(k, v)| (k.to_ocamlrep(alloc), v.to_ocamlrep(alloc)));
        let (value, _) = ocamlrep::sorted_iter_to_ocaml_map(&mut iter, alloc, len);
        value
    }
}

impl<'a, K, V> FromOcamlRepIn<'a> for SortedAssocList<'a, K, V>
where
    K: FromOcamlRepIn<'a> + Ord,
    V: FromOcamlRepIn<'a>,
{
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        let mut entries = bumpalo::collections::Vec::new_in(alloc);
        ocamlrep::vec_from_ocaml_map_in(value, &mut entries, alloc)?;
        let entries = entries.into_bump_slice();
        Ok(Self { entries })
    }
}
