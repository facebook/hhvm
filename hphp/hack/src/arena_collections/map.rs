// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::fmt::Debug;
use std::hash::Hash;
use std::hash::Hasher;

use arena_deserializer::impl_deserialize_in_arena;
use arena_trait::Arena;
use arena_trait::TrivialDrop;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

/// The maximum height difference (or balance factor) that is allowed
/// in the implementation of the AVL tree.
const MAX_DELTA: usize = 2;

/// An arena-allocated map.
///
/// The underlying is a balanced tree in which all tree
/// nodes are allocated inside an arena.
///
/// Currently the underlying tree is equivalent to the
/// one used in OCaml's `Map` type.
///
/// Note that the `Option<&'a T>` is optimized to have a size of 1 word.
///
/// Since the whole Map is just a 1 word pointer, it implements the
/// `Copy` trait.

#[derive(Deserialize, Serialize)]
#[serde(bound(
    deserialize = "K: 'de + arena_deserializer::DeserializeInArena<'de>, V: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
#[must_use]
pub struct Map<'a, K, V>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] Option<&'a Node<'a, K, V>>,
);

impl_deserialize_in_arena!(Map<'arena, K, V>);

impl<'a, K, V> TrivialDrop for Map<'a, K, V> {}

impl<'a, K: Debug, V: Debug> Debug for Map<'a, K, V> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_map().entries(self.iter()).finish()
    }
}

/// The derived implementations of Copy and Clone require that K and V be
/// Copy/Clone. We have no such requirement, since Map is just a pointer, so we
/// manually implement them here.
impl<'a, K, V> Clone for Map<'a, K, V> {
    fn clone(&self) -> Self {
        let Map(opt) = self;
        Map(opt.clone())
    }
}

impl<'a, K, V> Copy for Map<'a, K, V> {}

impl<'a, K: PartialEq, V: PartialEq> PartialEq for Map<'a, K, V> {
    fn eq(&self, other: &Self) -> bool {
        self.iter().eq(other.iter())
    }
}

impl<'a, K: Eq, V: Eq> Eq for Map<'a, K, V> {}

impl<K: PartialOrd, V: PartialOrd> PartialOrd for Map<'_, K, V> {
    #[inline]
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        self.iter().partial_cmp(other.iter())
    }
}

impl<K: Ord, V: Ord> Ord for Map<'_, K, V> {
    #[inline]
    fn cmp(&self, other: &Self) -> Ordering {
        self.iter().cmp(other.iter())
    }
}

impl<K: Hash, V: Hash> Hash for Map<'_, K, V> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        for elt in self {
            elt.hash(state);
        }
    }
}

impl<K, V> Default for Map<'_, K, V> {
    fn default() -> Self {
        Map(None)
    }
}

impl<K: ToOcamlRep, V: ToOcamlRep> ToOcamlRep for Map<'_, K, V> {
    fn to_ocamlrep<'a, A: ocamlrep::Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        match self.0 {
            None => alloc.add(&()),
            Some(val) => alloc.add(val),
        }
    }
}

impl<'a, K, V> FromOcamlRepIn<'a> for Map<'a, K, V>
where
    K: FromOcamlRepIn<'a> + TrivialDrop,
    V: FromOcamlRepIn<'a> + TrivialDrop,
{
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        alloc: &'a bumpalo::Bump,
    ) -> Result<Self, ocamlrep::FromError> {
        if value.is_int() {
            let _ = ocamlrep::from::expect_nullary_variant(value, 0)?;
            Ok(Map(None))
        } else {
            Ok(Map(Some(
                alloc.alloc(<Node<'a, K, V>>::from_ocamlrep_in(value, alloc)?),
            )))
        }
    }
}

