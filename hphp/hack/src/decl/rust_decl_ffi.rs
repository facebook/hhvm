// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_rust::direct_decl_parser::parse_decls;
use ocamlrep::OcamlRep;
use ocamlrep_ocamlpool::to_ocaml;
use oxidized::relative_path::RelativePath;

#[no_mangle]
pub extern "C" fn parse_decls_ffi(filename: usize, text: usize, trace: usize) -> usize {
    let filename = unsafe { RelativePath::from_ocaml(filename).unwrap() };
    let text = unsafe { String::from_ocaml(text).unwrap() };
    let trace = unsafe { bool::from_ocaml(trace).unwrap() };
    let decls = parse_decls(&filename, &text, trace);
    if trace {
        println!("Returning {:?}", decls);
    }
    to_ocaml(&decls)
}
