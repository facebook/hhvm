// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Result;
use hhbc_gen::OpcodeData;
use quote::quote;
use std::{
    fs::File,
    io::Write,
    path::PathBuf,
    process::{Command, Stdio},
};
use structopt::StructOpt;

#[derive(StructOpt)]
#[structopt(no_version)]
struct Opts {
    #[structopt(short = "o", long = "out")]
    output: Option<PathBuf>,

    #[structopt(long)]
    no_format: bool,
}

fn main() -> Result<()> {
    let opts = Opts::from_args();

    let mut opcode_data: Vec<OpcodeData> = hhbc_gen::opcode_data().to_vec();
    opcode_data.sort_by(|a: &OpcodeData, b: &OpcodeData| a.name.cmp(b.name));

    let input = quote!(
        #[emit_opcodes_macro::emit_opcodes]
        #[derive(Clone, Debug, Targets)]
        #[repr(C)]
        pub enum Opcode<'arena> {
            // This is filled in by the emit_opcodes macro.  It can be printed using the
            // "//hphp/hack/src/hackc/hhbc:dump-opcodes" binary.
        }
    );

    let opcodes = emit_opcodes::emit_opcodes(input.clone(), &opcode_data)?;
    let targets = emit_opcodes::emit_impl_targets(input, &opcode_data)?;

    let output = format!("{}\n\n{}", opcodes, targets);

    if opts.no_format {
        if let Some(out) = opts.output.as_ref() {
            std::fs::write(out, output)?;
        } else {
            println!("{}", output);
        }
        return Ok(());
    }

    let mut child = Command::new("rustfmt");
    child.args(["--emit", "stdout"]);
    child.stdin(Stdio::piped());

    if let Some(out) = opts.output.as_ref() {
        use std::os::unix::io::{FromRawFd, IntoRawFd};
        let file = File::create(out).expect("couldn't create output file");
        child.stdout(unsafe { Stdio::from_raw_fd(file.into_raw_fd()) });
    }

    let mut child = child.spawn()?;
    child.stdin.as_mut().unwrap().write_all(output.as_bytes())?;
    child.wait()?;

    Ok(())
}