#[derive(Deserialize, Serialize, ToOcamlRep)]
#[serde(bound(
    deserialize = "K: 'de + arena_deserializer::DeserializeInArena<'de>, V: 'de + arena_deserializer::DeserializeInArena<'de>"
))]
struct Node<'a, K, V>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] Map<'a, K, V>,
    #[serde(deserialize_with = "arena_deserializer::arena")] K,
    #[serde(deserialize_with = "arena_deserializer::arena")] V,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] Map<'a, K, V>,
    usize,
);

impl_deserialize_in_arena!(Node<'arena, K, V>);

impl<'a, K, V> FromOcamlRepIn<'a> for Node<'a, K, V>
where
    K: FromOcamlRepIn<'a> + TrivialDrop,
    V: FromOcamlRepIn<'a> + TrivialDrop,
{
    fn from_ocamlrep_in(
        value: ocamlrep::Value<'_>,
        alloc: &'a bumpalo::Bump,
    ) -> std::result::Result<Self, ocamlrep::FromError> {
        let block = ocamlrep::from::expect_tuple(value, 5)?;
        Ok(Node(
            ocamlrep::from::field_in(block, 0, alloc)?,
            ocamlrep::from::field_in(block, 1, alloc)?,
            ocamlrep::from::field_in(block, 2, alloc)?,
            ocamlrep::from::field_in(block, 3, alloc)?,
            ocamlrep::from::field_in(block, 4, alloc)?,
        ))
    }
}

impl<'a, K: TrivialDrop, V: TrivialDrop> TrivialDrop for Node<'a, K, V> {}

#[macro_export]
macro_rules! map {
  ( ) => ({ Map::empty() });
  ( $arena:expr; $($x:expr => $y:expr),* ) => ({
      let mut temp_map = Map::empty();
      $(
          temp_map = temp_map.add($arena, $x, $y);
      )*
      temp_map
  });
}

impl<'a, K, V> Map<'a, K, V> {
    pub fn keys(&self) -> impl Iterator<Item = &'a K> {
        self.iter().map(|(k, _v)| k)
    }
}

impl<'a, K: Ord, V> Map<'a, K, V> {
    /// Check whether a key is present in the map.
    pub fn mem(self, x: &K) -> bool {
        match self {
            Map(None) => false,
            Map(Some(Node(l, v, _d, r, _h))) => match x.cmp(v) {
                Ordering::Equal => true,
                Ordering::Less => l.mem(x),
                Ordering::Greater => r.mem(x),
            },
        }
    }
}

impl<'a, K, V> Map<'a, K, V> {
    /// Create a new empty map.
    ///
    /// Note that this does not require heap allocation,
    /// as it is equivalent to a 1 word null pointer.
    pub const fn empty() -> Self {
        Map(None)
    }

    /// Compute the number of entries in a map.
    ///
    /// Note that this function takes linear time and logarithmic
    /// stack space in the size of the map.
    pub fn count(self) -> usize {
        match self {
            Map(None) => 0,
            Map(Some(Node(l, _, _, r, _))) => l.count() + 1 + r.count(),
        }
    }
}

impl<'a, K: Ord, V> Map<'a, K, V> {
    /// Check whether the map is empty.
    pub fn is_empty(self) -> bool {
        match self {
            Map(None) => true,
            Map(Some(_)) => false,
        }
    }
}

impl<'a, K: TrivialDrop + Clone + Ord, V: TrivialDrop + Clone> Map<'a, K, V> {
    /// Returns a one-element map.
    pub fn singleton<A: Arena>(arena: &'a A, x: K, d: V) -> Self {
        let node = Node(Map(None), x, d, Map(None), 1);
        return Map(Some(arena.alloc(node)));
    }

