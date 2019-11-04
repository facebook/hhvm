// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#[macro_use]
extern crate clap;

mod common;
mod gen_enum_constr;
mod gen_visitor;

use common::*;
use std::{fs::File, io::prelude::*, path::PathBuf, process::Command};

fn main() -> Result<()> {
    let matches = clap_app!(myapp =>
        (@arg regencmd: --("regen-cmd") +takes_value "command to re-genreate the output, this text will be included in file header")
        (@arg rustfmt: -f --rustfmt +takes_value "Rust formatter")
        (@arg signer: -s --signer +takes_value "file signer")
        (@subcommand enum_constr =>
            (@arg input: -i --input +required +takes_value ... "Rust file contains enums")
            (@arg output: -o --output +takes_value "mod output path")
        )
        (@subcommand visitor =>
            (@arg input: -i --input +required +takes_value ... "Rust file contains all types")
            (@arg root: -r --root +required +takes_value "root type")
            (@arg output: -o --output +takes_value "mod output path")
        )
    )
    .get_matches();

    let formatter = matches.value_of("rustfmt");
    eprintln!("Rust formatter set to {:?}", formatter);

    let signer = matches.value_of("signer");
    eprintln!("Signer set to {:?}", signer);

    let regencmd = matches.value_of("regencmd");
    eprintln!("Re-generate cmd set to {:?}", regencmd);

    let files = match matches.subcommand() {
        ("enum_constr", Some(sub_m)) => gen_enum_constr::run(sub_m)?,
        ("visitor", Some(sub_m)) => gen_visitor::run(sub_m)?,
        _ => vec![],
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
        .map(|o| sign(signer, o))
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

fn sign(signer: Option<&str>, file: &PathBuf) -> Result<()> {
    match signer {
        Some(signer) => {
            let output = Command::new(signer).arg("sign").arg(file).output()?;
            if !output.status.success() {
                eprintln!("signer failed:\n  {:#?}", output);
            }
        }
        _ => eprintln!("Skip: signer not found"),
    }
    Ok(())
}
