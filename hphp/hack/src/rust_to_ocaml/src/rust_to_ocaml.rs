// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod convert;
mod ir;
mod rewrite_types;

use std::io::Write;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Result;

#[derive(Debug, clap::Parser)]
struct Opts {
    /// The Hack source file to convert.
    #[clap(value_name("FILEPATH"))]
    filename: PathBuf,

    /// The OCaml source file to generate.
    #[clap(value_name("OUTPATH"))]
    out_path: Option<PathBuf>,

    /// Command to regenerate the output. This text will be included in generated file headers.
    #[clap(long)]
    regen_cmd: Option<String>,

    /// Do not add copyright header and generated tag (for tests).
    #[clap(long)]
    no_header: bool,

    /// Path to an OCaml formatter binary which will be used on the generated output.
    #[clap(long)]
    formatter: Option<String>,
}

fn main() -> Result<()> {
    let opts = <Opts as clap::Parser>::from_args();

    let src = std::fs::read_to_string(&opts.filename)?;
    let file = syn::parse_file(&src)?;
    let mut ocaml_src = convert::convert_file(&opts.filename, &file)?;

    if !opts.no_header {
        ocaml_src = attach_header(opts.regen_cmd.as_deref(), &ocaml_src);
    }
    let mut ocaml_src = ocamlformat(opts.formatter.as_deref(), ocaml_src.into_bytes())?;
    if !opts.no_header {
        ocaml_src = signed_source::sign_file(&ocaml_src)?;
    }

    if let Some(out_path) = &opts.out_path {
        write_file(out_path, &ocaml_src)?;
    } else {
        let mut stdout = std::io::stdout().lock();
        stdout.write_all(&ocaml_src)?;
    }

    Ok(())
}

fn attach_header(regen_cmd: Option<&str>, contents: &str) -> String {
    let regen_cmd = regen_cmd.map_or_else(String::new, |regen_cmd| {
        format!(" *\n * To regenerate this file, run:\n *   {}\n", regen_cmd)
    });
    format!(
        r#"(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 * {}
{} *)

{}"#,
        signed_source::SIGNING_TOKEN,
        regen_cmd,
        contents
    )
}

fn ocamlformat(formatter: Option<&str>, contents: Vec<u8>) -> Result<Vec<u8>> {
    let formatter = match formatter {
        None => return Ok(contents),
        Some(f) => f,
    };
    let mut child = std::process::Command::new(formatter)
        .stdin(std::process::Stdio::piped())
        .stdout(std::process::Stdio::piped())
        .arg("--impl")
        .arg("-")
        .spawn()?;
    let child_stdin = child.stdin.as_mut().unwrap();
    child_stdin.write_all(&contents)?;
    let output = child.wait_with_output()?;
    if !output.status.success() {
        anyhow::bail!("Formatter failed:\n{:#?}", output);
    }
    Ok(output.stdout)
}

fn write_file(path: &Path, contents: &[u8]) -> Result<()> {
    let mut file = std::fs::File::create(path)?;
    file.write_all(contents)?;
    Ok(())
}
