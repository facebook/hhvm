// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;

#[derive(Debug, structopt::StructOpt)]
pub struct Args {
    #[structopt(flatten)]
    pub common: crate::common::args::Args,

    /// Additional Rust files containing types for which codegen will be performed.
    /// All types reachable from the given root type must be defined in on of
    /// the files provided as `--input` or `--extern-input`.
    #[structopt(short, long, parse(from_os_str))]
    pub extern_input: Vec<PathBuf>,
}
