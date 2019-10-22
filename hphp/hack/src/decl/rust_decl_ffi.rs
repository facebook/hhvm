// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[macro_use]
extern crate ocaml;

#[macro_use]
extern crate lazy_static;

use decl_rust::direct_decl_parser::parse_decls;
use ocamlrep::Arena;
use oxidized::relative_path::RelativePath;
use std::collections::HashMap;
use std::sync::Mutex;

lazy_static! {
    static ref PARSE_RESULTS: Mutex<HashMap<String, Arena<'static>>> = Mutex::default();
}

caml!(parse_decls_ffi(
    filename,
    text,
    trace
) {
    let relative_path = RelativePath::from_ocamlvalue(&filename);
    let trace = ocaml::value::Value::isize_val(&trace) != 0;
    let decls = parse_decls(
        &relative_path,
        ocaml::Str::from(text).as_str(),
        trace);
    if trace { println!("Returning {:?}", decls);
}
    let mut arena = Arena::new();
    let value = arena.add(&decls);
    PARSE_RESULTS.lock().unwrap().insert(relative_path.path_str().to_string(), arena);
    ocaml::Value::new(value.to_bits())
});
