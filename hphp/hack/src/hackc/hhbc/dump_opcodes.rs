// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fs::File;
use std::io::Write;
use std::path::PathBuf;
use std::process::Command;
use std::process::Stdio;

use anyhow::Result;
use clap::Parser;
use hhbc_gen::OpcodeData;
use quote::quote;

#[derive(Parser)]
struct Opts {
    #[clap(short = 'o', long = "out")]
    output: Option<PathBuf>,

    #[clap(long)]
    no_format: bool,
}

fn main() -> Result<()> {
    let opts = Opts::parse();
    let opcode_data: Vec<OpcodeData> = hhbc_gen::opcode_data().to_vec();

    let input = quote!(
        #[emit_opcodes_macro::emit_opcodes]
        #[derive(Clone, Debug, Targets)]
        #[repr(C)]
        pub enum Opcode {
            // This is filled in by the emit_opcodes macro.  It can be printed using the
            // "//hphp/hack/src/hackc/hhbc:dump-opcodes" binary.
        }
    );

    let opcodes = emit_opcodes::emit_opcodes(input.clone(), &opcode_data)?;
    let targets = emit_opcodes::emit_impl_targets(input.clone(), &opcode_data)?;
    let flow = emit_opcodes::emit_impl_flow(input.clone(), &opcode_data)?;
    let locals = emit_opcodes::emit_impl_locals(input, &opcode_data)?;

    let output = format!("{}\n\n{}\n\n{}\n\n{}", opcodes, targets, flow, locals);

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
        use std::os::unix::io::FromRawFd;
        use std::os::unix::io::IntoRawFd;
        let file = File::create(out).expect("couldn't create output file");
        child.stdout(unsafe { Stdio::from_raw_fd(file.into_raw_fd()) });
    }

    let mut child = child.spawn()?;
    child.stdin.as_mut().unwrap().write_all(output.as_bytes())?;
    child.wait()?;

    Ok(())
}
