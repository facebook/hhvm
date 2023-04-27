// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

ocamlrep_ocamlpool::ocaml_ffi! {
    fn hh_measure_push_global() {
        measure::push_global()
    }
    fn hh_measure_pop_global() -> measure::Record {
        measure::pop_global()
    }
}
