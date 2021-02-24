// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use compile_rust as compile;
use ocamlrep::{rc::RcOc, FromOcamlRep};
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_ocamlpool::to_ocaml;
use oxidized::relative_path::RelativePath;
use parser_core_types::source_text::SourceText;
use stack_limit::{StackLimit, GI, KI, MI};

use anyhow::{anyhow, Result};
use serde_json::{map::Map, value::Value};
use std::io::Write;

use libc::{c_char, c_int};
use log::warn;

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

#[repr(C)]
struct COutputConfig {
    include_header: bool,
    output_file: *const c_char,
}

impl RustOutputConfig {
    /// Returns `None` if `output_config` is null.
    ///
    /// # Safety
    /// * `output_config` must be a valid, aligned pointer to a `COutputConfig`,
    ///   which must not be accessed through another pointer during this function call.
    /// * The COutputConfig's `output_file` must be a valid null-terminated C string or nullptr.
    #[cfg(unix)]
    unsafe fn from_c_output_config(output_config: *const COutputConfig) -> Option<Self> {
        let output_config = output_config.as_ref()?;
        Some(Self {
            include_header: output_config.include_header,
            output_file: if output_config.output_file.is_null() {
                None
            } else {
                std::ffi::CStr::from_ptr(output_config.output_file)
                    .to_str()
                    .ok()
                    .map(|s| s.to_owned())
            },
        })
    }
}

#[repr(C)]
struct CEnv {
    filepath: *const c_char,
    config_jsons: *const *const c_char,
    num_config_jsons: usize,
    config_list: *const *const c_char,
    num_config_list: usize,
    flags: u8,
}

impl CEnv {
    /// Returns `None` if `env` is null.
    ///
    /// # Safety
    /// * `env` must be a valid, aligned pointer to a `CEnv` which is not
    ///   accessed through another pointer for lifetime `'a` (note that this
    ///   lifetime is arbitrarily chosen by the caller)
    /// * Contents of the CEnv must be valid nul-terminated C strings
    ///   containing valid UTF-8, or arrays of same
    #[cfg(unix)]
    pub unsafe fn to_compile_env<'a>(env: *const CEnv) -> Option<compile::Env<&'a str>> {
        use std::os::unix::ffi::OsStrExt;

        let env = env.as_ref()?;

        Some(compile::Env {
            filepath: RelativePath::make(
                oxidized::relative_path::Prefix::Dummy,
                std::path::PathBuf::from(std::ffi::OsStr::from_bytes(
                    std::ffi::CStr::from_ptr(env.filepath).to_bytes(),
                )),
            ),
            config_jsons: cpp_helper::cstr::to_vec(env.config_jsons, env.num_config_jsons),
            config_list: cpp_helper::cstr::to_vec(env.config_list, env.num_config_list),
            flags: compile::EnvFlags::from_bits(env.flags).unwrap(),
        })
    }
}

// Return a result of `compile_from_text_cpp_ffi` to Rust.
#[no_mangle]
unsafe extern "C" fn compile_from_text_free_string_cpp_ffi(s: *mut c_char) {
    // Safety:
    //   - This should only ever be called on a pointer obtained by
    //     `CString::into_raw`.
    //   - `CString::from_raw` and `CString::to_raw` should not be
    //     used with C functions that can modify the string's length.
    let _ = std::ffi::CString::from_raw(s);
}

