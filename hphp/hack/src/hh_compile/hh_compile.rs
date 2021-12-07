// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod compile;
mod parse;
mod utils;

use ::anyhow::Result;
use structopt::StructOpt;

#[derive(StructOpt, Debug)]
#[structopt(no_version)] // don't consult CARGO_PKG_VERSION (Buck doesn't set it)
pub enum Opts {
    /// Compile one Hack source file or a list of files to HHAS
    Compile(compile::Opts),

    /// Parse many files whose filenames are read from stdin
    Parse(parse::Opts),
}

fn main() -> Result<()> {
    let opts = Opts::from_args();
    match opts {
        Opts::Compile(opts) => compile::run(opts),
        Opts::Parse(opts) => parse::run(opts),
    }
}
