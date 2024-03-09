// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fs::File;
use std::io::stdout;
use std::path::Path;
use std::path::PathBuf;

use ::assemble as _;
use anyhow::anyhow;
use anyhow::Result;
use clap::Args;
use parking_lot::Mutex;
use rayon::prelude::*;

use crate::util::SyncWrite;
use crate::FileOpts;

#[derive(Args, Debug)]
pub struct Opts {
    /// Output file. Creates it if necessary
    #[clap(short = 'o')]
    output_file: Option<PathBuf>,

    /// The input hhas file(s) to assemble back to ir::Unit
    #[command(flatten)]
    files: FileOpts,
}

pub fn run(opts: Opts) -> Result<()> {
    let writer: SyncWrite = match &opts.output_file {
        None => Mutex::new(Box::new(stdout())),
        Some(output_file) => Mutex::new(Box::new(File::create(output_file)?)),
    };
    let files = opts.files.gather_input_files()?;
    files
        .into_par_iter()
        .map(|path| process_one_file(&path, &writer))
        .collect::<Vec<_>>()
        .into_iter()
        .collect()
}

/// Assemble the hhas in a given file to a hhbc::Unit. Then use bytecode printer
/// to write the hhas representation of that HCU to output.
pub fn process_one_file(f: &Path, w: &SyncWrite) -> Result<()> {
    let unit = ir::assemble::unit_from_path(f)?;
    let mut output = String::new();
    match ir::print::print_unit(&mut output, &unit, false) {
        Err(e) => {
            eprintln!("Error printing file {}: {}", f.display(), e);
            Err(anyhow!("ir::print problem"))
        }
        Ok(_) => {
            w.lock().write_all(output.as_bytes())?;
            Ok(())
        }
    }
}
