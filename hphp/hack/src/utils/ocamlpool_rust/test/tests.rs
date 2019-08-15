// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#[macro_use]
extern crate ocaml;

extern crate libc;

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

use ocamlpool_rust::ocamlvalue::*;

macro_rules! test_case {
    ($name:ident, $($code:block)*) => {
        caml!($name, | |, <r>, {
            ocamlpool_enter();
            let list = vec![
                $($code),*
            ];
            r = ocaml::Value::new(list.ocamlvalue());
            ocamlpool_leave();
        } -> r);
    }
}

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
