// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use typing_defs_rust::tast;

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

pub fn print(tast: &tast::Program) {
    unsafe {
        ocamlpool_enter();
        let ocaml_tast = ocamlrep_ocamlpool::add_to_ambient_pool(tast);
        ocamlpool_leave();

        let ocaml_print_tast =
            ocaml::named_value("print_tast_for_rust").expect("print_tast_for_rust not registered");
        let _ = ocaml_print_tast
            .call_n(&[ocaml::Value::new(ocaml_tast)])
            .unwrap();
    }
}
