// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{AssocList, AssocListMut, SortedAssocList};
use bumpalo::Bump;

// Doctests ////////////////////////////////////////////////////////////////////

// The tests below are copied from doc comments. We should eventually be able to
// run doctests as Buck tests, and at that time we can remove these test cases.

#[test]
fn with_capacity_in() {
    let b = Bump::new();
    let mut alist = AssocListMut::with_capacity_in(10, &b);

    // The list contains no items, even though it has capacity for more
    assert_eq!(alist.len(), 0);

    // These are all done without reallocating...
    for i in 0..10 {
        alist.insert(i, i);
    }

    // ...but this may make the list reallocate
    alist.insert(11, 11);
}

#[test]
fn insert() {
    let b = Bump::new();
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "a");
    assert_eq!(alist.get(&1), Some(&"a"));
    alist.insert(1, "b");
    assert_eq!(alist.get(&1), Some(&"b"));
    assert_eq!(alist.len(), 2);
}

#[test]
fn insert_or_replace() {
    let b = Bump::new();
    let mut alist = AssocListMut::new_in(&b);
    alist.insert_or_replace(1, "a");
    assert_eq!(alist.get(&1), Some(&"a"));
    alist.insert_or_replace(1, "b");
    assert_eq!(alist.get(&1), Some(&"b"));
    assert_eq!(alist.len(), 1);
}

#[test]
fn remove() {
    let b = Bump::new();
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "a");
    alist.insert(1, "b");
    assert_eq!(alist.get(&1), Some(&"b"));
    alist.remove(&1);
    assert_eq!(alist.get(&1), Some(&"a"));
    alist.remove(&1);
    assert_eq!(alist.get(&1), None);
}

#[test]
fn remove_all() {
    let b = Bump::new();
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "a");
    alist.insert(1, "b");
    assert_eq!(alist.get(&1), Some(&"b"));
    alist.remove_all(&1);
    assert_eq!(alist.get(&1), None);
}

#[test]
fn into() {
    let b = Bump::new();
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "a");
    let alist: AssocList<'_, _, _> = alist.into();
    let entries = [(1, "a")];
    assert_eq!(alist, AssocList::new(&entries[..]));
}

#[test]
fn get() {
    let b = Bump::new();
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "a");
    assert_eq!(alist.get(&1), Some(&"a"));
    assert_eq!(alist.get(&2), None);
    let alist = SortedAssocList::from(alist);
    assert_eq!(alist.get(&1), Some(&"a"));
    assert_eq!(alist.get(&2), None);
}

#[test]
fn get_key_value() {
    let b = Bump::new();
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "a");
    assert_eq!(alist.get_key_value(&1), Some((&1, &"a")));
    assert_eq!(alist.get_key_value(&2), None);
    let alist = SortedAssocList::from(alist);
    assert_eq!(alist.get_key_value(&1), Some((&1, &"a")));
    assert_eq!(alist.get_key_value(&2), None);
}

#[test]
fn contains_key() {
    let b = Bump::new();
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "a");
    assert_eq!(alist.contains_key(&1), true);
    assert_eq!(alist.contains_key(&2), false);
    let alist = SortedAssocList::from(alist);
    assert_eq!(alist.contains_key(&1), true);
    assert_eq!(alist.contains_key(&2), false);
}

#[test]
fn iter() {
    let b = Bump::new();
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "a");
    alist.insert(2, "b");
    let (first_key, first_value) = alist.iter().next().unwrap();
    assert_eq!((*first_key, *first_value), (1, "a"));
    let alist = SortedAssocList::from(alist);
    let (first_key, first_value) = alist.iter().next().unwrap();
    assert_eq!((*first_key, *first_value), (1, "a"));
}

#[test]
fn keys() {
    let b = Bump::new();
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "a");
    alist.insert(2, "b");
    let keys: Vec<_> = alist.keys().copied().collect();
    assert_eq!(keys, [1, 2]);
    let alist = SortedAssocList::from(alist);
    let keys: Vec<_> = alist.keys().copied().collect();
    assert_eq!(keys, [1, 2]);
}

#[test]
fn values() {
    let b = Bump::new();
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "hello");
    alist.insert(2, "goodbye");
    let values: Vec<&str> = alist.values().copied().collect();
    assert_eq!(values, ["hello", "goodbye"]);
    let alist = SortedAssocList::from(alist);
    let values: Vec<&str> = alist.values().copied().collect();
    assert_eq!(values, ["hello", "goodbye"]);
}

#[test]
fn len() {
    let b = Bump::new();
    let alist: AssocListMut<'_, i32, i32> = AssocListMut::new_in(&b);
    assert_eq!(alist.len(), 0);
    let alist = SortedAssocList::from(alist);
    assert_eq!(alist.len(), 0);
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "a");
    assert_eq!(alist.len(), 1);
    let alist = SortedAssocList::from(alist);
    assert_eq!(alist.len(), 1);
}

#[test]
fn is_empty() {
    let b = Bump::new();
    let alist: AssocListMut<'_, i32, i32> = AssocListMut::new_in(&b);
    assert!(alist.is_empty());
    let alist = SortedAssocList::from(alist);
    assert!(alist.is_empty());
    let mut alist = AssocListMut::new_in(&b);
    alist.insert(1, "a");
    assert!(!alist.is_empty());
    let alist = SortedAssocList::from(alist);
    assert!(!alist.is_empty());
}
