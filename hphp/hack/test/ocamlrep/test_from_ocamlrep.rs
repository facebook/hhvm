// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};

#[cfg(test)]
use ocamlrep::{Allocator, Arena, FromError::*, FromOcamlRep, OpaqueValue, Value};

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
    // SAFETY: The value was allocated by an `Arena`.
    let value = unsafe { Arena::make_transparent(value) };
    let err = isize::from_ocamlrep(value).err().unwrap();
    match err {
        ExpectedImmediate(..) => {}
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
    // SAFETY: The value was allocated by an `Arena`.
    let value = unsafe { Arena::make_transparent(value) };
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
        arena.set_field(&mut foo, 0, OpaqueValue::int(0));
        arena.set_field(&mut foo, 1, OpaqueValue::int(42));
        // SAFETY: `foo` was allocated by an `Arena`.
        unsafe { Arena::make_transparent(foo.build()) }
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
        arena.set_field(&mut foo, 0, OpaqueValue::int(0));
        arena.set_field(&mut foo, 1, OpaqueValue::int(42));
        foo.build()
    };

    let bar = {
        let mut bar = arena.block_with_size_and_tag(2, 0);
        arena.set_field(&mut bar, 0, foo);
        arena.set_field(&mut bar, 1, OpaqueValue::int(0));
        // SAFETY: `bar` was allocated by an `Arena`.
        unsafe { Arena::make_transparent(bar.build()) }
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
    // SAFETY: The value was allocated by an `Arena`.
    let value = unsafe { Arena::make_transparent(value) };
    let err = UnitStruct::from_ocamlrep(value).err().unwrap();
    match err {
        ExpectedImmediate(..) => {}
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
    // SAFETY: The value was allocated by an `Arena`.
    let value = unsafe { Arena::make_transparent(value) };
    let err = Fruit::from_ocamlrep(value).err().unwrap();
    assert_eq!(err, BlockTagOutOfRange { max: 2, actual: 42 });
}

#[test]
fn wrong_block_variant_size() {
    let arena = Arena::new();
    let value = arena.block_with_size_and_tag(42, 0).build();
    // SAFETY: The value was allocated by an `Arena`.
    let value = unsafe { Arena::make_transparent(value) };
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
        arena.set_field(&mut orange, 0, OpaqueValue::int(42));
        // SAFETY: `orange` was allocated by an `Arena`.
        unsafe { Arena::make_transparent(orange.build()) }
    };
    let err = Fruit::from_ocamlrep(orange).err().unwrap();
    assert_eq!(err, ErrorInField(0, Box::new(ExpectedBool(42))));
}

#[test]
fn bad_struct_variant_value() {
    let arena = Arena::new();
    let pear = {
        let mut pear = arena.block_with_size_and_tag(1, 1);
        arena.set_field(&mut pear, 0, OpaqueValue::int(42));
        // SAFETY: `pear` was allocated by an `Arena`.
        unsafe { Arena::make_transparent(pear.build()) }
    };
    let err = Fruit::from_ocamlrep(pear).err().unwrap();
    assert_eq!(err, ErrorInField(0, Box::new(ExpectedBool(42))));
}

#[test]
fn good_boxed_tuple_variant() {
    let arena = Arena::new();
    let peach = {
        let mut peach = arena.block_with_size_and_tag(2, 2);
        arena.set_field(&mut peach, 0, OpaqueValue::int(42));
        arena.set_field(&mut peach, 1, OpaqueValue::int(1));
        // SAFETY: `peach` was allocated by an `Arena`.
        unsafe { Arena::make_transparent(peach.build()) }
    };
    let peach = Fruit::from_ocamlrep(peach);
    assert_eq!(peach, Ok(Fruit::Peach(Box::new((42, true)))));
}
