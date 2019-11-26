// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

extern crate bitflags;

use aast_parser::{rust_aast_parser_types::Env as AastEnv, AastParser};
use ocamlrep::rc::RcOc;
use options::{HhvmFlags, Options};
use oxidized::{namespace_env::Env as NamespaceEnv, relative_path::RelativePath};
use parser_core_types::{indexed_source_text::IndexedSourceText, source_text::SourceText};

use bitflags::bitflags;

/// Common input needed for compilation.  Extra care is taken
/// so that everything is easily serializable at the FFI boundary
/// until the migration from OCaml is fully complete
pub struct Env {
    pub filepath: RelativePath,
    pub empty_namespace: NamespaceEnv,
    pub config_jsons: Vec<String>,
    pub config_list: Vec<String>,
    pub flags: EnvFlags,
}

bitflags! {
    // Note: these flags are intentionally packed into bits to overcome
    // the limitation of to-OCaml FFI functions having at most 5 parameters
    pub struct EnvFlags: u8 {
        const IS_SYSTEMLIB = 1 << 0;
        const IS_EVALED = 1 << 1;
        const FOR_DEBUGGER_EVAL = 1 << 2;
        const DUMP_SYMBOL_REFS = 1 << 3;
    }
}

/// Compilation output. All times are in seconds,
/// except when they are ignored and should not be reported,
/// such as in the case hhvm.log_extern_compiler_perf is false
/// (this avoids the need to read Options from OCaml, as
/// they can be simply returned as NaNs to signal that
/// they should _not_ be passed back as JSON to HHVM process)
#[derive(Debug)]
pub struct Output {
    pub bytecode_segments: Vec<String>,
    pub parsing_t: f64,
    pub codegen_t: f64,
    pub printing_t: f64,
}

// TODO(hrust) switch over to Option<Duration> once FFI is no longer needed;
// however, for FFI it's easier to serialize to floating point in seconds
// and use NaN for ignored (i.e., when hhvm.log_extern_compiler_perf is false)
pub const IGNORED_DURATION: f64 = std::f64::NAN;
pub fn is_ignored_duration(dt: &f64) -> bool {
    dt.is_nan()
}

pub fn from_text(text: &[u8], env: Env) -> Output {
    let opts = Options::from_configs(&env.config_jsons, &env.config_list).unwrap();

    let mut ret = Output {
        bytecode_segments: vec![],
        parsing_t: IGNORED_DURATION,
        codegen_t: IGNORED_DURATION,
        printing_t: IGNORED_DURATION,
    };

    let ast = profile(&opts, &mut ret.parsing_t, || {
        let source_text = SourceText::make(RcOc::new(env.filepath.clone()), text);
        let mut aast_env = AastEnv::default();
        aast_env.keep_errors = true;
        aast_env.show_all_errors = true;
        aast_env.fail_open = true;
        let indexed_source_text = IndexedSourceText::new(source_text);
        AastParser::from_text(&aast_env, &indexed_source_text, None)
            .unwrap()
            .aast
            .unwrap()
    });

    // TODO(hrust) this is just a placeholder for emitted segments
    ret.bytecode_segments.push(format!("{:?}", ast));
    ret
}

fn profile<T, F>(opts: &Options, dt: &mut f64, f: F) -> T
where
    F: FnOnce() -> T,
{
    let t0 = std::time::Instant::now();
    let ret = f();
    *dt = if opts
        .hhvm
        .flags
        .contains(HhvmFlags::LOG_EXTERN_COMPILER_PERF)
    {
        t0.elapsed().as_secs_f64()
    } else {
        IGNORED_DURATION
    };
    ret
}