    /// Create a map from an iterator.
    pub fn from<A: Arena, I>(arena: &'a A, i: I) -> Self
    where
        I: IntoIterator<Item = (K, V)>,
    {
        let mut m = Self::empty();

        for (k, v) in i {
            m = m.add(arena, k, v);
        }

        m
    }

    /// Returns a pointer the current entry belonging to the key,
    /// or returns None, if no such entry exists.
    pub fn get(self, x: &K) -> Option<&'a V> {
        match self {
            Map(None) => None,
            Map(Some(Node(l, v, d, r, _h))) => match x.cmp(v) {
                Ordering::Equal => Some(d),
                Ordering::Less => l.get(x),
                Ordering::Greater => r.get(x),
            },
        }
    }

    /// Return a map containing the same entries as before,
    /// plus a new entry. If the key was already bound,
    /// its previous entry disappears.
    pub fn add<A: Arena>(self, arena: &'a A, x: K, data: V) -> Self {
        match self {
            Map(None) => {
                let node = Node(Self::empty(), x, data, Self::empty(), 1);
                Map(Some(arena.alloc(node)))
            }
            Map(Some(Node(ref l, v, d, r, h))) => match x.cmp(v) {
                Ordering::Equal => {
                    let node = Node(*l, x, data, *r, *h);
                    Map(Some(arena.alloc(node)))
                }
                Ordering::Less => bal(arena, l.add(arena, x, data), v.clone(), d.clone(), *r),
                Ordering::Greater => bal(arena, *l, v.clone(), d.clone(), r.add(arena, x, data)),
            },
        }
    }

    /// Returns a map containing the same entries as before,
    /// except for the key, which is unbound in the returned map.
    pub fn remove<A: Arena>(self, arena: &'a A, x: &K) -> Self {
        match self {
            Map(None) => Map(None),
            Map(Some(Node(l, v, d, r, _))) => match x.cmp(v) {
                Ordering::Equal => merge(arena, *l, *r),
                Ordering::Less => bal(arena, l.remove(arena, x), v.clone(), d.clone(), *r),
                Ordering::Greater => bal(arena, *l, v.clone(), d.clone(), r.remove(arena, x)),
            },
        }
    }

    pub fn add_all<A: Arena>(self, arena: &'a A, other: Self) -> Self {
        other
            .iter()
            .fold(self, |m, (k, v)| m.add(arena, k.clone(), v.clone()))
    }

    /// Find the minimal key-value entry.
    pub fn min_entry(self) -> Option<(&'a K, &'a V)> {
        match self {
            Map(None) => None,
            Map(Some(Node(l, x, d, _r, _))) => match l {
                Map(None) => Some((x, d)),
                l => l.min_entry(),
            },
        }
    }

    /// Remove the minimal key-value entry.
    pub fn remove_min_entry<A: Arena>(self, arena: &'a A) -> Self {
        match self {
            Map(None) => Map(None),
            Map(Some(Node(l, x, d, r, _))) => match l {
                Map(None) => *r,
                l => bal(arena, l.remove_min_entry(arena), x.clone(), d.clone(), *r),
            },
        }
    }

    /// Find the maximum key-value entry.
    pub fn max_entry(self) -> Option<(&'a K, &'a V)> {
        match self {
            Map(None) => None,
            Map(Some(Node(_l, x, d, r, _))) => match r {
                Map(None) => Some((x, d)),
                r => r.max_entry(),
            },
        }
    }

    /// Remove the maximum key-value entry.
    pub fn remove_max_entry<A: Arena>(self, arena: &'a A) -> Self {
        match self {
            Map(None) => Map(None),
            Map(Some(Node(l, x, d, r, _))) => match r {
                Map(None) => *l,
                r => bal(arena, *l, x.clone(), d.clone(), r.remove_max_entry(arena)),
            },
        }
    }

    /// Set difference. O(n*log(n))
    pub fn diff<A: Arena>(self, arena: &'a A, other: Self) -> Self {
        other
            .into_iter()
            .fold(self, |set, (k, _v)| set.remove(arena, k))
    }
}

