// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::Bump;

use crate::list::List;

macro_rules! sl {
    () => { List::Nil::<i32> };
    ($($xs:expr $(,)?)+) => { stack_list![$($xs,)*] };
}

#[test]
fn test_size() {
    // List's variants are Nil and Cons(&_). Since there's only one nullary
    // variant (Nil), and only one variant with an argument (Cons), and Cons
    // contains a (non-nullable) reference, List benefits from the
    // null-pointer-optimization. Cons is represented with just a reference, and
    // Nil is represented with the null pointer. As a result, any List is the
    // size of a pointer.
    assert_eq!(
        std::mem::size_of::<List<'_, i32>>(),
        std::mem::size_of::<*const u8>()
    );
}

#[test]
fn cons() {
    let a = &Bump::new();
    assert_eq!(List::cons(3, List::Nil, a), sl![3]);
    assert_eq!(List::cons(3, sl![2, 1], a), sl![3, 2, 1]);
}

#[test]
fn len() {
    assert_eq!(sl![].len(), 0);
    assert_eq!(sl![1].len(), 1);
    assert_eq!(sl![1, 2].len(), 2);
    assert_eq!(sl![1, 2, 3, 4, 5, 6, 7, 8, 9, 10].len(), 10);
}

#[test]
fn is_empty() {
    assert!(sl![].is_empty());
    assert!(!sl![1].is_empty());
    assert!(!sl![1, 2].is_empty());
    assert!(!sl![1, 2, 3, 4, 5, 6, 7, 8, 9, 10].is_empty());
}

#[test]
fn hd() {
    assert_eq!(sl![].hd(), None);
    assert_eq!(sl![42].hd(), Some(&42));
    assert_eq!(sl![19, 84].hd(), Some(&19));
}

#[test]
fn tl() {
    assert_eq!(sl![].tl(), None);
    assert_eq!(sl![42].tl(), Some(sl![]));
    assert_eq!(sl![19, 84].tl(), Some(sl![84]));
}

#[test]
fn nth() {
    assert_eq!(sl![].nth(0), None);
    assert_eq!(sl![].nth(1), None);
    assert_eq!(sl![1].nth(0), Some(&1));
    assert_eq!(sl![1].nth(1), None);
    assert_eq!(sl![1, 2].nth(0), Some(&1));
    assert_eq!(sl![1, 2].nth(1), Some(&2));
    assert_eq!(sl![1, 2, 3, 4, 5, 6, 7, 8, 9, 10].nth(9), Some(&10));
    assert_eq!(sl![1, 2, 3, 4, 5, 6, 7, 8, 9, 10].nth(10), None);
}

#[test]
fn rev() {
    let a = &Bump::new();
    assert_eq!(sl![].rev(a), sl![]);
    assert_eq!(sl![1].rev(a), sl![1]);
    assert_eq!(sl![1, 2].rev(a), sl![2, 1]);
    assert_eq!(
        sl![1, 2, 3, 4, 5, 6, 7, 8, 9, 10].rev(a),
        sl![10, 9, 8, 7, 6, 5, 4, 3, 2, 1]
    );
}

#[test]
fn append() {
    let a = &Bump::new();
    assert_eq!(sl![].append(sl![], a), sl![]);
    assert_eq!(sl![].append(sl![42], a), sl![42]);
    assert_eq!(sl![42].append(sl![], a), sl![42]);
    assert_eq!(sl![19].append(sl![84], a), sl![19, 84]);
    assert_eq!(sl![19, 84].append(sl![22, 66], a), sl![19, 84, 22, 66]);

    assert_eq!(List::init(2, |x| x, a).append(sl![42], a), sl![0, 1, 42]);
    assert_eq!(List::init(3, |x| x, a).append(sl![42], a), sl![0, 1, 2, 42]);
    assert_eq!(
        List::init(4, |x| x, a).append(sl![42], a),
        sl![0, 1, 2, 3, 42]
    );
    assert_eq!(
        List::init(5, |x| x, a).append(sl![42], a),
        sl![0, 1, 2, 3, 4, 42]
    );
    assert_eq!(
        List::init(6, |x| x, a).append(sl![42], a),
        sl![0, 1, 2, 3, 4, 5, 42]
    );
    assert_eq!(
        List::init(7, |x| x, a).append(sl![42], a),
        sl![0, 1, 2, 3, 4, 5, 6, 42]
    );
    assert_eq!(
        List::init(8, |x| x, a).append(sl![42], a),
        sl![0, 1, 2, 3, 4, 5, 6, 7, 42]
    );
    assert_eq!(
        List::init(9, |x| x, a).append(sl![42], a),
        sl![0, 1, 2, 3, 4, 5, 6, 7, 8, 42]
    );
    assert_eq!(
        List::init(10, |x| x, a).append(sl![42], a),
        sl![0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 42]
    );

    // After appending l1, l2 physically points to l1 as a suffix
    // (i.e., append does not clone the elements of l1).
    let l1 = sl![22, 66];
    let l2 = sl![42].append(l1, a);
    assert_eq!(
        l1.nth(0).unwrap() as *const i32,
        l2.nth(1).unwrap() as *const i32
    );

    // A naive recursive implementation of `append` would blow the stack here.
    assert_eq!(
        List::init(1_000_000, |x| x, a)
            .append(sl![1, 2, 3, 4, 5, 6, 7], a)
            .len(),
        1_000_007
    );
}

