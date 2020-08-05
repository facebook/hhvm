// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg(test)]

use std::fmt::Debug;

use bumpalo::Bump;

use ocamlrep::{FromOcamlRepIn, ToOcamlRep};
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};

fn test_round_trip<'a, T>(bump: &'a Bump, rust_value: T)
where
    T: FromOcamlRepIn<'a> + ToOcamlRep + Debug + PartialEq,
{
    let arena = ocamlrep::Arena::new();
    let ocaml_value = arena.add(&rust_value);
    assert_eq!(T::from_ocamlrep_in(ocaml_value, bump), Ok(rust_value));
}

#[test]
fn convert_primitives() {
    let bump = &Bump::new();

    test_round_trip(bump, ());
    test_round_trip(bump, 1isize);
    test_round_trip(bump, 2usize);
    test_round_trip(bump, 3i64);
    test_round_trip(bump, 4u64);
    test_round_trip(bump, 5i32);
    test_round_trip(bump, 6u32);
    test_round_trip(bump, true);
    test_round_trip(bump, false);
    test_round_trip(bump, 'a');
    test_round_trip(bump, 7.7f64);
}

#[test]
fn convert_std_types() {
    let bump = &Bump::new();

    test_round_trip(bump, None::<usize>);
    test_round_trip(bump, Some(&*bump.alloc(5usize)));
    test_round_trip(bump, Ok::<&str, &str>("okay"));
    test_round_trip(bump, Err::<&str, &str>("error"));
}

#[derive(Debug, FromOcamlRepIn, ToOcamlRep, PartialEq)]
struct Foo<'a> {
    bar: &'a usize,
    baz: usize,
}

#[test]
fn convert_struct_with_ref() {
    let bump = &Bump::new();
    test_round_trip(
        bump,
        Foo {
            bar: bump.alloc(3),
            baz: 4,
        },
    );
}

#[derive(Debug, FromOcamlRepIn, ToOcamlRep, PartialEq)]
enum Fruit<'a> {
    Apple,
    Orange(&'a str),
    Pear { is_tasty: bool },
    Kiwi,
    Peach(&'a (isize, bool)),
}

#[test]
fn convert_str_variant() {
    test_round_trip(&Bump::new(), Fruit::Orange("mandarin"));
}

#[test]
fn convert_boxed_tuple_variant() {
    let bump = &Bump::new();
    test_round_trip(bump, Fruit::Peach(bump.alloc((42, true))));
}
