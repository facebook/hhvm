// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#[macro_use]
extern crate clap;

mod common;
mod gen_enum_helper;
mod gen_visitor;
mod quote_helper;

use common::*;
use md5::{Digest, Md5};
use std::{fs, fs::File, io::prelude::*, path::PathBuf, process::Command};

fn main() -> Result<()> {
    let matches = clap_app!(myapp =>
        (@arg regencmd: --("regen-cmd") +takes_value "command to re-genreate the output, this text will be included in file header")
        (@arg rustfmt: -f --rustfmt +takes_value "Rust formatter")
        (@subcommand enum_helpers =>
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

    let regencmd = matches.value_of("regencmd");
    eprintln!("Re-generate cmd set to {:?}", regencmd);

    let files = match matches.subcommand() {
        ("enum_helpers", Some(sub_m)) => gen_enum_helper::run(sub_m)?,
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
        let error = "signing failed: input does not contain marker";
        eprintln!("{}", error);
        return Err(error.into());
    }

    let mut digest = Md5::new();
    digest.input(&contents);
    let md5 = hex::encode(digest.result());

    let new_contents =
        contents.replace(&expected, &format!("{} SignedSource<<{}>>", token_tag, md5));
    fs::write(file, new_contents)?;

    Ok(())
}