#[test]
fn rev_append() {
    let a = &Bump::new();
    assert_eq!(sl![].rev_append(sl![], a), sl![]);
    assert_eq!(sl![].rev_append(sl![42], a), sl![42]);
    assert_eq!(sl![42].rev_append(sl![], a), sl![42]);
    assert_eq!(sl![19].rev_append(sl![84], a), sl![19, 84]);
    assert_eq!(sl![19, 84].rev_append(sl![22, 66], a), sl![84, 19, 22, 66]);

    // After appending l1, l2 physically points to l1 as a suffix
    // (i.e., rev_append does not clone the elements of l1).
    let l1 = sl![22, 66];
    let l2 = sl![42].rev_append(l1, a);
    assert_eq!(
        l1.nth(0).unwrap() as *const i32,
        l2.nth(1).unwrap() as *const i32
    );
}

#[test]
fn init() {
    let a = &Bump::new();
    assert_eq!(List::init(0, |_| 42, a), sl![]);
    assert_eq!(List::init(1, |_| 42, a), sl![42]);
    assert_eq!(List::init(2, |x| x as i32, a), sl![0, 1]);
    assert_eq!(List::init(3, |x| x as i32, a), sl![0, 1, 2]);

    assert_eq!(List::init(100_000, |x| x, a).len(), 100_000);

    // Evaluation order is right to left (i.e., the last element of the list is
    // produced first).
    let mut n = 0;
    #[rustfmt::skip]
    assert_eq!(List::init(0, |_| { n += 1; n }, a), sl![]);
    assert_eq!(n, 0);
    #[rustfmt::skip]
    assert_eq!(List::init(3, |_| { n += 1; n }, a), sl![3, 2, 1]);
    assert_eq!(n, 3);

    assert_eq!(list![in a; 42; 3], sl![42, 42, 42]);
}

#[test]
fn iter() {
    let mut iter = sl![].iter().map(|x| x * x);
    assert_eq!(iter.next(), None);

    let mut iter = sl![1, 2, 3].iter().map(|x| x * x);
    assert_eq!(iter.next(), Some(1));
    assert_eq!(iter.next(), Some(4));
    assert_eq!(iter.next(), Some(9));
    assert_eq!(iter.next(), None);
}

#[test]
fn contains() {
    assert!(!sl![].contains(&42));
    assert!(sl![42].contains(&42));
    assert!(!sl![42].contains(&66));
    assert!(sl![19, 84].contains(&19));
    assert!(sl![19, 84].contains(&84));
    assert!(!sl![19, 84].contains(&42));
}

#[test]
fn find() {
    assert_eq!(sl![].find(|&x| x == 42), None);
    assert_eq!(sl![42].find(|&x| x == 42), Some(&42));
    assert_eq!(sl![42].find(|&x| x == 66), None);
    assert_eq!(sl![19, 84].find(|&x| x == 19), Some(&19));
    assert_eq!(sl![19, 84].find(|&x| x == 84), Some(&84));
    assert_eq!(sl![19, 84].find(|&x| x == 42), None);
}

#[test]
fn rev_from_iter_in() {
    let a = &Bump::new();
    assert_eq!(List::rev_from_iter_in(vec![], a), sl![]);
    assert_eq!(List::rev_from_iter_in(vec![1], a), sl![1]);
    assert_eq!(List::rev_from_iter_in(vec![1, 2], a), sl![2, 1]);
    assert_eq!(List::rev_from_iter_in(vec![1, 2, 3], a), sl![3, 2, 1]);
}

#[test]
fn into_iter() {
    let mut expected = 1;
    for &x in sl![1, 2, 3] {
        assert_eq!(x, expected);
        expected += 1;
    }
}

#[test]
fn debug() {
    assert_eq!(format!("{:?}", sl![]), "[]");
    assert_eq!(format!("{:?}", sl![1, 2, 3]), "[1, 2, 3]");
}
