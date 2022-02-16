// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use decl_provider::NoDeclProvider;
use ocamlrep::FromOcamlRep;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_ocamlpool::to_ocaml;
use oxidized::relative_path::RelativePath;
use parser_core_types::source_text::SourceText;

use anyhow::{anyhow, Result};
use serde_json::{map::Map, value::Value};
use std::io::Write;

#[derive(Debug, FromOcamlRep)]
pub struct RustOutputConfig {
    include_header: bool,
    output_file: Option<String>,
}

pub struct OcamlStr<'content>(&'content [u8]);

impl<'content> AsRef<str> for OcamlStr<'content> {
    fn as_ref(&self) -> &str {
        unsafe { std::str::from_utf8_unchecked(self.0) }
    }
}

impl<'content> FromOcamlRep for OcamlStr<'content> {
    fn from_ocamlrep(value: ocamlrep::Value<'_>) -> Result<Self, ocamlrep::FromError> {
        Ok(Self(unsafe {
            std::mem::transmute(ocamlrep::bytes_from_ocamlrep(value)?)
        }))
    }
}

#[no_mangle]
extern "C" fn compile_from_text_ffi(
    env: usize,
    rust_output_config: usize,
    source_text: usize,
) -> usize {
    ocamlrep_ocamlpool::catch_unwind_with_handler(
        || {
            let r: Result<(), String> = stack_limit::with_elastic_stack(|stack_limit| {
                let source_text = unsafe { SourceText::from_ocaml(source_text).unwrap() };
                let output_config =
                    unsafe { RustOutputConfig::from_ocaml(rust_output_config).unwrap() };
                let env = unsafe { compile::Env::<OcamlStr<'_>>::from_ocaml(env).unwrap() };
                let mut w = Vec::new();
                let alloc = bumpalo::Bump::new();
                match compile::from_text(
                    &alloc,
                    &env,
                    stack_limit,
                    &mut w,
                    source_text,
                    None,
                    &NoDeclProvider,
                ) {
                    Ok(profile) => print_output(
                        w,
                        output_config,
                        &env.filepath,
                        profile.map(|p| (p.parsing_t, p.codegen_t, p.parsing_t)),
                    ),
                    Err(e) => Err(anyhow!("{}", e)),
                }
            })
            .map_err(|e| format!("{}", e))
            .expect("Retry Failed")
            .map_err(|e| e.to_string());
            unsafe { to_ocaml(&r) }
        },
        // This handler is to catch `panic` from parser,
        // TODO(hrust): parser shouldn't panic instead it should return result
        // and then revert this diff.
        |panic_msg: &str| -> Result<usize, String> {
            let output_config =
                unsafe { RustOutputConfig::from_ocaml(rust_output_config).unwrap() };

            let mut w = Vec::new();
            let env = unsafe { compile::Env::<OcamlStr<'_>>::from_ocaml(env).unwrap() };
            compile::emit_fatal_unit(&env, &mut w, panic_msg)
                .and_then(|_| print_output(w, output_config, &env.filepath, None))
                .map(|_| unsafe { to_ocaml(&<Result<(), String>>::Ok(())) })
                .map_err(|e| e.to_string())
        },
    )
}

fn print_output(
    bytecode: Vec<u8>,
    config: RustOutputConfig,
    file: &RelativePath,
    // TODO:(shiqicao) change following tuple to Profile after hhbc remove
    profile: Option<(f64, f64, f64)>,
) -> Result<()> {
    fn insert(o: &mut Map<String, Value>, k: impl Into<String>, v: impl Into<Value>) {
        o.insert(k.into(), v.into());
    }

    let mut writer: Box<dyn Write> = match config.output_file {
        Some(file) => Box::new(std::fs::File::create(file)?),
        None => Box::new(std::io::stdout()),
    };
    if config.include_header {
        let mut obj = Map::new();
        let to_microsec = |x| (x * 1_000_000.0) as u64;
        if let Some((parsing_t, codegen_t, printing_t)) = profile {
            insert(&mut obj, "parsing_time", to_microsec(parsing_t));
            insert(&mut obj, "codegen_time", to_microsec(codegen_t));
            insert(&mut obj, "printing_time", to_microsec(printing_t));
        }
        insert(
            &mut obj,
            "file",
            file.to_absolute()
                .to_str()
                .ok_or_else(|| anyhow!("invalid char in file path"))?,
        );
        insert(&mut obj, "type", "success");
        insert(&mut obj, "bytes", bytecode.len());
        write!(writer, "{}\n", Value::Object(obj))?;
    }

    writer.write_all(&bytecode)?;
    writer.flush()?;
    Ok(())
}

ocamlrep_ocamlpool::ocaml_ffi! {
  fn desugar_and_print_expr_trees(env: compile::Env<OcamlStr<'_>>) {
    compile::dump_expr_tree::desugar_and_print(&env);
  }
}
