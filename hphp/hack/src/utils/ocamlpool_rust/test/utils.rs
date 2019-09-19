// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[macro_export]
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