// Compile to HHAS from source text.
#[no_mangle]
unsafe extern "C" fn compile_from_text_cpp_ffi(
    env: usize,
    source_text: *const c_char,
    output_cfg: usize,
    err_buf: usize,
) -> *const c_char {
    // Safety: We rely on the C caller that `env` can be legitmately
    // reinterpreted as a `*const CBuf` and that on doing so, it is
    // non-null is well aligned and points to a valid properly
    // initialized value.
    let err_buf: &cpp_helper::CBuf = (err_buf as *const cpp_helper::CBuf).as_ref().unwrap();
    let buf_len: c_int = err_buf.buf_len;
    // Safety : We rely on the C caller that `err_buf.buf` be valid for
    // reads and write for `buf_len * mem::sizeof::<u8>()` bytes.
    let buf: &mut [u8] = cpp_helper::cstr::to_mut_u8(err_buf.buf, buf_len);
    // Safety: We rely on the C caller that `output_cfg` can be
    // legitmately reinterpreted as a `*const COutputConfig` and that
    // on doing so, it points to a valid properly initialized value.
    let _output_config: Option<RustOutputConfig> =
        RustOutputConfig::from_c_output_config(output_cfg as *const COutputConfig);
    // Safety: We rely on the C caller that `source_text` be a
    // properly iniitalized null-terminated C string.
    let text: &[u8] = std::ffi::CStr::from_ptr(source_text).to_bytes();

    let job_builder = move || {
        move |stack_limit: &StackLimit, _nomain_stack_size: Option<usize>| {
            // Safety: We rely on the C caller that `env` can be
            // legitmately reinterpreted as a `*const CEnv` and that
            // on doing so, it points to a valid properly initialized
            // value.
            let env: compile::Env<&str> = CEnv::to_compile_env(env as *const CEnv).unwrap();
            let source_text = SourceText::make(RcOc::new(env.filepath.clone()), text);
            let mut w = String::new();
            match compile::from_text_(&env, stack_limit, &mut w, source_text) {
                Ok(_) => {
                    //print_output(w, output_config, &env.filepath, profile)?;
                    Ok(w)
                }
                Err(e) => Err(anyhow!("{}", e)),
            }
        }
    };
    // Assume peak is 2.5x of stack. This is initial estimation, need
    // to be improved later.
    let stack_slack = |stack_size| stack_size * 6 / 10;
    let on_retry = &mut |stack_size_tried: usize| {
        // Not always printing warning here because this would fail
        // some HHVM tests.
        if atty::is(atty::Stream::Stderr) || std::env::var_os("HH_TEST_MODE").is_some() {
            // Safety : We rely on the C caller that `env` can be
            // legitmately reinterpreted as a `*const CEnv` and that
            // on doing so, it points to a valid properly initialized
            // value.
            let env = CEnv::to_compile_env(env as *const CEnv).unwrap();
            eprintln!(
                "[hrust] warning: compile_from_text_ffi exceeded stack of {} KiB on: {}",
                (stack_size_tried - stack_slack(stack_size_tried)) / KI,
                env.filepath.path_str(),
            );
        }
    };
    let job = stack_limit::retry::Job {
        nonmain_stack_min: 13 * MI,
        // TODO(hrust) aast_parser_ffi only requies 1 * GI, it's like
        // rust compiler produce inconsistent binary.
        nonmain_stack_max: Some(7 * GI),
        ..Default::default()
    };

    let r: Result<String, String> = job
        .with_elastic_stack(job_builder, on_retry, stack_slack)
        .map_err(|e| format!("{}", e))
        .expect("compile_ffi: compile_from_text_cpp_ffi: retry failed")
        .map_err(|e| e.to_string());
    match r {
        Ok(out) => {
            let cs = std::ffi::CString::new(out)
                .expect("compile_ffi: compile_from_text_cpp_ffi: String::new failed");
            cs.into_raw() as *const c_char
        }
        Err(e) => {
            if e.len() >= buf.len() {
                warn!("Provided error buffer too is too small.");
                warn!(
                    "Expected at least {} bytes but got {}",
                    e.len() + 1,
                    buf.len()
                );
            } else {
                // Safety:
                //   - `e` must be valid for reads of `e.len() *
                //     size_of::<u8>()` bytes;
                //   - `buf` must be valid for writes of of `e.len() *
                //     size_of::<u8>()` bytes;
                //   - The region of memory beginning at `e` with a
                //     size of of `e.len() * size_of::<u8>()` bytes must
                //     not overlap with the region of memory beginning
                //     at `buf` with the same size;
                //   - Even if the of `e.len() * size_of::<u8>()` is
                //     `0`, the pointers must be non-null and properly
                //     aligned.
                std::ptr::copy_nonoverlapping(e.as_ptr(), buf.as_mut_ptr(), e.len());
                buf[e.len()] = 0;
            }
            std::ptr::null()
        }
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
                }
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
                .with_elastic_stack(job_builder, on_retry, stack_slack)
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

ocamlrep_ocamlpool::ocaml_ffi! {
  fn desugar_and_print_expr_trees(env: compile::Env<OcamlStr>) {
    compile::dump_expr_tree::desugar_and_print(&env);
  }
}
