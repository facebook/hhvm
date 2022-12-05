// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![cfg(test)]

use ocamlrep::Allocator;
use ocamlrep::Arena;
use ocamlrep::FromError::*;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use ocamlrep::Value;

#[test]
fn expected_block_but_got_int() {
    let value = Value::int(42);
    let err = <(isize, isize)>::from_ocamlrep(value).err().unwrap();
    assert_eq!(err, ExpectedBlock(42));
}

#[test]
fn expected_int_but_got_block() {
    let arena = Arena::new();
    let value = arena.block_with_size_and_tag(1, 0).build();
    let err = isize::from_ocamlrep(value).err().unwrap();
    match err {
        ExpectedInt(..) => {}
        _ => panic!("unexpected error: {}", err.to_string()),
    }
}

#[test]
fn wrong_tag_for_none() {
    let value = Value::int(1);
    let err = <Option<isize>>::from_ocamlrep(value).err().unwrap();
    assert_eq!(err, NullaryVariantTagOutOfRange { max: 0, actual: 1 });
}

#[test]
fn wrong_tag_for_some() {
    let arena = Arena::new();
    let value = arena.block_with_size_and_tag(1, 1).build();
    let err = <Option<isize>>::from_ocamlrep(value).err().unwrap();
    assert_eq!(
        err,
        ExpectedBlockTag {
            expected: 0,
            actual: 1
        }
    );
}

#[test]
fn out_of_bool_range() {
    let value = Value::int(42);
    let err = bool::from_ocamlrep(value).err().unwrap();
    assert_eq!(err, ExpectedBool(42));
}

#[test]
fn out_of_char_range() {
    let value = Value::int(-1);
    let err = char::from_ocamlrep(value).err().unwrap();
    assert_eq!(err, ExpectedChar(-1));
}

#[derive(FromOcamlRep, ToOcamlRep)]
struct Foo {
    a: isize,
    b: bool,
}

#[test]
fn bad_struct_field() {
    let arena = Arena::new();
    let value = {
        let mut foo = arena.block_with_size_and_tag(2, 0);
        arena.set_field(&mut foo, 0, Value::int(0));
        arena.set_field(&mut foo, 1, Value::int(42));
        foo.build()
    };
    let err = Foo::from_ocamlrep(value).err().unwrap();
    assert_eq!(err, ErrorInField(1, Box::new(ExpectedBool(42))));
}

#[derive(FromOcamlRep, ToOcamlRep)]
struct Bar {
    c: Foo,
    d: Option<Vec<Option<isize>>>,
}

#[test]
fn bad_nested_struct_field() {
    let arena = Arena::new();

    let foo = {
        let mut foo = arena.block_with_size_and_tag(2, 0);
        arena.set_field(&mut foo, 0, Value::int(0));
        arena.set_field(&mut foo, 1, Value::int(42));
        foo.build()
    };

    let bar = {
        let mut bar = arena.block_with_size_and_tag(2, 0);
        arena.set_field(&mut bar, 0, foo);
        arena.set_field(&mut bar, 1, Value::int(0));
        bar.build()
    };

    let err = Bar::from_ocamlrep(bar).err().unwrap();
    assert_eq!(
        err,
        ErrorInField(0, Box::new(ErrorInField(1, Box::new(ExpectedBool(42)))))
    );
}

#[derive(FromOcamlRep, ToOcamlRep)]
struct UnitStruct;

#[test]
fn expected_unit_struct_but_got_nonzero() {
    let value = Value::int(42);
    let err = UnitStruct::from_ocamlrep(value).err().unwrap();
    assert_eq!(err, ExpectedUnit(42));
}

#[test]
fn expected_unit_struct_but_got_block() {
    let arena = Arena::new();
    let value = arena.block_with_size_and_tag(1, 0).build();
    let err = UnitStruct::from_ocamlrep(value).err().unwrap();
    match err {
        ExpectedInt(..) => {}
        _ => panic!("unexpected error: {}", err.to_string()),
    }
}

#[derive(FromOcamlRep, ToOcamlRep)]
struct WrapperStruct(bool);

#[test]
fn bad_value_in_wrapper_struct() {
    let value = Value::int(42);
    let err = WrapperStruct::from_ocamlrep(value).err().unwrap();
    assert_eq!(err, ExpectedBool(42))
}

#[derive(Debug, PartialEq, FromOcamlRep, ToOcamlRep)]
enum Fruit {
    Apple,
    Orange(bool),
    Pear { is_tasty: bool },
    Kiwi,
    Peach(Box<(isize, bool)>),
}

#[test]
fn nullary_variant_tag_out_of_range() {
    let value = Value::int(42);
    let err = Fruit::from_ocamlrep(value).err().unwrap();
    assert_eq!(err, NullaryVariantTagOutOfRange { max: 1, actual: 42 });
}

#[test]
fn block_variant_tag_out_of_range() {
    let arena = Arena::new();
    let value = arena.block_with_size_and_tag(1, 42).build();
    let err = Fruit::from_ocamlrep(value).err().unwrap();
    assert_eq!(err, BlockTagOutOfRange { max: 2, actual: 42 });
}

#[test]
fn wrong_block_variant_size() {
    let arena = Arena::new();
    let value = arena.block_with_size_and_tag(42, 0).build();
    let err = Fruit::from_ocamlrep(value).err().unwrap();
    assert_eq!(
        err,
        WrongBlockSize {
            expected: 1,
            actual: 42
        }
    );
}

#[test]
fn bad_tuple_variant_value() {
    let arena = Arena::new();
    let orange = {
        let mut orange = arena.block_with_size_and_tag(1, 0);
        arena.set_field(&mut orange, 0, Value::int(42));
        orange.build()
    };
    let err = Fruit::from_ocamlrep(orange).err().unwrap();
    assert_eq!(err, ErrorInField(0, Box::new(ExpectedBool(42))));
}

#[test]
fn bad_struct_variant_value() {
    let arena = Arena::new();
    let pear = {
        let mut pear = arena.block_with_size_and_tag(1, 1);
        arena.set_field(&mut pear, 0, Value::int(42));
        pear.build()
    };
    let err = Fruit::from_ocamlrep(pear).err().unwrap();
    assert_eq!(err, ErrorInField(0, Box::new(ExpectedBool(42))));
}

#[test]
fn good_boxed_tuple_variant() {
    let arena = Arena::new();
    let peach = {
        let mut peach = arena.block_with_size_and_tag(2, 2);
        arena.set_field(&mut peach, 0, Value::int(42));
        arena.set_field(&mut peach, 1, Value::int(1));
        peach.build()
    };
    let peach = Fruit::from_ocamlrep(peach);
    assert_eq!(peach, Ok(Fruit::Peach(Box::new((42, true)))));
}

#[test]
fn round_trip_through_ocaml_value_unsigned_int() {
    let num = 7334234036144964024u64;
    let value = Value::int(num as isize);
    let num_int: isize = ocamlrep::from::expect_int(value).ok().unwrap();
    let num_uint: usize =
        !(1usize << 63) & ocamlrep::from::expect_int(value).ok().unwrap() as usize;

    assert!(num_int < 0);
    assert_eq!((num_int as u64) & !(1 << 63), num);
    assert_eq!(num_uint as u64, num);
}
