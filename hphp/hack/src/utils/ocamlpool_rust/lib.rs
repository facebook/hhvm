// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod ocamlvalue;
pub mod utils;

#[macro_export]
macro_rules! caml_raise {
    ($name:ident, |$($param:ident),*|, <$($local:ident),*>, $code:block -> $retval:ident) => {
        ocaml::caml!($name, |$($param),*|, <caml_raise_ret, $($local),*>, {
            let result = std::panic::catch_unwind(
                || {
                    $code;
                    return $retval;
                }
            );
            match result {
                Ok (value) => {
                    caml_raise_ret = value;
                },
                Err (err) => {
                    ocamlpool_leave();
                    let msg: &str;
                    if let Some (str) = err.downcast_ref::<&str>() {
                        msg = str;
                    } else if let Some (string) = err.downcast_ref::<String>() {
                        msg = &string[..];
                    } else {
                        msg = "Unknown panic type, only support string type.";
                    }
                    ocaml::runtime::raise_with_string(
                        &ocaml::named_value("rust exception").unwrap(),
                        msg,
                    );
                },
            };
        } -> caml_raise_ret);
    };
}