impl<'a, K: Clone + Ord, V: Copy> Map<'a, K, V> {
    /// Returns a copy of the current entry belonging to the key,
    /// or returns None, if no such entry exists.
    pub fn find(self, x: &K) -> Option<V> {
        match self {
            Map(None) => None,
            Map(Some(Node(l, v, d, r, _h))) => match x.cmp(v) {
                Ordering::Equal => Some(*d),
                Ordering::Less => l.find(x),
                Ordering::Greater => r.find(x),
            },
        }
    }
}

fn height<'a, K, V>(l: Map<'a, K, V>) -> usize {
    match l {
        Map(None) => 0,
        Map(Some(Node(_, _, _, _, h))) => *h,
    }
}

fn create<'a, A: Arena, K: TrivialDrop, V: TrivialDrop>(
    arena: &'a A,
    l: Map<'a, K, V>,
    x: K,
    v: V,
    r: Map<'a, K, V>,
) -> Map<'a, K, V> {
    let hl = height(l);
    let hr = height(r);
    let h = if hl >= hr { hl + 1 } else { hr + 1 };
    let node = Node(l, x, v, r, h);
    Map(Some(arena.alloc(node)))
}

fn bal<'a, A: Arena, K: TrivialDrop + Clone, V: TrivialDrop + Clone>(
    arena: &'a A,
    l: Map<'a, K, V>,
    x: K,
    d: V,
    r: Map<'a, K, V>,
) -> Map<'a, K, V> {
    let hl = height(l);
    let hr = height(r);
    if hl > hr + MAX_DELTA {
        match l {
            Map(None) => panic!("impossible"),
            Map(Some(Node(ll, lv, ld, lr, _))) => {
                if height(*ll) >= height(*lr) {
                    create(
                        arena,
                        *ll,
                        lv.clone(),
                        ld.clone(),
                        create(arena, *lr, x, d, r),
                    )
                } else {
                    match lr {
                        Map(None) => panic!("impossible"),
                        Map(Some(Node(lrl, lrv, lrd, lrr, _))) => create(
                            arena,
                            create(arena, *ll, lv.clone(), ld.clone(), *lrl),
                            lrv.clone(),
                            lrd.clone(),
                            create(arena, *lrr, x, d, r),
                        ),
                    }
                }
            }
        }
    } else if hr > hl + MAX_DELTA {
        match r {
            Map(None) => panic!("impossible"),
            Map(Some(Node(rl, rv, rd, rr, _))) => {
                if height(*rr) >= height(*rl) {
                    create(
                        arena,
                        create(arena, l, x, d, *rl),
                        rv.clone(),
                        rd.clone(),
                        *rr,
                    )
                } else {
                    match rl {
                        Map(None) => panic!("impossible"),
                        Map(Some(Node(rll, rlv, rld, rlr, _))) => create(
                            arena,
                            create(arena, l, x, d, *rll),
                            rlv.clone(),
                            rld.clone(),
                            create(arena, *rlr, rv.clone(), rd.clone(), *rr),
                        ),
                    }
                }
            }
        }
    } else {
        create(arena, l, x, d, r)
    }
}

fn merge<'a, A: Arena, K: TrivialDrop + Clone + Ord, V: TrivialDrop + Clone>(
    arena: &'a A,
    t1: Map<'a, K, V>,
    t2: Map<'a, K, V>,
) -> Map<'a, K, V> {
    if t1.is_empty() {
        t2
    } else if t2.is_empty() {
        t1
    } else {
        let (x, d) = t2.min_entry().unwrap();
        bal(arena, t1, x.clone(), d.clone(), t2.remove_min_entry(arena))
    }
}

/// Iterator state for map.
pub struct MapIter<'a, K, V> {
    stack: Vec<NodeIter<'a, K, V>>,
}

struct NodeIter<'a, K, V> {
    left_done: bool,
    node: &'a Node<'a, K, V>,
}

