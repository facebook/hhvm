// Copyright (c) Meta Platforms, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
mod diff;
use anyhow::Result;
use clap::Parser;

/// A mult-tool for working with hhdg files
#[derive(Parser, Debug)]
pub enum Command {
    /// Compare two hhdg files and show differences with human readable hash labels
    Diff(diff::Opts),
}

fn main() -> Result<()> {
    match Command::parse() {
        Command::Diff(opts) => {
            let num_different = diff::run(opts)?;
            if num_different != 0 {
                std::process::exit(1);
            }
            Ok(())
        }
    }
}
