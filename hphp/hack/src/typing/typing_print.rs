// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::Env;
use ocamlrep::OcamlRep;
use typing_defs_rust::{InternalType, Ty};

extern "C" {
    fn ocamlpool_enter();
    fn ocamlpool_leave();
}

pub fn debug<'a>(_env: &Env<'a>, ty: Ty<'a>) -> String {
    unsafe {
        // TODO(hrust): The OCaml printing funtion uses the environment
        // to expand type variables and to configure the printer.
        //
        // For now a dummy environment is passed on to the
        // OCaml printing function.
        ocamlpool_enter();
        let ocaml_ty = ocamlrep_ocamlpool::add_to_ambient_pool(&ty);
        ocamlpool_leave();

        let ocaml_debug = ocaml::named_value("typing_print_ffi_debug")
            .expect("typing_print_ffi_debug not registered");
        let ocaml::Value(ocaml_str) = ocaml_debug.call_n(&[ocaml::Value::new(ocaml_ty)]).unwrap();
        return String::from_ocaml(ocaml_str).unwrap();
    }
}

pub fn debug_i<'a>(_env: &Env<'a>, ity: InternalType<'a>) -> String {
    unsafe {
        // TODO(hrust): The OCaml printing funtion uses the environment
        // to expand type variables and to configure the printer.
        //
        // For now a dummy environment is passed on to the
        // OCaml printing function.
        ocamlpool_enter();
        let ocaml_ity = ocamlrep_ocamlpool::add_to_ambient_pool(ity);
        ocamlpool_leave();

        let ocaml_debug_i = ocaml::named_value("typing_print_ffi_debug_i")
            .expect("typing_print_ffi_debug_i not registered");
        let ocaml::Value(ocaml_str) = ocaml_debug_i
            .call_n(&[ocaml::Value::new(ocaml_ity)])
            .unwrap();
        return String::from_ocaml(ocaml_str).unwrap();
    }
}
