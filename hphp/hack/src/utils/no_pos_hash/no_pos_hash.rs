// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub use std::hash::Hasher;

pub use no_pos_hash_derive::NoPosHash;

/// A type for which we can produce a position-insensitive hash.
///
/// For incremental typechecking, we are often interested in
/// determining whether a declaration or AST has changed in a way that
/// requires re-typechecking of dependents. Changes which only affect
/// the `Pos` fields in a declaration do not require rechecking of
/// dependents, so we want to distinguish "position-only" changes from
/// other types of changes.
///
/// In OCaml, we do this by reallocating the old and new declarations
/// (or ASTs) with `Pos.none` in every position field, then performing
/// a polymorphic hash or comparison. In Rust, we could rewrite
/// positions and then use the `Hash` trait, but we'd like to avoid
/// the reallocation/clone (besides, we don't have an endo-visitor for
/// our by-ref types at this time). By comparing the output of hashing
/// with `Hash` and `NoPosHash`, we can easily determine when a value
/// has only changed in positions.
pub trait NoPosHash {
    fn hash<H: Hasher>(&self, state: &mut H);

    fn hash_slice<H: Hasher>(data: &[Self], state: &mut H)
    where
        Self: Sized,
    {
        for piece in data {
            piece.hash(state);
        }
    }
}

pub fn position_insensitive_hash<T: NoPosHash>(value: &T) -> u64 {
    let mut hasher = fnv::FnvHasher::default();
    value.hash(&mut hasher);
    hasher.finish()
}

mod impls {
    use ocamlrep_caml_builtins::Int64;

    use super::*;

    impl<T: NoPosHash> NoPosHash for [T] {
        fn hash<H: Hasher>(&self, state: &mut H) {
            self.len().hash(state);
            NoPosHash::hash_slice(self, state)
        }
    }

    impl<T: NoPosHash> NoPosHash for Option<T> {
        fn hash<H: Hasher>(&self, state: &mut H) {
            match self {
                None => std::mem::discriminant(self).hash(state),
                Some(value) => {
                    std::mem::discriminant(self).hash(state);
                    value.hash(state);
                }
            }
        }
    }

    impl<T> NoPosHash for std::mem::Discriminant<T> {
        fn hash<H: Hasher>(&self, state: &mut H) {
            std::hash::Hash::hash(self, state);
        }
    }

    macro_rules! impl_with_std_hash {
        ($($ty:ty,)*) => {$(
            impl NoPosHash for $ty {
                #[inline]
                fn hash<H: Hasher>(&self, state: &mut H) {
                    std::hash::Hash::hash(self, state);
                }

                #[inline]
                fn hash_slice<H: Hasher>(data: &[$ty], state: &mut H) {
                    std::hash::Hash::hash_slice(data, state);
                }
            }
        )*}
    }

    impl_with_std_hash! {
        u8,
        u16,
        u32,
        u64,
        usize,
        i8,
        i16,
        i32,
        i64,
        isize,
        u128,
        i128,
        bool,
        char,
        String,
        std::path::PathBuf,
        bstr::BString,
        Int64,
    }

    macro_rules! impl_with_std_hash_unsized {
        ($($ty:ty,)*) => {$(
            impl NoPosHash for $ty {
                #[inline]
                fn hash<H: Hasher>(&self, state: &mut H) {
                    std::hash::Hash::hash(self, state);
                }
            }
        )*}
    }

    impl_with_std_hash_unsized! { str, std::path::Path, bstr::BStr, }

    macro_rules! impl_hash_tuple {
        () => (
            impl NoPosHash for () {
                fn hash<H: Hasher>(&self, _state: &mut H) {}
            }
        );

        ( $($name:ident)+) => (
            impl<$($name: NoPosHash),+> NoPosHash for ($($name,)+) where last_type!($($name,)+): ?Sized {
                #[allow(non_snake_case)]
                fn hash<S: Hasher>(&self, state: &mut S) {
                    let ($(ref $name,)+) = *self;
                    $($name.hash(state);)+
                }
            }
        );
    }

    macro_rules! last_type {
        ($a:ident,) => { $a };
        ($a:ident, $($rest_a:ident,)+) => { last_type!($($rest_a,)+) };
    }

    impl_hash_tuple! {}
    impl_hash_tuple! { A }
    impl_hash_tuple! { A B }
    impl_hash_tuple! { A B C }
    impl_hash_tuple! { A B C D }
    impl_hash_tuple! { A B C D E }
    impl_hash_tuple! { A B C D E F }
    impl_hash_tuple! { A B C D E F G }
    impl_hash_tuple! { A B C D E F G H }
    impl_hash_tuple! { A B C D E F G H I }
    impl_hash_tuple! { A B C D E F G H I J }
    impl_hash_tuple! { A B C D E F G H I J K }
    impl_hash_tuple! { A B C D E F G H I J K L }

    macro_rules! impl_with_deref {
        ($(<$($gen:ident $(: $bound:tt)?),* $(,)?> $ty:ty,)*) => {$(
            impl<$($gen: NoPosHash $(+ $bound)*,)*> NoPosHash for $ty {
                #[inline]
                fn hash<H: Hasher>(&self, state: &mut H) {
                    (**self).hash(state);
                }
            }
        )*}
    }

    impl_with_deref! {
        <T: (?Sized)> &T,
        <T: (?Sized)> &mut T,
        <T: (?Sized)> Box<T>,
        <T: (?Sized)> std::rc::Rc<T>,
        <T: (?Sized)> std::sync::Arc<T>,
        <T> ocamlrep::rc::RcOc<T>,
    }

    macro_rules! impl_with_iter {
        ($(<$($gen:ident $(: $bound:tt)?),* $(,)?> $ty:ty,)*) => {$(
            impl<$($gen: NoPosHash $(+ $bound)*,)*> NoPosHash for $ty {
                fn hash<H: Hasher>(&self, state: &mut H) {
                    for element in self.iter() {
                        element.hash(state);
                    }
                }
            }
        )*}
    }

    impl_with_iter! {
        <T> Vec<T>,
        <T> std::collections::HashSet<T>,
        <T> std::collections::BTreeSet<T>,
        <K, V> std::collections::HashMap<K, V>,
        <K, V> std::collections::BTreeMap<K, V>,
        <T> arena_collections::list::List<'_, T>,
        <T> arena_collections::set::Set<'_, T>,
        <T> arena_collections::MultiSet<'_, T>,
        <T> arena_collections::MultiSetMut<'_, T>,
        <T> arena_collections::SortedSet<'_, T>,
        <K, V> arena_collections::map::Map<'_, K, V>,
        <K, V> arena_collections::AssocList<'_, K, V>,
        <K, V> arena_collections::AssocListMut<'_, K, V>,
        <K, V> arena_collections::SortedAssocList<'_, K, V>,
    }
}