impl<'a, K, V> Map<'a, K, V> {
    pub fn iter(&self) -> MapIter<'a, K, V> {
        let stack = match self {
            Map(None) => Vec::new(),
            Map(Some(root)) => vec![NodeIter {
                left_done: false,
                node: root,
            }],
        };
        MapIter { stack }
    }
}

impl<'a, K, V> IntoIterator for &Map<'a, K, V> {
    type Item = (&'a K, &'a V);
    type IntoIter = MapIter<'a, K, V>;

    fn into_iter(self) -> Self::IntoIter {
        self.iter()
    }
}

impl<'a, K, V> Iterator for MapIter<'a, K, V> {
    type Item = (&'a K, &'a V);

    fn next(&mut self) -> Option<Self::Item> {
        let recurse_left = {
            match self.stack.last_mut() {
                None => None,
                Some(n) => {
                    if n.left_done {
                        None
                    } else {
                        n.left_done = true;
                        let Node(Map(l), _, _, _, _) = n.node;
                        *l
                    }
                }
            }
        };
        match recurse_left {
            Some(n) => {
                self.stack.push(NodeIter {
                    left_done: false,
                    node: n,
                });
                self.next()
            }
            None => match self.stack.pop() {
                None => None,
                Some(n) => {
                    if let Node(_, _, _, Map(Some(n)), _) = n.node {
                        self.stack.push(NodeIter {
                            left_done: false,
                            node: n,
                        });
                    }
                    let Node(_, k, v, _, _) = n.node;
                    Some((k, v))
                }
            },
        }
    }
}

#[cfg(test)]
mod tests {
    use bumpalo::Bump;

    use super::*;

    #[test]
    fn test_is_empty() {
        let arena = Bump::new();
        assert!(Map::<i64, i64>::empty().is_empty());
        assert!(!map![&arena; 4 => 9].is_empty());
    }

    #[test]
    fn test_singleton() {
        let a1 = Bump::new();
        let a2 = Bump::new();
        assert_eq!(map![&a1; 1 => 2], map![&a2; 1 => 2]);
    }

    #[test]
    fn test_from() {
        let a = Bump::new();
        assert_eq!(Map::<i64, i64>::from(&a, Vec::new()), map![]);
        assert_eq!(
            Map::<i64, i64>::from(&a, vec![(6, 7), (8, 9)]),
            map![&a; 8 => 9, 6 => 7]
        );
    }
}

#[cfg(test)]
mod tests_arbitrary {
    use std::collections::BTreeMap;
    use std::collections::BTreeSet;
    use std::collections::HashMap;
    use std::hash::Hash;

    use bumpalo::Bump;
    use quickcheck::*;

    use super::*;

    quickcheck! {
        fn prop_mem_find(xs: Vec<(u32, u32)>, ys: Vec<u32>) -> bool {
            let a = Bump::new();
            let m = Map::from(&a, xs.clone());
            let o: HashMap<u32, u32> = xs.into_iter().collect();
            for (k, v) in o.iter() {
                assert!(m.mem(k));
                assert_eq!(m.get(k), Some(v));
            }
            for k in ys {
                  let f = o.contains_key(&k);
                  assert_eq!(m.mem(&k), f);
                  assert_eq!(m.get(&k).is_some(), f);
            }
            true
        }
    }

    #[derive(Clone, Debug)]
    enum Action<T, V> {
        Add(T, V),
        Remove(T),
    }

    #[derive(Clone, Debug)]
    struct ActionSequence<T, V>(Vec<Action<T, V>>);

    struct ActionSequenceShrinker<T, V> {
        seed: ActionSequence<T, V>,
        index: usize,
    }

