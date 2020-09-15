// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::{AssocListMut, SortedAssocList};
use bumpalo::Bump;

fn new<'a, K: Copy + Ord, V: Copy>(
    b: &'a Bump,
    entries: &'a [(K, V)],
) -> SortedAssocList<'a, K, V> {
    let mut alist = AssocListMut::new_in(b);
    for (key, value) in entries {
        alist.insert(*key, *value);
    }
    alist.into()
}

#[test]
fn get_with_duplicate_keys() {
    let b = Bump::new();
    let alist = new(
        &b,
        &[(1, "a"), (2, "x"), (1, "b"), (0, "y"), (1, "c"), (3, "z")],
    );
    assert_eq!(alist.get(&1), Some(&"c"));
}

#[test]
fn get_key_value_with_duplicate_keys() {
    let b = Bump::new();
    let alist = new(
        &b,
        &[(1, "a"), (2, "x"), (1, "b"), (0, "y"), (1, "c"), (3, "z")],
    );
    assert_eq!(alist.get_key_value(&1), Some((&1, &"c")));
}

#[test]
fn len_with_duplicate_keys() {
    let b = Bump::new();
    let alist = new(&b, &[(1, "a"), (1, "b"), (2, "z")]);
    assert_eq!(alist.len(), 2);
}

#[test]
fn get_with_many_entries() {
    let b = &Bump::new();
    let mut entries = bumpalo::collections::Vec::new_in(&b);
    for i in 0..1024 {
        entries.push((i, &*b.alloc_str(i.to_string().as_str())))
    }
    let alist = new(b, &entries[..]);
    for i in 0..1024 {
        assert_eq!(alist.get(&i), Some(&i.to_string().as_str()));
    }
}

// Doctests ////////////////////////////////////////////////////////////////////

// The tests below are copied from doc comments. We should eventually be able to
// run doctests as Buck tests, and at that time we can remove these test cases.

// Other doctests for SortedAssocList are located in the test_alist_mut module.

#[test]
fn from_slice() {
    const EMPTY_ALIST: SortedAssocList<'_, i32, i32> = SortedAssocList::from_slice(&[]);
    assert!(EMPTY_ALIST.is_empty());
}
