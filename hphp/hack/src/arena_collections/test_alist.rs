// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::AssocList;

#[test]
fn get_with_duplicate_keys() {
    let entries = [(1, "a"), (1, "b")];
    let alist = AssocList::new(&entries[..]);
    assert_eq!(alist.get(&1), Some(&"b"));
}

#[test]
fn get_key_value_with_duplicate_keys() {
    let entries = [(1, "a"), (1, "b")];
    let alist = AssocList::new(&entries[..]);
    assert_eq!(alist.get_key_value(&1), Some((&1, &"b")));
}

#[test]
fn len_with_duplicate_keys() {
    let entries = [(1, "a"), (1, "b"), (2, "c")];
    let alist = AssocList::new(&entries[..]);
    assert_eq!(alist.len(), 3);
}

// Doctests ////////////////////////////////////////////////////////////////////

// The tests below are copied from doc comments. We should eventually be able to
// run doctests as Buck tests, and at that time we can remove these test cases.

#[test]
fn get() {
    let entries = [(1, "a")];
    let alist = AssocList::new(&entries[..]);
    assert_eq!(alist.get(&1), Some(&"a"));
    assert_eq!(alist.get(&2), None);
}

#[test]
fn get_key_value() {
    let entries = [(1, "a")];
    let alist = AssocList::new(&entries[..]);
    assert_eq!(alist.get_key_value(&1), Some((&1, &"a")));
    assert_eq!(alist.get_key_value(&2), None);
}

#[test]
fn contains_key() {
    let entries = [(1, "a")];
    let alist = AssocList::new(&entries[..]);
    assert_eq!(alist.contains_key(&1), true);
    assert_eq!(alist.contains_key(&2), false);
}

#[test]
fn iter() {
    let entries = [(1, "a"), (2, "b")];
    let alist = AssocList::new(&entries[..]);
    let (first_key, first_value) = alist.iter().next().unwrap();
    assert_eq!((*first_key, *first_value), (1, "a"));
}

#[test]
fn keys() {
    let entries = [(1, "a"), (2, "b")];
    let alist = AssocList::new(&entries[..]);
    let keys: Vec<_> = alist.keys().copied().collect();
    assert_eq!(keys, [1, 2]);
}

#[test]
fn values() {
    let entries = [(1, "hello"), (2, "goodbye")];
    let alist = AssocList::new(&entries[..]);
    let values: Vec<&str> = alist.values().copied().collect();
    assert_eq!(values, ["hello", "goodbye"]);
}

#[test]
fn len() {
    let entries = [(1, "a")];
    let alist = AssocList::new(&entries[0..0]);
    assert_eq!(alist.len(), 0);
    let alist = AssocList::new(&entries[0..1]);
    assert_eq!(alist.len(), 1);
}

#[test]
fn is_empty() {
    let entries = [(1, "a")];
    let alist = AssocList::new(&entries[0..0]);
    assert!(alist.is_empty());
    let alist = AssocList::new(&entries[0..1]);
    assert!(!alist.is_empty());
}
