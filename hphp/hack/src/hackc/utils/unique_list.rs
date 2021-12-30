// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use indexmap::{Equivalent, IndexSet};
use std::hash::Hash;

#[derive(Clone)]
pub struct UniqueList<T>
where
    T: Hash + Eq,
{
    set: IndexSet<T>,
}

impl<T> UniqueList<T>
where
    T: Hash + Eq,
{
    pub fn new() -> Self {
        UniqueList {
            set: IndexSet::new(),
        }
    }

    pub fn empty(&mut self) {
        self.set.clear();
    }

    pub fn add(&mut self, item: T) {
        if !self.set.contains(&item) {
            self.set.insert(item);
        }
    }

    pub fn iter(&self) -> impl DoubleEndedIterator<Item = &T> {
        self.set.iter()
    }

    #[allow(clippy::should_implement_trait)]
    pub fn into_iter(self) -> impl DoubleEndedIterator<Item = T> {
        self.set.into_iter()
    }

    pub fn items_set(&self) -> &IndexSet<T> {
        &self.set
    }

    pub fn remove<Q: ?Sized>(&mut self, item: &Q)
    where
        Q: Hash + Equivalent<T>,
    {
        self.set.shift_remove(item);
    }

    pub fn diff_iter<'a>(&'a self, s2: &'a UniqueList<T>) -> impl Iterator<Item = &'a T> {
        self.set.difference(&s2.set)
    }

    pub fn diff<'a>(&'a self, s2: &'a UniqueList<T>) -> IndexSet<&'a T> {
        self.set.difference(&s2.set).collect()
    }

    pub fn cardinal(&self) -> usize {
        self.set.len()
    }
}

#[cfg(test)]
mod unique_list_tests {
    use super::UniqueList;

    fn make_test_list() -> UniqueList<usize> {
        let mut uniq_list = UniqueList::new();
        uniq_list.add(3);
        uniq_list.add(5);
        uniq_list.add(4);
        uniq_list.add(9);
        uniq_list.add(6);
        uniq_list.add(1);
        uniq_list
    }

    #[test]
    fn add_items_test() {
        let uniq_list = make_test_list();
        assert!(uniq_list.items_set().contains(&3));
        assert!(uniq_list.items_set().contains(&5));
        assert!(uniq_list.items_set().contains(&4));
        assert!(uniq_list.items_set().contains(&9));
        assert!(uniq_list.items_set().contains(&6));
        assert!(uniq_list.items_set().contains(&1));
        assert!(uniq_list.cardinal() == 6);
    }

    #[test]
    fn remove_test() {
        let mut uniq_list = make_test_list();
        uniq_list.remove(&4);
        assert!(!uniq_list.items_set().contains(&4));
        assert!(uniq_list.cardinal() == 5);

        let expect = vec![3, 5, 9, 6, 1];
        let matching1 = uniq_list
            .iter()
            .zip(expect.iter())
            .filter(|&(a, b)| a == b)
            .count();

        let matching2 = uniq_list
            .iter()
            .zip(expect.iter())
            .filter(|&(a, b)| a == b)
            .count();
        assert!(matching1 == 5);
        assert!(matching2 == 5);
    }

    #[test]
    fn empty_test() {
        let mut uniq_list = make_test_list();
        uniq_list.empty();
        assert!(!uniq_list.items_set().contains(&3));
        assert!(uniq_list.cardinal() == 0);
    }

    #[test]
    fn diff_test() {
        let uniq_list = make_test_list();
        let mut subtract_list = make_test_list();
        subtract_list.remove(&5);
        subtract_list.remove(&9);
        subtract_list.remove(&1);
        let set = uniq_list.diff(&subtract_list);
        let expect = vec![5, 9, 1];
        let matching = set
            .iter()
            .zip(expect.iter())
            .filter(|&(&a, b)| a == b)
            .count();
        assert!(matching == 3);
        assert!(set.len() == 3);
    }
}
