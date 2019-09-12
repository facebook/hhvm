// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

use ocaml::caml;
use ocaml_ffi_test_utils::test_case;
use ocamlpool_rust::ocamlvalue::*;
use ocamlvalue_macro::Ocamlvalue;

test_case!(
    getString,
    { "".to_string() }
    { "TEST".to_string() }
);

test_case!(
    getIsize,
    {-1isize}
    {0isize}
    {1isize}
    {::std::isize::MAX >> 1}
);

test_case!(
    getOption,
    { None }
    { Some(1isize)}
);

test_case!(
    getVec,
    { vec![] }
    { vec![0isize]}
);

test_case!(
    getBool,
    { true }
    { false }
);

test_case!(
    getBox,
    { Box::new("".to_string())}
    { Box::new("A".to_string())}
);

#[derive(Ocamlvalue)]
enum Foo1 {
    AA,
    BB(bool, String),
    CC,
    DD(isize),
}

#[derive(Ocamlvalue)]
struct Foo2(bool);

#[derive(Ocamlvalue)]
struct Foo3 {
    aa: isize,
    bb: bool,
}

#[derive(Ocamlvalue)]
struct Foo4(bool, String);

test_case!(
    getFoo1,
    { Foo1::AA }
    { Foo1::BB(true, "A".to_string())}
    { Foo1::CC }
    { Foo1::DD(2isize) }
);

test_case!(
    getFoo2,
    { Foo2(true) }
    { Foo2(false) }
);

test_case!(getFoo3, {
    Foo3 {
        aa: 2isize,
        bb: true,
    }
});

test_case!(getFoo4, { Foo4(true, "C".to_string()) });

test_case!(getStrRef,
    {""}
    { "TEST" }
);

test_case!(getRc, { std::rc::Rc::new(Foo2(true)) });

test_case!(
    getPathBuf,
    { std::path::Path::new("foo.txt").to_path_buf() }
    { std::path::Path::new("").to_path_buf() }
);

test_case!(getEmptyStringMap, {
    std::collections::BTreeMap::<String, String>::new()
});

test_case!(getSingletonStringMap, {
    {
        let mut res = std::collections::BTreeMap::<String, String>::new();
        res.insert("1".to_string(), "one".to_string());
        res
    }
});

test_case!(getStringMap, {
    {
        let mut map = std::collections::BTreeMap::<String, String>::new();
        map.insert("1".to_string(), "a".to_string());
        map.insert("2".to_string(), "b".to_string());
        map.insert("3".to_string(), "c".to_string());
        map
    }
});