    impl<T: Arbitrary + Ord + Hash, V: Arbitrary + Eq + Hash> Arbitrary for ActionSequence<T, V> {
        fn arbitrary(g: &mut Gen) -> Self {
            let size = {
                let s = g.size();
                usize::arbitrary(g) % s
            };
            let mut elements: BTreeSet<T> = BTreeSet::new();
            let mut actions: Vec<Action<T, V>> = Vec::with_capacity(size);
            for _ in 0..size {
                let r = f64::arbitrary(g);
                if r < 0.1 {
                    let key: T = Arbitrary::arbitrary(g);
                    elements.remove(&key);
                    actions.push(Action::Remove(key));
                } else if !elements.is_empty() && r < 0.3 {
                    let index = usize::arbitrary(g) % elements.len();
                    let key: T = elements.iter().nth(index).unwrap().clone();
                    elements.remove(&key);
                    actions.push(Action::Remove(key));
                } else {
                    let key: T = Arbitrary::arbitrary(g);
                    elements.insert(key.clone());
                    actions.push(Action::Add(key, Arbitrary::arbitrary(g)));
                }
            }
            ActionSequence(actions)
        }

        fn shrink(&self) -> Box<dyn Iterator<Item = Self>> {
            Box::new(ActionSequenceShrinker {
                seed: self.clone(),
                index: 0,
            })
        }
    }

    impl<T: Clone, V: Clone> Iterator for ActionSequenceShrinker<T, V> {
        type Item = ActionSequence<T, V>;

        fn next(&mut self) -> Option<ActionSequence<T, V>> {
            let ActionSequence(ref actions) = self.seed;
            if self.index > actions.len() {
                None
            } else {
                let actions = actions[..self.index].to_vec();
                self.index += 1;
                Some(ActionSequence(actions))
            }
        }
    }

    fn check_height_invariant<'a, K, V>(m: Map<'a, K, V>) -> bool {
        match m {
            Map(None) => true,
            Map(Some(Node(l, _, _, r, h))) => {
                let lh = height(*l);
                let rh = height(*r);
                let h_exp = if lh > rh { lh + 1 } else { rh + 1 };
                if *h != h_exp {
                    println!("incorrect node height");
                    return false;
                }
                if lh > rh + MAX_DELTA || rh > lh + MAX_DELTA {
                    println!("height difference invariant violated");
                    return false;
                }
                check_height_invariant(*l) && check_height_invariant(*r)
            }
        }
    }

    quickcheck! {
        fn prop_action_seq(actions: ActionSequence<u32, u32>) -> bool {
            let ActionSequence(ref actions) = actions;
            let a = Bump::new();
            let mut m: Map<'_, u32, u32> = Map::empty();
            let mut o: BTreeMap<u32, u32> = BTreeMap::new();
            for action in actions {
                match action {
                   Action::Add(key, value) => {
                       m = m.add(&a, *key, *value);
                       o.insert(*key, *value);
                   }
                   Action::Remove(key) => {
                       m = m.remove(&a, key);
                       o.remove(key);
                   }
                }
            }
            if !m.into_iter().eq(o.iter()) {
                println!("EXPECTED {:?} GOT {:?}", o, m);
                false
            } else {
                check_height_invariant(m)
            }
        }
    }
}

#[cfg(test)]
mod tests_iter {
    use bumpalo::Bump;

    use super::*;

    #[test]
    fn test_iter_manual() {
        assert_eq!(
            Map::<i64, i64>::empty()
                .into_iter()
                .map(|(k, v)| (*k, *v))
                .collect::<Vec<(i64, i64)>>(),
            vec![]
        );

        //      ,5.
        //   ,2.   `6.
        // 1'   4     `7
        //    3'
        let arena = Bump::new();
        let a = &arena;
        let empty = Map::empty();
        let map = create(
            a,
            create(
                a,
                create(a, empty, 1, (), empty),
                2,
                (),
                create(a, create(a, empty, 3, (), empty), 4, (), empty),
            ),
            5,
            (),
            create(a, empty, 6, (), create(a, empty, 7, (), empty)),
        );
        assert_eq!(
            map.into_iter().map(|(k, ())| *k).collect::<Vec<i64>>(),
            vec![1, 2, 3, 4, 5, 6, 7]
        );
    }
}
