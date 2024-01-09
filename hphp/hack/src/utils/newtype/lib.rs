// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

mod idhasher;

use std::hash::Hash;
use std::hash::Hasher;

pub use idhasher::BuildIdHasher;
use serde::Deserialize;
use serde::Serialize;

pub trait HasNone: Copy {
    const NONE: Self;
}

pub trait FromUsize {
    fn from_usize(u: usize) -> Self;
}

/// Define a macro that makes building integer-based newtypes easy.
///
/// Using a newtype for integer based types can be useful to use the typechecker
/// to make sure that different contextual types of ints aren't accidentally
/// mixed.
///
/// In addition to building the newtype HashMap and HashSet types are created
/// which take the newtype as a key.  These use BuildIdHasher to produce good
/// hash values for integers - which is not the case for the default hasher.
///
/// Usage:
///     newtype_int!(NAME, BASIS, MAP-NAME, SET-NAME)
///
///     NAME - The name used to create the newtype.
///     BASIS - The basis type that the newtype wraps (such as i64, u32, etc)
///     MAP-NAME - The name used to create the HashMap.
///     SET-NAME - The name used to create the HashSet.
///
/// The newtype created has this signature:
///
///   struct NAME(pub BASIS);
///
///   impl NAME {
///     pub fn as_usize(&self) -> usize;
///     pub fn from_usize(usize) -> Self;
///   }
///
///   impl Clone for NAME;
///   impl Copy for NAME;
///   impl Debug for NAME;
///   impl Default for NAME;
///   impl Display for NAME;
///   impl Eq for NAME;
///   impl From<BASIS> for NAME;
///   impl FromUsize for NAME;
///   impl HasNone for NAME;
///   impl Hash for NAME;
///   impl Ord for NAME;
///   impl PartialEq for NAME;
///   impl PartialOrd for NAME;
///
///   impl From<NAME> for usize;
#[macro_export]
macro_rules! newtype_int {
    ($name:ident, $num:ident, $hashmap:ident, $hashset:ident $(, $derive:ident)*) => {
        #[derive(Copy, Clone, Debug, Eq, PartialEq, Hash, PartialOrd, Ord $(, $derive)*)]
        #[repr(transparent)]
        pub struct $name(pub $num);

        impl $name {
            #[inline]
            pub fn as_usize(&self) -> usize {
                self.0 as usize
            }

            #[inline]
            pub fn from_usize(u: usize) -> Self {
                Self(u as $num)
            }

            pub const NONE: Self = Self(std::$num::MAX);
        }

        impl $crate::FromUsize for $name {
            fn from_usize(u: usize) -> Self {
                Self::from_usize(u)
            }
        }

        impl std::convert::From<$num> for $name {
            fn from(x: $num) -> $name {
                $name(x)
            }
        }

        impl std::convert::From<$name> for usize {
            fn from(id: $name) -> Self {
                id.0 as usize
            }
        }

        impl $crate::HasNone for $name {
            const NONE: Self = Self(std::$num::MAX);
        }

        impl std::fmt::Display for $name {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                write!(f, "{}({})", stringify!($name), self.0)
            }
        }

        impl std::default::Default for $name {
            fn default() -> Self {
                Self::NONE
            }
        }

        // Unfortunately Rust does not yet let you make associated generic types,
        // so these need to be given top-level names here.
        pub type $hashmap<V> =
            std::collections::HashMap<$name, V, $crate::BuildIdHasher<$num>>;
        pub type $hashset =
            std::collections::HashSet<$name, $crate::BuildIdHasher<$num>>;
    };
}

/// A Vec indexable by a newtype_int.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct IdVec<N: Into<usize>, T> {
    pub vec: Vec<T>,
    phantom: core::marker::PhantomData<N>,
}

impl<N: Into<usize>, T> IdVec<N, T> {
    pub fn new() -> IdVec<N, T> {
        IdVec::new_from_vec(Vec::new())
    }

    pub fn new_from_vec(vec: Vec<T>) -> IdVec<N, T> {
        IdVec {
            vec,
            phantom: Default::default(),
        }
    }

