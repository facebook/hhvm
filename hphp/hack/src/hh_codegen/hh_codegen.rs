// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod common;
mod gen_by_ref_decl_visitor;
mod gen_by_ref_nast_visitor;
mod gen_enum_helper;
mod gen_visitor;
mod quote_helper;

use anyhow::{anyhow, Result};
use common::*;
use md5::{Digest, Md5};
use std::{fs, fs::File, io::prelude::*, path::PathBuf, process::Command};
use structopt::StructOpt;

#[derive(Debug, StructOpt)]
#[structopt(no_version)] // don't consult CARGO_PKG_VERSION (buck doesn't set it)
struct Opts {
    /// Command to regenerate the output. This text will be included in generated file headers.
    #[structopt(long)]
    regen_cmd: Option<String>,

    /// Path to a Rust formatter binary, which will be used on the generated output.
    #[structopt(long)]
    rustfmt: Option<String>,

    /// The codegen task to run.
    #[structopt(subcommand)]
    subcommand: Subcommand,
}

#[derive(Debug, StructOpt)]
enum Subcommand {
    /// Generate convenient factory functions and predicates for enum types.
    EnumHelpers(gen_enum_helper::Args),
    /// Generate Visitor and VisitorMut traits.
    Visitor(gen_visitor::Args),
    /// Generate a Visitor trait for by-reference types.
    ByRefDeclVisitor(gen_by_ref_decl_visitor::Args),
    /// Generate a Visitor trait for by-reference NASTs.
    ByRefNastVisitor(gen_by_ref_nast_visitor::Args),
}

fn main() -> Result<()> {
    let opts = Opts::from_args();

    let formatter = opts.rustfmt.as_deref();
    eprintln!("Rust formatter set to {:?}", formatter);

    let regencmd = opts.regen_cmd.as_deref();
    eprintln!("Re-generate cmd set to {:?}", regencmd);

    let files = match opts.subcommand {
        Subcommand::EnumHelpers(args) => gen_enum_helper::run(&args)?,
        Subcommand::Visitor(args) => gen_visitor::run(&args)?,
        Subcommand::ByRefDeclVisitor(args) => gen_by_ref_decl_visitor::run(&args)?,
        Subcommand::ByRefNastVisitor(args) => gen_by_ref_nast_visitor::run(&args)?,
    };

    let output_files = files
        .into_iter()
        .map(|f| write_file(f, regencmd))
        .collect::<Result<Vec<_>>>()?;

    if let Err(e) = output_files
        .iter()
        .map(|o| format(formatter, o))
        .collect::<Result<Vec<_>>>()
    {
        eprintln!("formatter failed:\n  {:#?}", e);
    }

    if let Err(e) = output_files
        .iter()
        .map(|o| sign(o))
        .collect::<Result<Vec<_>>>()
    {
        eprintln!("signer failed:\n  {:#?}", e);
    }
    Ok(())
}

fn write_file(output: (PathBuf, String), regencmd: Option<&str>) -> Result<PathBuf> {
    let mut file = File::create(&output.0)?;
    let content = insert_header(&output.1[..], regencmd.unwrap_or(""))?;
    file.write_all(content.as_bytes())?;
    Ok(output.0)
}

fn format(formatter: Option<&str>, file: &PathBuf) -> Result<()> {
    match formatter {
        Some(formatter) => {
            let output = Command::new(formatter).arg(file).output()?;
            if !output.status.success() {
                eprintln!("formatter failed:\n  {:#?}", output);
            }
        }
        _ => eprintln!("Skip: formatter not found"),
    }
    Ok(())
}

fn sign(file: &PathBuf) -> Result<()> {
    // avoid putting the obvious literal in this source file, as that makes the
    // file as generated
    let token_tag = format!("@{}", "generated");
    let token = "<<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>";
    let expected = format!("{} {}", token_tag, token);

    let contents = fs::read_to_string(file).expect("signing failed: could not read file");
    if !contents.contains(&expected) {
        return Err(anyhow!("signing failed: input does not contain marker"));
    }

    let mut digest = Md5::new();
    digest.input(&contents);
    let md5 = hex::encode(digest.result());

    let new_contents =
        contents.replace(&expected, &format!("{} SignedSource<<{}>>", token_tag, md5));
    fs::write(file, new_contents)?;

    Ok(())
}
