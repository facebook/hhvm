// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::ffi::OsString;

use ocamlrep_ocamlpool::ocaml_ffi;

ocaml_ffi! {
  fn hh_fanout_build_main(
    allow_empty: bool,
    incremental: Option<OsString>,
    new_edges_dir: Option<OsString>,
    delta_file: Option<OsString>,
    output: OsString,
  ) {
    hh_fanout_build::build(allow_empty, incremental, new_edges_dir, delta_file, output).unwrap();
    std::process::exit(0)
  }
}
