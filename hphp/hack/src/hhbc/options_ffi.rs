// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_options::Options;
use ocamlrep_ocamlpool::ocaml_ffi;

ocaml_ffi! {
    fn configs_to_json_ffi(
        jsons: Vec<compile_ffi::OcamlStr>,
        cli_args: Vec<compile_ffi::OcamlStr>,
    ) -> String {
        let jsons: Vec<_> = jsons.into_iter().rev().collect();
        Options::from_configs(&jsons, &cli_args)
            .expect("bug in deserializing Hhbc_options from Rust")
            .to_string()
    }
}
