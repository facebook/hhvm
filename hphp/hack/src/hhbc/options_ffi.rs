// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep_ocamlpool::ocaml_ffi_no_panic;
use options::Options;

ocaml_ffi_no_panic! {
    fn configs_to_json_ffi(
        jsons: Vec<String>,
        cli_args: Vec<String>,
    ) -> String {
        // FIXME(hrust) ocaml_ffi_no_panic disallows "mut" in parameter
        // jsons.reverse();
        let jsons: Vec<&String> = jsons.iter().rev().collect();

        Options::from_configs(&jsons, &cli_args)
        .expect("bug in deserializing Hhbc_options from Rust")
        .to_string()
    }
}
