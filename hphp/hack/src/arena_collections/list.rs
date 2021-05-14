// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt::Debug;

use serde::{Deserialize, Serialize};

use arena_trait::{Arena, TrivialDrop};
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};

#[derive(
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    PartialEq,
    PartialOrd,
    Ord,
    Serialize,
    ToOcamlRep
)]
#[serde(bound(deserialize = "T: 'de + arena_deserializer::DeserializeInArena<'de>"))]
pub enum List<'a, T> {
    Nil,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Cons(&'a (T, List<'a, T>)),
}
arena_deserializer::impl_deserialize_in_arena!(List<'arena, T>);

use List::*;

impl<'a, T> List<'a, T> {
    /// Return the empty list.
    pub const fn empty() -> Self {
        Nil
    }

    /// Prepend the given element to the given list.
    #[inline]
    pub fn cons<A: Arena>(element: T, list: Self, arena: &'a A) -> Self
    where
        T: TrivialDrop,
    {
        Cons(arena.alloc((element, list)))
    }

    /// Return the length (number of elements) of the given list.
    pub fn len(self) -> usize {
        let mut node = self;
        let mut len = 0;
        while let Cons(&(_, next)) = node {
            node = next;
            len += 1;
        }
        len
    }

    /// Return `true` if the list contains no elements.
    #[inline]
    pub fn is_empty(self) -> bool {
        match self {
            Nil => true,
            Cons(..) => false,
        }
    }

    /// Return the first element of the list.
    #[inline]
    pub fn hd(self) -> Option<&'a T> {
        match self {
            Nil => None,
            Cons((x, _)) => Some(x),
        }
    }

    /// Return the list without its first element.
    #[inline]
    pub fn tl(self) -> Option<Self> {
        match self {
            Nil => None,
            Cons(&(_, l)) => Some(l),
        }
    }

    /// Return the `n`-th element of the given list. The first element (head of
    /// the list) is at position 0. Return `None` if the list is too short.
    pub fn nth(self, n: usize) -> Option<&'a T> {
        let mut node = self;
        let mut idx = 0;
        while let Cons((x, next)) = node {
            if n == idx {
                return Some(x);
            }
            node = *next;
            idx += 1;
        }
        None
    }

    /// List reversal.
    pub fn rev<A: Arena>(self, arena: &'a A) -> Self
    where
        T: Clone + TrivialDrop,
    {
        self.rev_append(Nil, arena)
    }

    /// Concatenate two lists by cloning all elements from `self` into a list
    /// which points to `other` as a suffix.
    pub fn append<A: Arena>(self, other: Self, arena: &'a A) -> Self
    where
        T: Clone + TrivialDrop,
    {
        self.count_append(other, 0, arena)
    }

    /// `count_append` is borrowed from Jane Street Base, and applies two
    /// optimizations for `append`: loop unrolling, and dynamic switching
    /// between stack and arena allocation.
    ///
    /// The loop-unrolling is straightforward, we just unroll 5 levels of the
    /// loop. This should make each iteration faster, and also reduces the
    /// number of stack frames consumed per list element.
    ///
    /// The dynamic switching is done by counting the number of stack frames,
    /// and then switching to the "slow" implementation when we exceed a given
    /// limit. This means that short lists use the fast stack-allocation method,
    /// and long lists use a slower one that doesn't require stack space.
    fn count_append<A: Arena>(self, other: Self, count: usize, arena: &'a A) -> Self
    where
        T: Clone + TrivialDrop,
    {
        if other.is_empty() {
            return self;
        }
        let a = arena;
        match self {
            Nil => other,
            Cons((x1, Nil)) => Self::cons(x1.clone(), other, a),
            Cons((x1, Cons((x2, Nil)))) => {
                let x1 = x1.clone();
                let x2 = x2.clone();
                Self::cons(x1, Self::cons(x2, other, a), a)
            }
            Cons((x1, Cons((x2, Cons((x3, Nil)))))) => {
                let x1 = x1.clone();
                let x2 = x2.clone();
                let x3 = x3.clone();
                Self::cons(x1, Self::cons(x2, Self::cons(x3, other, a), a), a)
            }
            Cons((x1, Cons((x2, Cons((x3, Cons((x4, Nil)))))))) => {
                let x1 = x1.clone();
                let x2 = x2.clone();
                let x3 = x3.clone();
                let x4 = x4.clone();
                Self::cons(
                    x1,
                    Self::cons(x2, Self::cons(x3, Self::cons(x4, other, a), a), a),
                    a,
                )
            }
            Cons((x1, Cons((x2, Cons((x3, Cons((x4, Cons((x5, tl)))))))))) => {
                let tl = if count > 1000 {
                    tl.slow_append(other, a)
                } else {
                    tl.count_append(other, count + 1, a)
                };
                let x1 = x1.clone();
                let x2 = x2.clone();
                let x3 = x3.clone();
                let x4 = x4.clone();
                let x5 = x5.clone();
                Self::cons(
                    x1,
                    Self::cons(
                        x2,
                        Self::cons(x3, Self::cons(x4, Self::cons(x5, tl, a), a), a),
                        a,
                    ),
                    a,
                )
            }
        }
    }

    /// Helper for `append`. See comment on `count_append`.
    fn slow_append<A: Arena>(self, other: Self, arena: &'a A) -> Self
    where
        T: Clone + TrivialDrop,
    {
        let temp_arena = bumpalo::Bump::new();
        self.rev(&temp_arena).rev_append(other, arena)
    }

    /// `l1.rev_append(l2)` reverses `l1` and concatenates it to `l2`. This is
    /// equivalent to `l1.rev().append(l2)`, but `rev_append` is more efficient.
    pub fn rev_append<'b, A: Arena>(self, other: List<'b, T>, arena: &'b A) -> List<'b, T>
    where
        'b: 'a,
        T: Clone + TrivialDrop + 'b,
    {
        let mut node = self;
        let mut result = other;
        while let Cons((x, next)) = node {
            node = *next;
            result = Cons(arena.alloc((x.clone(), result)));
        }
        result
    }

    /// `List::init(len, f, arena)` is `list![in arena; f(0), f(1), ..., f(len-1)]`,
    /// evaluated right to left.
    pub fn init<A: Arena>(len: usize, mut f: impl FnMut(usize) -> T, arena: &'a A) -> Self
    where
        T: TrivialDrop,
    {
        let mut node = Nil;
        for i in (0..len).rev() {
            node = Cons(arena.alloc((f(i), node)));
        }
        node
    }

    /// Return an iterator over the elements of the list.
    pub fn iter(self) -> Iter<'a, T> {
        Iter(self)
    }

    /// Returns `true` if the `List` contains an element equal to the given value.
    pub fn contains(self, value: &T) -> bool
    where
        T: PartialEq,
    {
        self.iter().any(|x| x == value)
    }

    /// `l.find(p)` returns the first element of the list `l` that satisfies the
    /// predicate `p`, or `None` if there is no value that satisfies `p` in the
    /// list `l`.
    pub fn find(self, mut f: impl FnMut(&T) -> bool) -> Option<&'a T> {
        self.iter().find(|&x| f(x))
    }

    /// Produce a List containing the elements yielded by the given iterator, in
    /// reverse order.
    pub fn rev_from_iter_in<I: IntoIterator<Item = T>, A: Arena>(iter: I, arena: &'a A) -> Self
    where
        T: TrivialDrop,
    {
        let mut node = Nil;
        for x in iter {
            node = Cons(arena.alloc((x, node)));
        }
        node
    }

    /// Prepend the given element to the list in-place.
    pub fn push_front<A: Arena>(&mut self, element: T, arena: &'a A)
    where
        T: TrivialDrop,
    {
        *self = Cons(arena.alloc((element, *self)));
    }

    /// Remove the first element of the list in-place and return a reference to
    /// it, or `None` if the list is empty.
    pub fn pop_front(&mut self) -> Option<&'a T> {
        match self {
            Nil => None,
            Cons((x, l)) => {
                *self = *l;
                Some(x)
            }
        }
    }
}

// The derived implementations of Copy and Clone require `T` to be Copy/Clone.
// We have no such requirement, since List is just a pointer, so we manually
// implement them here.
impl<'a, T> Clone for List<'a, T> {
    fn clone(&self) -> Self {
        *self
    }
}
impl<'a, T> Copy for List<'a, T> {}
impl<'a, T: TrivialDrop> TrivialDrop for List<'a, T> {}

impl<T: Debug> Debug for List<'_, T> {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        fmt.debug_list().entries(self.iter()).finish()
    }
}

pub struct Iter<'a, T>(List<'a, T>);

impl<'a, T> Iterator for Iter<'a, T> {
    type Item = &'a T;

    #[inline]
    fn next(&mut self) -> Option<Self::Item> {
        match self.0 {
            Nil => None,
            Cons((x, l)) => {
                self.0 = *l;
                Some(x)
            }
        }
    }
}

impl<'a, T> IntoIterator for List<'a, T> {
    type Item = &'a T;
    type IntoIter = Iter<'a, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.iter()
    }
}

#[macro_export]
macro_rules! list {
    () => {
        $crate::list::List::Nil
    };
    (in $arena:expr; $x:expr $(,)?) => {{
        use $crate::Arena;
        $crate::list::List::Cons($arena.alloc(($x, $crate::list::List::Nil)))
    }};
    (in $arena:expr; $x:expr, $($xs:expr $(,)?)+) => {{
        use $crate::Arena;
        let arena = $arena;
        $crate::list::List::Cons(arena.alloc(($x, l![in arena; $($xs,)*])))
    }};
    (in $arena:expr; $elem:expr; $n:expr) => {{
        let elem = $elem;
        $crate::list::List::init($n, |_| elem.clone(), $arena)
    }};
}

#[macro_export]
macro_rules! stack_list {
    () => {
        $crate::list::List::Nil
    };
    ($x:expr $(,)?) => {
        $crate::list::List::Cons(&($x, $crate::list::List::Nil))
    };
    ($x:expr, $($xs:expr $(,)?)+) => {
        $crate::list::List::Cons(&($x, stack_list![$($xs,)*]))
    };
}
