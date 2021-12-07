// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod compile;
mod crc;
mod parse;
mod utils;

use ::anyhow::Result;
use structopt::StructOpt;

#[derive(StructOpt, Debug)]
#[structopt(no_version)] // don't consult CARGO_PKG_VERSION (Buck doesn't set it)
pub enum Opts {
    /// Compile one Hack source file or a list of files to HHAS
    Compile(compile::Opts),

    /// Compile Hack source files or directories and produce a single CRC per
    /// input file.
    Crc(crc::Opts),

    /// Parse many files whose filenames are read from stdin
    Parse(parse::Opts),
}

fn main() -> Result<()> {
    env_logger::init();
    let opts = Opts::from_args();
    match opts {
        Opts::Compile(opts) => compile::run(opts),
        Opts::Crc(opts) => crc::run(opts),
        Opts::Parse(opts) => parse::run(opts),
    }
}
