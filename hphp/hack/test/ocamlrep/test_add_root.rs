// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg(test)]

use ocamlrep::{Allocator, Arena, FromOcamlRep, ToOcamlRep};

#[test]
fn shared_str() {
    // Without `add_root`, converting this tuple would convert the string
    // "hello" to its OCaml representation and copy it into the ocamlrep::Arena
    // twice.
    let arena = Arena::new();
    let s = "hello";
    let tuple = (s, s);
    let ocaml_tuple = arena.add_root(&tuple);
    let ocaml_block = ocaml_tuple.as_block().unwrap();

    assert_eq!(
        ocaml_block[0].as_str(),
        Some(std::borrow::Cow::Borrowed("hello"))
    );
    assert_eq!(
        ocaml_block[1].as_str(),
        Some(std::borrow::Cow::Borrowed("hello"))
    );

    // The string pointer in the first field is physically equal to the string
    // pointer in the second field.
    assert_eq!(ocaml_block[0].to_bits(), ocaml_block[1].to_bits());
}

#[test]
fn shared_slice() {
    // Without `add_root`, converting this tuple would convert the list to its
    // OCaml representation and copy it into the ocamlrep::Arena twice.
    let arena = Arena::new();
    let s = &[1usize, 2, 3][..];
    let tuple = (s, s);
    let ocaml_tuple = arena.add_root(&tuple);

    assert_eq!(
        <(Vec<usize>, Vec<usize>)>::from_ocamlrep(ocaml_tuple),
        Ok((vec![1, 2, 3], vec![1, 2, 3]))
    );

    // The list pointer in the first field is physically equal to the list
    // pointer in the second field.
    let ocaml_block = ocaml_tuple.as_block().unwrap();
    assert_eq!(ocaml_block[0].to_bits(), ocaml_block[1].to_bits());
}

#[test]
fn overlapping_substrs() {
    // Without `Allocator::memoized_slice`, a naive implementation of
    // `ToOcamlRep` for slices might use `Allocator::memoized`, failing to take
    // the slice length into consideration.
    // Then we'd incorrectly produce the tuple ("hello", "hello").
    let arena = Arena::new();
    let s1 = "hello";
    let s2 = &s1[..4];
    let tuple = (s1, s2);
    let ocaml_tuple = arena.add_root(&tuple);

    assert_eq!(
        <(String, String)>::from_ocamlrep(ocaml_tuple),
        Ok((String::from("hello"), String::from("hell")))
    );
}

#[test]
fn overlapping_subslices() {
    // Without `Allocator::memoized_slice`, a naive implementation of
    // `ToOcamlRep` for slices might use `Allocator::memoized`, failing to take
    // the slice length into consideration.
    // Then we'd incorrectly produce the tuple ([1,2,3], [1,2,3]).
    let arena = Arena::new();
    let s1 = &[1usize, 2, 3][..];
    let s2 = &s1[..2];
    let tuple = (s1, s2);
    let ocaml_tuple = arena.add_root(&tuple);

    assert_eq!(
        <(Vec<usize>, Vec<usize>)>::from_ocamlrep(ocaml_tuple),
        Ok((vec![1, 2, 3], vec![1, 2]))
    );
}

#[derive(Debug, PartialEq)]
#[repr(transparent)]
struct U32Pair(u64);

impl U32Pair {
    fn new(fst: u32, snd: u32) -> Self {
        let fst = fst as u64;
        let snd = snd as u64;
        Self(fst << 32 | snd)
    }
    fn fst(&self) -> u32 {
        (self.0 >> 32) as u32
    }
    fn snd(&self) -> u32 {
        self.0 as u32
    }
    fn inner(&self) -> &u64 {
        &self.0
    }
}
impl ToOcamlRep for U32Pair {
    fn to_ocamlrep<'a, A: Allocator>(&'a self, alloc: &'a A) -> ocamlrep::OpaqueValue<'a> {
        let mut block = alloc.block_with_size(2);
        alloc.set_field(&mut block, 0, alloc.add_copy(self.fst()));
        alloc.set_field(&mut block, 1, alloc.add_copy(self.snd()));
        block.build()
    }
}
impl FromOcamlRep for U32Pair {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        let (fst, snd) = <(u32, u32)>::from_ocamlrep(value)?;
        Ok(Self::new(fst, snd))
    }
}

#[test]
fn differently_typed_views_of_same_data() {
    // `Allocator::memoized` is keyed solely off of address and size in bytes.
    // If we have two views of the same bytes, but the views have two different
    // OCaml representations, then `Allocator::add_root` will produce an OCaml
    // value with an unexpected type.
    //
    // Here, `pair_as_int` gets converted first, and memoized. Since its address
    // and size are the same as `pair`, the allocator uses the same memoized
    // OCaml value for both. But the implementations of `ToOcamlRep` and
    // `FromOcamlRep` for U32Pair specify that the OCaml representation is a
    // tuple, not an immediate integer, so we encounter an error when trying to
    // convert the OCaml value to `(u64, U32Pair)`.
    let arena = Arena::new();
    let pair = &U32Pair::new(1, 2);
    let pair_as_int = pair.inner();
    let value = (pair_as_int, pair);

    use ocamlrep::FromError::*;
    assert_eq!(
        <(u64, U32Pair)>::from_ocamlrep(arena.add_root(&value)),
        Err(ErrorInField(1, Box::new(ExpectedBlock(1 << 32 | 2))))
    );

    // Using arena.add instead produces a correct result.
    assert_eq!(
        <(u64, U32Pair)>::from_ocamlrep(arena.add(&value)),
        Ok((1 << 32 | 2, U32Pair::new(1, 2)))
    );
}
