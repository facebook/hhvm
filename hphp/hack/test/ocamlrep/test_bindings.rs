// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::collections::{BTreeMap, BTreeSet};

use ocamlrep::OcamlRep;
use ocamlrep_derive::OcamlRep;

fn val<T: OcamlRep>(value: T) -> usize {
    let arena = Box::leak(Box::new(ocamlrep::Arena::new()));
    let value = arena.add(&value);
    // Round-trip back to T to exercise from_ocamlrep.
    let value = T::from_ocamlrep(value).unwrap();
    let value = arena.add(&value);
    value.to_bits()
}

#[no_mangle]
pub unsafe extern "C" fn convert_to_ocamlrep(value: usize) -> usize {
    match ocamlrep::slab::OwnedSlab::from_ocaml(value) {
        Some(slab) => slab.leak().to_bits(),
        None => value,
    }
}

#[no_mangle]
pub extern "C" fn realloc_in_ocaml_heap(value: usize) -> usize {
    let slab = unsafe { ocamlrep::slab::OwnedSlab::from_ocaml(value) };
    match slab {
        None => value,
        Some(slab) => {
            let pool = unsafe { ocamlrep_ocamlpool::Pool::new() };
            slab.value().clone_with_allocator(&pool).to_bits()
        }
    }
}

// Primitive Tests

#[no_mangle]
pub extern "C" fn get_a(_unit: usize) -> usize {
    val('a')
}

#[no_mangle]
pub extern "C" fn get_five(_unit: usize) -> usize {
    val(5)
}

#[no_mangle]
pub extern "C" fn get_true(_unit: usize) -> usize {
    val(true)
}

#[no_mangle]
pub extern "C" fn get_false(_unit: usize) -> usize {
    val(false)
}

// Option Tests

#[no_mangle]
pub extern "C" fn get_none(_unit: usize) -> usize {
    val(None::<isize>)
}

#[no_mangle]
pub extern "C" fn get_some_five(_unit: usize) -> usize {
    val(Some(5))
}

#[no_mangle]
pub extern "C" fn get_some_none(_unit: usize) -> usize {
    val(Some(None::<isize>))
}

#[no_mangle]
pub extern "C" fn get_some_some_five(_unit: usize) -> usize {
    val(Some(Some(5)))
}

// Ref tests

#[no_mangle]
pub extern "C" fn get_int_ref(_unit: usize) -> usize {
    val(RefCell::new(5))
}

#[no_mangle]
pub extern "C" fn get_int_option_ref(_unit: usize) -> usize {
    val(RefCell::new(Some(5)))
}

// List Tests

#[no_mangle]
pub extern "C" fn get_empty_list(_unit: usize) -> usize {
    val(Vec::<isize>::new())
}

#[no_mangle]
pub extern "C" fn get_five_list(_unit: usize) -> usize {
    val(vec![5])
}

#[no_mangle]
pub extern "C" fn get_one_two_three_list(_unit: usize) -> usize {
    val(vec![1, 2, 3])
}

#[no_mangle]
pub extern "C" fn get_float_list(_unit: usize) -> usize {
    val(vec![1.0, 2.0, 3.0])
}

// Struct tests

#[derive(OcamlRep)]
struct Foo {
    a: isize,
    b: bool,
}

#[derive(OcamlRep)]
struct Bar {
    c: Foo,
    d: Option<Vec<Option<isize>>>,
}

#[no_mangle]
pub extern "C" fn get_foo(_unit: usize) -> usize {
    val(Foo { a: 25, b: true })
}

#[no_mangle]
pub extern "C" fn get_bar(_unit: usize) -> usize {
    val(Bar {
        c: Foo { a: 42, b: false },
        d: Some(vec![Some(88), None, Some(66)]),
    })
}

// String Tests

#[no_mangle]
pub extern "C" fn get_empty_string(_unit: usize) -> usize {
    val(String::from(""))
}

#[no_mangle]
pub extern "C" fn get_a_string(_unit: usize) -> usize {
    val(String::from("a"))
}

#[no_mangle]
pub extern "C" fn get_ab_string(_unit: usize) -> usize {
    val(String::from("ab"))
}

#[no_mangle]
pub extern "C" fn get_abcde_string(_unit: usize) -> usize {
    val(String::from("abcde"))
}

#[no_mangle]
pub extern "C" fn get_abcdefg_string(_unit: usize) -> usize {
    val(String::from("abcdefg"))
}

#[no_mangle]
pub extern "C" fn get_abcdefgh_string(_unit: usize) -> usize {
    val(String::from("abcdefgh"))
}

#[no_mangle]
pub extern "C" fn get_zero_float(_unit: usize) -> usize {
    val(0.0 as f64)
}

#[no_mangle]
pub extern "C" fn get_one_two_float(_unit: usize) -> usize {
    val(1.2 as f64)
}

// Variant tests

#[derive(OcamlRep)]
enum Fruit {
    Apple,
    Orange(isize),
    Pear { num: isize },
    Kiwi,
}

#[no_mangle]
pub extern "C" fn get_apple(_unit: usize) -> usize {
    val(Fruit::Apple)
}

#[no_mangle]
pub extern "C" fn get_orange(_unit: usize) -> usize {
    val(Fruit::Orange(39))
}

#[no_mangle]
pub extern "C" fn get_pear(_unit: usize) -> usize {
    val(Fruit::Pear { num: 76 })
}

#[no_mangle]
pub extern "C" fn get_kiwi(_unit: usize) -> usize {
    val(Fruit::Kiwi)
}

// Map tests

#[no_mangle]
pub extern "C" fn get_empty_smap(_unit: usize) -> usize {
    let map: BTreeMap<String, isize> = BTreeMap::new();
    val(map)
}

#[no_mangle]
pub extern "C" fn get_int_smap_singleton(_unit: usize) -> usize {
    let mut map = BTreeMap::new();
    map.insert(String::from("a"), 1);
    val(map)
}

#[no_mangle]
pub extern "C" fn get_int_smap(_unit: usize) -> usize {
    let mut map = BTreeMap::new();
    map.insert(String::from("a"), 1);
    map.insert(String::from("b"), 2);
    map.insert(String::from("c"), 3);
    val(map)
}

// Set tests

#[no_mangle]
pub extern "C" fn get_empty_sset(_unit: usize) -> usize {
    let set: BTreeSet<String> = BTreeSet::new();
    val(set)
}

#[no_mangle]
pub extern "C" fn get_sset_singleton(_unit: usize) -> usize {
    let mut set = BTreeSet::new();
    set.insert(String::from("a"));
    val(set)
}

#[no_mangle]
pub extern "C" fn get_sset(_unit: usize) -> usize {
    let mut set = BTreeSet::new();
    set.insert(String::from("a"));
    set.insert(String::from("b"));
    set.insert(String::from("c"));
    val(set)
}
