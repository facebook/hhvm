// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod common;
mod gen_asts;
mod gen_by_ref_decl_visitor;
mod gen_elab_transform;
mod gen_enum_helper;
mod gen_visitor;
mod quote_helper;

use std::fs::File;
use std::io::prelude::*;
use std::path::Path;
use std::path::PathBuf;
use std::process::Command;

use anyhow::Context;
use anyhow::Result;
use clap::Parser;
use common::*;

#[derive(Debug, Parser)]
struct Opts {
    /// Command to regenerate the output. This text will be included in generated file headers.
    #[clap(long)]
    regen_cmd: Option<String>,

    /// Path to a Rust formatter binary, which will be used on the generated output.
    #[clap(long)]
    rustfmt: Option<String>,

    /// The codegen task to run.
    #[clap(subcommand)]
    subcommand: Subcommand,
}

#[derive(Debug, Parser)]
enum Subcommand {
    /// Generate convenient factory functions and predicates for enum types.
    EnumHelpers(gen_enum_helper::Args),
    /// Generate Visitor and VisitorMut traits.
    Visitor(gen_visitor::Args),
    /// Generate AST and NAST modules containing instantiated (non-generic) AAST types.
    Asts(gen_asts::Args),
    /// Generate a Visitor trait for by-reference types.
    ByRefDeclVisitor(gen_by_ref_decl_visitor::Args),
    /// Generate a transformer and Pass trait for AST elaboration.
    ElabTransform(gen_elab_transform::Args),
}

fn main() -> Result<()> {
    let opts = Opts::parse();

    let formatter = opts.rustfmt.as_deref();
    eprintln!("Rust formatter set to {:?}", formatter);

    let regencmd = opts.regen_cmd.as_deref();
    eprintln!("Re-generate cmd set to {:?}", regencmd);

    let files = match opts.subcommand {
        Subcommand::EnumHelpers(args) => gen_enum_helper::run(&args)?,
        Subcommand::Visitor(args) => gen_visitor::run(&args)?,
        Subcommand::Asts(args) => gen_asts::run(&args)?,
        Subcommand::ByRefDeclVisitor(args) => gen_by_ref_decl_visitor::run(&args)?,
        Subcommand::ElabTransform(args) => gen_elab_transform::run(&args)?,
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

fn format(formatter: Option<&str>, file: &Path) -> Result<()> {
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

fn sign(file: &Path) -> Result<()> {
    let contents = std::fs::read(file).context("Failed to read file for signing")?;
    let new_contents = signed_source::sign_file(&contents)?;
    std::fs::write(file, new_contents).context("Failed to write signed file")?;
    Ok(())
}