    pub fn with_capacity(cap: usize) -> IdVec<N, T> {
        IdVec {
            vec: Vec::with_capacity(cap),
            phantom: Default::default(),
        }
    }

    pub fn get(&self, index: N) -> Option<&T> {
        self.vec.get(index.into())
    }

    pub fn get_mut(&mut self, index: N) -> Option<&mut T> {
        self.vec.get_mut(index.into())
    }

    pub fn capacity_bytes(&self) -> usize {
        std::mem::size_of::<T>() * self.vec.capacity()
    }

    pub fn swap(&mut self, a: N, b: N) {
        self.vec.swap(a.into(), b.into());
    }
}

impl<N: FromUsize + Into<usize>, T> IdVec<N, T> {
    pub fn keys(&self) -> impl DoubleEndedIterator<Item = N> + '_ {
        (0..self.vec.len()).into_iter().map(|i| N::from_usize(i))
    }

    pub fn push(&mut self, v: T) -> N {
        let id = N::from_usize(self.len());
        self.vec.push(v);
        id
    }
}

impl<N: Into<usize>, T: PartialEq> PartialEq for IdVec<N, T> {
    fn eq(&self, rhs: &Self) -> bool {
        self.vec.eq(&rhs.vec)
    }
}

impl<N: Into<usize>, T: Eq> Eq for IdVec<N, T> {}

impl<N: Into<usize>, T: Hash> Hash for IdVec<N, T> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.vec.hash(state)
    }
}

impl<N: Into<usize>, T> Default for IdVec<N, T> {
    fn default() -> Self {
        IdVec::new_from_vec(Vec::new())
    }
}

impl<N: Into<usize>, T> IntoIterator for IdVec<N, T> {
    type Item = T;
    type IntoIter = ::std::vec::IntoIter<T>;

    fn into_iter(self) -> Self::IntoIter {
        self.vec.into_iter()
    }
}

impl<N: Into<usize>, T> FromIterator<T> for IdVec<N, T> {
    fn from_iter<I: IntoIterator<Item = T>>(iter: I) -> Self {
        IdVec {
            vec: Vec::from_iter(iter),
            phantom: Default::default(),
        }
    }
}

impl<N: Into<usize>, T> std::ops::Index<N> for IdVec<N, T> {
    type Output = T;
    fn index(&self, i: N) -> &'_ T {
        self.vec.index(i.into())
    }
}

impl<N: Into<usize>, T> std::ops::IndexMut<N> for IdVec<N, T> {
    fn index_mut(&mut self, i: N) -> &'_ mut T {
        self.vec.index_mut(i.into())
    }
}

impl<N: Into<usize>, T> std::ops::Deref for IdVec<N, T> {
    type Target = Vec<T>;

    #[inline]
    fn deref(&self) -> &Vec<T> {
        &self.vec
    }
}

impl<N: Into<usize>, T> std::ops::DerefMut for IdVec<N, T> {
    #[inline]
    fn deref_mut(&mut self) -> &mut Vec<T> {
        &mut self.vec
    }
}

#[cfg(test)]
mod tests {
    newtype_int!(MyId, u32, MyIdMap, MyIdSet);

    #[test]
    fn test_newtype() {
        let id = MyId::from_usize(42);
        assert_eq!(id.as_usize(), 42);
        assert_eq!(format!("{}", id), "MyId(42)");

        let id_set: MyIdSet = [
            MyId::from_usize(1),
            MyId::from_usize(2),
            MyId::from_usize(3),
            MyId::from_usize(2),
        ]
        .iter()
        .copied()
        .collect();
        assert_eq!(id_set.len(), 3);

        let id_map: MyIdMap<&str> = [(MyId::from_usize(1), "a"), (MyId::from_usize(2), "b")]
            .iter()
            .copied()
            .collect();
        assert_eq!(id_map.get(&MyId::from_usize(1)).copied(), Some("a"));
        assert_eq!(id_map.get(&MyId::from_usize(2)).copied(), Some("b"));
    }
}
