// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![deny(clippy::mut_from_ref)]

use bumpalo::Bump;

pub trait Arena {
    #[allow(clippy::mut_from_ref)]
    fn alloc<T: TrivialDrop>(&self, val: T) -> &mut T;
}

impl Arena for Bump {
    #[allow(clippy::mut_from_ref)]
    #[inline(always)]
    fn alloc<T: TrivialDrop>(&self, val: T) -> &mut T {
        self.alloc(val)
    }
}

/// Marker trait for types whose implementation of `Drop` is a no-op.
///
/// Used to denote types which can be moved into an arena (which does not drop
/// its contents) without leaking memory.
///
/// Must not be implemented for any type which owns heap memory or otherwise
/// needs to run cleanup in its `Drop` implementation (e.g., `Box`, `Vec`,
/// `std::fs::File`, etc.), or any type containing a field with such a
/// nontrivial `Drop` implementation.
pub trait TrivialDrop {}

impl TrivialDrop for () {}

impl TrivialDrop for bool {}

impl TrivialDrop for usize {}
impl TrivialDrop for u8 {}
impl TrivialDrop for u16 {}
impl TrivialDrop for u32 {}
impl TrivialDrop for u64 {}
impl TrivialDrop for u128 {}

impl TrivialDrop for isize {}
impl TrivialDrop for i8 {}
impl TrivialDrop for i16 {}
impl TrivialDrop for i32 {}
impl TrivialDrop for i64 {}
impl TrivialDrop for i128 {}

impl TrivialDrop for f32 {}
impl TrivialDrop for f64 {}

impl TrivialDrop for str {}
impl TrivialDrop for &str {}

impl<T: TrivialDrop> TrivialDrop for [T] {}
impl<T> TrivialDrop for &[T] {}

impl<T: Sized> TrivialDrop for &T {}

impl<T: TrivialDrop> TrivialDrop for Option<T> {}

impl<T: TrivialDrop, E: TrivialDrop> TrivialDrop for Result<T, E> {}

impl<T0, T1> TrivialDrop for (T0, T1)
where
    T0: TrivialDrop,
    T1: TrivialDrop,
{
}

impl<T0, T1, T2> TrivialDrop for (T0, T1, T2)
where
    T0: TrivialDrop,
    T1: TrivialDrop,
    T2: TrivialDrop,
{
}

impl<T0, T1, T2, T3> TrivialDrop for (T0, T1, T2, T3)
where
    T0: TrivialDrop,
    T1: TrivialDrop,
    T2: TrivialDrop,
    T3: TrivialDrop,
{
}

impl<T0, T1, T2, T3, T4> TrivialDrop for (T0, T1, T2, T3, T4)
where
    T0: TrivialDrop,
    T1: TrivialDrop,
    T2: TrivialDrop,
    T3: TrivialDrop,
    T4: TrivialDrop,
{
}

impl<T0, T1, T2, T3, T4, T5> TrivialDrop for (T0, T1, T2, T3, T4, T5)
where
    T0: TrivialDrop,
    T1: TrivialDrop,
    T2: TrivialDrop,
    T3: TrivialDrop,
    T4: TrivialDrop,
    T5: TrivialDrop,
{
}

impl<T0, T1, T2, T3, T4, T5, T6> TrivialDrop for (T0, T1, T2, T3, T4, T5, T6)
where
    T0: TrivialDrop,
    T1: TrivialDrop,
    T2: TrivialDrop,
    T3: TrivialDrop,
    T4: TrivialDrop,
    T5: TrivialDrop,
    T6: TrivialDrop,
{
}

impl<T0, T1, T2, T3, T4, T5, T6, T7> TrivialDrop for (T0, T1, T2, T3, T4, T5, T6, T7)
where
    T0: TrivialDrop,
    T1: TrivialDrop,
    T2: TrivialDrop,
    T3: TrivialDrop,
    T4: TrivialDrop,
    T5: TrivialDrop,
    T6: TrivialDrop,
    T7: TrivialDrop,
{
}
