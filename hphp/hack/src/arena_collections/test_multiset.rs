// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{MultiSet, MultiSetMut, SortedSet};
use bumpalo::Bump;

// Doctests ////////////////////////////////////////////////////////////////////

// The tests below are copied from doc comments. We should eventually be able to
// run doctests as Buck tests, and at that time we can remove these test cases.

#[test]
fn contains() {
    let b = Bump::new();
    let mut set = MultiSetMut::new_in(&b);
    set.insert(1);
    assert!(set.contains(&1));
    let set = MultiSet::from(set);
    assert!(set.contains(&1));
}

#[test]
fn iter() {
    let b = Bump::new();
    let mut set = MultiSetMut::new_in(&b);
    set.insert(3);
    set.insert(1);
    set.insert(2);
    {
        let mut set_iter = set.iter();
        assert_eq!(set_iter.next(), Some(&3));
        assert_eq!(set_iter.next(), Some(&1));
        assert_eq!(set_iter.next(), Some(&2));
        assert_eq!(set_iter.next(), None);
    }
    let set = MultiSet::from(set);
    let mut set_iter = set.iter();
    assert_eq!(set_iter.next(), Some(&3));
    assert_eq!(set_iter.next(), Some(&1));
    assert_eq!(set_iter.next(), Some(&2));
    assert_eq!(set_iter.next(), None);
}

#[test]
fn iter_sorted() {
    let b = Bump::new();
    let mut set = MultiSetMut::new_in(&b);
    set.insert(3);
    set.insert(1);
    set.insert(2);
    let set = SortedSet::from(set);
    let mut set_iter = set.iter();
    assert_eq!(set_iter.next(), Some(&1));
    assert_eq!(set_iter.next(), Some(&2));
    assert_eq!(set_iter.next(), Some(&3));
    assert_eq!(set_iter.next(), None);
}

#[test]
fn len() {
    let b = Bump::new();
    let mut set = MultiSetMut::new_in(&b);
    assert_eq!(set.len(), 0);
    set.insert(1);
    assert_eq!(set.len(), 1);
    set.insert(1);
    assert_eq!(set.len(), 2);
    set.insert(2);
    assert_eq!(set.len(), 3);
    let set = MultiSet::from(set);
    assert_eq!(set.len(), 3);
}

#[test]
fn len_sorted() {
    let b = Bump::new();
    let mut set = MultiSetMut::new_in(&b);
    set.insert(1);
    set.insert(2);
    set.insert(1);
    let set = SortedSet::from(set);
    assert_eq!(set.len(), 2);
}

#[test]
fn from_slice() {
    const EMPTY_MULTISET: MultiSet<'_, i32> = MultiSet::from_slice(&[]);
    assert!(EMPTY_MULTISET.is_empty());
    const EMPTY_SORTED_SET: SortedSet<'_, i32> = SortedSet::from_slice(&[]);
    assert!(EMPTY_SORTED_SET.is_empty());
}

#[test]
fn insert() {
    let b = Bump::new();
    let mut set = MultiSetMut::new_in(&b);
    assert_eq!(set.contains(&1), false);
    set.insert(1);
    assert_eq!(set.contains(&1), true);
}

#[test]
fn remove() {
    let b = Bump::new();
    let mut set = MultiSetMut::new_in(&b);

    set.insert(2);
    assert_eq!(set.remove(&2), true);
    assert_eq!(set.remove(&2), false);
}

#[test]
fn remove_all() {
    let b = Bump::new();
    let mut set = MultiSetMut::new_in(&b);

    set.insert(2);
    set.insert(2);
    assert_eq!(set.remove_all(&2), true);
    assert_eq!(set.remove_all(&2), false);
}
