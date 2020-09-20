// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use compile_rust as compile;
use ocamlrep::FromOcamlRep;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_ocamlpool::to_ocaml;
use oxidized::relative_path::RelativePath;
use parser_core_types::source_text::SourceText;
use stack_limit::{StackLimit, GI, KI, MI};

use anyhow::{anyhow, Result};
use serde_json::{map::Map, value::Value};
use std::io::Write;

#[derive(Debug, FromOcamlRep)]
struct RustOutputConfig {
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
            let job_builder = move || {
                Box::new(
                    move |stack_limit: &StackLimit, _nomain_stack_size: Option<usize>| {
                        let source_text = unsafe { SourceText::from_ocaml(source_text).unwrap() };
                        let output_config =
                            unsafe { RustOutputConfig::from_ocaml(rust_output_config).unwrap() };
                        let env = unsafe { compile::Env::<OcamlStr>::from_ocaml(env).unwrap() };
                        let mut w = String::new();
                        match compile::from_text_(&env, stack_limit, &mut w, source_text) {
                            Ok(profile) => print_output(w, output_config, &env.filepath, profile),
                            Err(e) => Err(anyhow!("{}", e)),
                        }
                    },
                )
            };
            // Assume peak is 2.5x of stack.
            // This is initial estimation, need to be improved later.
            let stack_slack = |stack_size| stack_size * 6 / 10;
            let on_retry = &mut |stack_size_tried: usize| {
                // Not always printing warning here because this would fail some HHVM tests
                if atty::is(atty::Stream::Stderr) || std::env::var_os("HH_TEST_MODE").is_some() {
                    let source_text = unsafe { SourceText::from_ocaml(source_text).unwrap() };
                    eprintln!(
                        "[hrust] warning: compile_from_text_ffi exceeded stack of {} KiB on: {}",
                        (stack_size_tried - stack_slack(stack_size_tried)) / KI,
                        source_text.file_path().path_str(),
                    );
                }
            };
            let job = stack_limit::retry::Job {
                nonmain_stack_min: 13 * MI,
                // TODO(hrust) aast_parser_ffi only requies 1 * GI, it's like rust compiler produce inconsistent binary.
                nonmain_stack_max: Some(7 * GI),
                ..Default::default()
            };

            let r: Result<(), String> = job
                .with_elastic_stack(&job_builder, on_retry, stack_slack)
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
            let env = unsafe { compile::Env::<OcamlStr>::from_ocaml(env).unwrap() };
            let mut w = String::new();
            compile::emit_fatal_program(&env, &mut w, panic_msg)
                .and_then(|_| print_output(w, output_config, &env.filepath, None))
                .map(|_| unsafe { to_ocaml(&<Result<(), String>>::Ok(())) })
                .map_err(|e| e.to_string())
        },
    )
}

fn print_output(
    bytecode: String,
    config: RustOutputConfig,
    file: &RelativePath,
    profile: Option<compile::Profile>,
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
        if let Some(p) = profile {
            insert(&mut obj, "parsing_time", to_microsec(p.parsing_t));
            insert(&mut obj, "codegen_time", to_microsec(p.codegen_t));
            insert(&mut obj, "printing_time", to_microsec(p.printing_t));
        }
        insert(
            &mut obj,
            "file",
            file.to_absolute()
                .to_str()
                .ok_or_else(|| anyhow!("invalid char in file path"))?,
        );
        insert(&mut obj, "type", "success");
        insert(&mut obj, "bytes", bytecode.as_bytes().len());
        write!(writer, "{}\n", Value::Object(obj))?;
    }

    writer.write_all(bytecode.as_bytes())?;
    writer.flush()?;
    Ok(())
}
