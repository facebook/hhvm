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
      if std::env::var("HH_LOG_RELATIVE").is_ok() {
          delta_log::init_delta_logger();
      } else {
        env_logger::init_from_env(
            env_logger::Env::default().filter_or(env_logger::DEFAULT_FILTER_ENV, "info"),
        );
      }

    hh_fanout_build::build(allow_empty, incremental, new_edges_dir, delta_file, output.as_ref()).unwrap();
    std::process::exit(0)
  }
}
