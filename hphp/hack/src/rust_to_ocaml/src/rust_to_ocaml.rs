// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[macro_use]
mod macros;

mod config;
mod convert;
mod ir;
mod rewrite_module_names;
mod rewrite_types;

use std::io::Write;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Context;
use anyhow::Result;

use crate::config::Config;

#[derive(Debug, clap::Parser)]
struct Opts {
    /// The Hack source file to convert.
    #[clap(value_name("FILEPATH"))]
    filename: PathBuf,

    /// The OCaml source file to generate.
    #[clap(value_name("OUTPATH"))]
    out_path: Option<PathBuf>,

    /// Path to a configuration file.
    #[clap(long)]
    config: Option<PathBuf>,

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
    let opts = <Opts as clap::Parser>::parse();

    let config = Box::leak(Box::new(match opts.config {
        Some(path) => {
            let contents = std::fs::read_to_string(&path)
                .with_context(|| format!("Failed to read config file at {}", path.display()))?;
            toml::from_str(&contents)
                .with_context(|| format!("Failed to parse config file at {}", path.display()))?
        }
        None => Config::default(),
    }));

    let src = std::fs::read_to_string(&opts.filename)
        .with_context(|| format!("Failed to read input file {}", opts.filename.display()))?;
    let file = syn::parse_file(&src)?;
    let mut ocaml_src = convert::convert_file(config, &opts.filename, &file)?;

    if !opts.no_header {
        ocaml_src = attach_header(opts.regen_cmd.as_deref(), &ocaml_src);
    }
    let absolute_filename = opts.filename.canonicalize()?;
    let mut ocaml_src = ocamlformat(
        opts.formatter.as_deref(),
        opts.out_path
            .as_deref()
            .and_then(Path::parent)
            .or_else(|| absolute_filename.parent()),
        ocaml_src.into_bytes(),
    )
    .context("failed to run ocamlformat")?;
    if !opts.no_header {
        ocaml_src = signed_source::sign_file(&ocaml_src)?;
    }

    if let Some(out_path) = &opts.out_path {
        std::fs::write(out_path, &ocaml_src)?;
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

fn ocamlformat(
    formatter: Option<&str>,
    out_dir: Option<&Path>,
    contents: Vec<u8>,
) -> Result<Vec<u8>> {
    let formatter = match formatter {
        None => return Ok(contents),
        Some(f) => f,
    };
    // Even if we format the file on disk (i.e., at `opts.out_path`),
    // ocamlformat won't look for an .ocamlformat file in the directory
    // containing the file. It only looks up from the current working directory.
    // There's a --root arg, but it doesn't seem to produce the same behavior.
    let prev_dir = std::env::current_dir()?;
    if let Some(out_dir) = out_dir {
        std::env::set_current_dir(out_dir)?;
    }
    let mut child = std::process::Command::new(formatter)
        .stdin(std::process::Stdio::piped())
        .stdout(std::process::Stdio::piped())
        .stderr(std::process::Stdio::piped())
        // In the event that an .ocamlformat file is still not available, tell
        // ocamlformat to please format it anyway.
        .arg("--enable-outside-detected-project")
        .arg("--impl")
        .arg("-")
        .spawn()?;
    let child_stdin = child.stdin.as_mut().unwrap();
    child_stdin.write_all(&contents)?;
    let output = child.wait_with_output()?;
    if !output.status.success() {
        anyhow::bail!("Formatter failed:\n{:#?}", output);
    }
    if out_dir.is_some() {
        std::env::set_current_dir(prev_dir)?;
    }
    Ok(output.stdout)
}
