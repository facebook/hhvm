// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use anyhow::{Context, Result};
use clap::Parser;
use hdf::Value;
use std::path::PathBuf;

/// A tool for auditing HDF
#[derive(Parser, Debug)]
pub struct Opts {
    files: Vec<PathBuf>,
}

fn main() -> Result<()> {
    let opts = Opts::parse();
    let mut bad: Vec<anyhow::Error> = Default::default();
    for path in opts.files {
        match Value::from_file(&path).with_context(|| path.display().to_string()) {
            Ok(v) => print!("{:?}", v),
            Err(e) => bad.push(e),
        }
    }
    for e in bad {
        eprintln!("{:#}", e);
    }
    Ok(())
}
