// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::{BTreeMap, BTreeSet};
use std::mem;

use ocaml::caml;
use ocamlrep::OcamlRep;
use ocamlrep_derive::OcamlRep;

fn val<T: OcamlRep>(value: T) -> ocaml::Value {
    let arena = ocamlrep::Arena::new_with_size(8);
    let value = value.into_ocamlrep(&arena);
    mem::forget(arena);
    ocaml::Value::new(unsafe { value.as_usize() })
}

caml!(convert_to_ocamlrep, |value|, <result>, {
    let mut arena = ocamlrep::Arena::new_with_size(8);
    let value = arena.add_from_ocaml(value.0);
    mem::forget(arena);
    result = ocaml::Value::new(value.as_usize());
} -> result);

// Primitive Tests

caml!(get_a, |_unit|, <result>, {
    result = val('a');
} -> result);

caml!(get_five, |_unit|, <result>, {
    result = val(5);
} -> result);

caml!(get_true, |_unit|, <result>, {
    result = val(true);
} -> result);

caml!(get_false, |_unit|, <result>, {
    result = val(false);
} -> result);

// Option Tests

caml!(get_none, |_unit|, <result>, {
    result = val(None::<isize>);
} -> result);

caml!(get_some_five, |_unit|, <result>, {
    result = val(Some(5));
} -> result);

caml!(get_some_none, |_unit|, <result>, {
    result = val(Some(None::<isize>));
} -> result);

caml!(get_some_some_five, |_unit|, <result>, {
    result = val(Some(Some(5)));
} -> result);

// List Tests

caml!(get_empty_list, |_unit|, <result>, {
    result = val(Vec::<isize>::new());
} -> result);

caml!(get_five_list, |_unit|, <result>, {
    result = val(vec![5]);
} -> result);

caml!(get_one_two_three_list, |_unit|, <result>, {
    result = val(vec![1, 2, 3]);
} -> result);

caml!(get_float_list, |_unit|, <result>, {
    result = val(vec![1.0, 2.0, 3.0]);
} -> result);

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

caml!(get_foo, |_unit|, <result>, {
    result = val(Foo { a: 25, b: true });
} -> result);

caml!(get_bar, |_unit|, <result>, {
    result = val(Bar {
        c: Foo { a: 42, b: false },
        d: Some(vec![Some(88), None, Some(66)])
    });
} -> result);

// String Tests

caml!(get_empty_string, |_unit|, <result>, {
    result = val("");
} -> result);

caml!(get_a_string, |_unit|, <result>, {
    result = val("a");
} -> result);

caml!(get_ab_string, |_unit|, <result>, {
    result = val("ab");
} -> result);

caml!(get_abcde_string, |_unit|, <result>, {
    result = val(String::from("abcde"));
} -> result);

caml!(get_abcdefg_string, |_unit|, <result>, {
    result = val("abcdefg");
} -> result);

caml!(get_abcdefgh_string, |_unit|, <result>, {
    result = val(String::from("abcdefgh"));
} -> result);

caml!(get_zero_float, |_unit|, <result>, {
    result = val(0.0 as f64);
} -> result);

caml!(get_one_two_float, |_unit|, <result>, {
    result = val(1.2 as f64);
} -> result);

// Variant tests

#[derive(OcamlRep)]
enum Fruit {
    Apple,
    Orange(isize),
    Pear { num: isize },
    Kiwi,
}

caml!(get_apple, |_unit|, <result>, {
    result = val(Fruit::Apple);
} -> result);

caml!(get_orange, |_unit|, <result>, {
    result = val(Fruit::Orange(39));
} -> result);

caml!(get_pear, |_unit|, <result>, {
    result = val(Fruit::Pear { num: 76 });
} -> result);

caml!(get_kiwi, |_unit|, <result>, {
    result = val(Fruit::Kiwi);
} -> result);

// Map tests

caml!(get_empty_smap, |_unit|, <result>, {
    let map: BTreeMap<String, isize> = BTreeMap::new();
    result = val(map);
} -> result);

caml!(get_int_smap_singleton, |_unit|, <result>, {
    let mut map = BTreeMap::new();
    map.insert("a", 1);
    result = val(map);
} -> result);

caml!(get_int_smap, |_unit|, <result>, {
    let mut map = BTreeMap::new();
    map.insert("a", 1);
    map.insert("b", 2);
    map.insert("c", 3);
    result = val(map);
} -> result);

// Set tests

caml!(get_empty_sset, |_unit|, <result>, {
    let set: BTreeSet<String> = BTreeSet::new();
    result = val(set);
} -> result);

caml!(get_sset_singleton, |_unit|, <result>, {
    let mut set = BTreeSet::new();
    set.insert("a");
    result = val(set);
} -> result);

caml!(get_sset, |_unit|, <result>, {
    let mut set = BTreeSet::new();
    set.insert("a");
    set.insert("b");
    set.insert("c");
    result = val(set);
} -> result);
