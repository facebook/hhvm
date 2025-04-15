// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::FromOcamlRep;
use ocamlrep::ptr::UnsafeOcamlPtr;
use oxidized::full_fidelity_parser_env::FullFidelityParserEnv;

// We don't use the ocaml_ffi! macro here because we want precise
// control over the Pool--when a parse fails, we want to free the old
// pool and create a new one.
#[no_mangle]
pub extern "C" fn parse_positioned_by_ref(ocaml_source_text: usize, env: usize) -> usize {
    ocamlrep_ocamlpool::catch_unwind(|| {
        let ocaml_source_text = unsafe { UnsafeOcamlPtr::new(ocaml_source_text) };
        let env = unsafe { FullFidelityParserEnv::from_ocaml(env).unwrap() };
        rust_parser_ffi::parse(ocaml_source_text, env, |a, s, e| {
            positioned_by_ref_parser::parse_script(a, s, e)
        })
        .as_usize()
    })
}
