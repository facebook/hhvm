// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::PathBuf;

#[derive(Debug, clap::Parser)]
pub struct CommonArgs {
    /// Rust files containing the types for which codegen will be performed.
    /// All types reachable from the given root type must be defined in one of
    /// the files provided as `--input`.
    #[clap(short, long)]
    pub input: Vec<PathBuf>,

    /// The root type of the AST. All types reachable from this type will be
    /// visited by the generated visitor.
    #[clap(short, long)]
    pub root: String,

    /// The directory to which generated files will be written.
    #[clap(short, long)]
    pub output: PathBuf,
}
