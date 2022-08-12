// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cell::RefCell;
use std::fs::File;
use std::io::stdout;
use std::io::Write;
use std::path::Path;
use std::path::PathBuf;
use std::sync::Mutex;

use anyhow::Result;
use clap::Parser;
use compile::EnvFlags;
use compile::HHBCFlags;
use compile::NativeEnv;
use compile::ParserFlags;
use compile::Profile;
use decl_provider::DeclProvider;
use decl_provider::Error;
use decl_provider::TypeDecl;
use direct_decl_parser::Decls;
use multifile_rust as multifile;
use ocamlrep::rc::RcOc;
use oxidized::relative_path::Prefix;
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::shallow_decl_defs::ConstDecl;
use oxidized_by_ref::shallow_decl_defs::FunDecl;
use oxidized_by_ref::shallow_decl_defs::ModuleDecl;
use parser_core_types::source_text::SourceText;
use rayon::prelude::*;

use crate::FileOpts;

#[derive(Parser, Debug)]
pub struct Opts {
    /// Output file. Creates it if necessary
    #[clap(short = 'o')]
    output_file: Option<PathBuf>,

    #[clap(flatten)]
    files: FileOpts,

    #[clap(flatten)]
    single_file_opts: SingleFileOpts,
}

#[derive(Parser, Clone, Debug)]
pub(crate) struct SingleFileOpts {
    /// Disable toplevel definition elaboration
    #[clap(long)]
    pub(crate) disable_toplevel_elaboration: bool,

    /// Dump IR instead of HHAS
    #[clap(long)]
    pub(crate) dump_ir: bool,

    /// Compile files using the IR pass
    #[clap(long)]
    pub(crate) enable_ir: bool,

    /// Treat the files as part of systemlib
    #[clap(long)]
    pub(crate) systemlib: bool,

    /// The level of verbosity (can be set multiple times)
    #[clap(long = "verbose", parse(from_occurrences))]
    pub(crate) verbosity: isize,

    /// [Experimental] Enable Type-Directed Bytecode Compilation
    #[clap(long)]
    pub(crate) type_directed: bool,
}

type SyncWrite = Mutex<Box<dyn Write + Sync + Send>>;

pub fn run(opts: &mut Opts) -> Result<()> {
    if opts.single_file_opts.verbosity > 1 {
        eprintln!("hackc compile options/flags: {:#?}", opts);
    }

    let writer: SyncWrite = match &opts.output_file {
        None => Mutex::new(Box::new(stdout())),
        Some(output_file) => Mutex::new(Box::new(File::create(output_file)?)),
    };

    let files = opts.files.gather_input_files()?;

    // Collect a Vec first so we process all files - not just up to the first failure.
    files
        .into_par_iter()
        .map(|path| process_one_file(&path, opts, &writer))
        .collect::<Vec<_>>()
        .into_iter()
        .collect()
}

/// Process a single physical file by breaking it into multiple logical
/// files (using multifile) and then processing each of those with
/// process_single_file().
///
/// If an error occurs then continue to process as much as possible,
/// returning the first error that occured.
fn process_one_file(f: &Path, opts: &Opts, w: &SyncWrite) -> Result<()> {
    let content = std::fs::read(f)?;
    let files = multifile::to_files(f, content)?;

    // Collect a Vec so we process all files - not just up to the first
    // failure.
    let results: Vec<Result<()>> = files
        .into_iter()
        .map(|(f, content)| {
            let f = f.as_ref();
            match process_single_file(
                &opts.single_file_opts,
                f.into(),
                content,
                &mut Profile::default(),
            ) {
                Err(e) => {
                    writeln!(w.lock().unwrap(), "Error in file {}: {}", f.display(), e)?;
                    Err(e)
                }
                Ok(output) => {
                    w.lock().unwrap().write_all(&output)?;
                    Ok(())
                }
            }
        })
        .collect();

    results.into_iter().collect()
}

pub(crate) fn native_env(filepath: RelativePath, opts: &SingleFileOpts) -> NativeEnv<'_> {
    let mut flags = EnvFlags::empty();
    flags.set(
        EnvFlags::DISABLE_TOPLEVEL_ELABORATION,
        opts.disable_toplevel_elaboration,
    );
    flags.set(EnvFlags::DUMP_IR, opts.dump_ir);
    flags.set(EnvFlags::ENABLE_IR, opts.enable_ir);
    flags.set(EnvFlags::IS_SYSTEMLIB, opts.systemlib);
    flags.set(EnvFlags::TYPE_DIRECTED, opts.type_directed);
    let hhbc_flags = HHBCFlags::EMIT_CLS_METH_POINTERS
        | HHBCFlags::EMIT_METH_CALLER_FUNC_POINTERS
        | HHBCFlags::FOLD_LAZY_CLASS_KEYS
        | HHBCFlags::LOG_EXTERN_COMPILER_PERF;
    let parser_flags = ParserFlags::ENABLE_ENUM_CLASSES;
    NativeEnv {
        filepath,
        hhbc_flags,
        parser_flags,
        flags,
        ..Default::default()
    }
}

pub(crate) fn process_single_file(
    opts: &SingleFileOpts,
    filepath: PathBuf,
    content: Vec<u8>,
    profile: &mut Profile,
) -> Result<Vec<u8>> {
    if opts.verbosity > 1 {
        eprintln!("processing file: {}", filepath.display());
    }
    let filepath = RelativePath::make(Prefix::Dummy, filepath);
    let source_text = SourceText::make(RcOc::new(filepath.clone()), &content);
    let env = native_env(filepath, opts);
    let arena = bumpalo::Bump::new();
    let mut output = Vec::new();
    compile::from_text(&arena, &mut output, source_text, &env, None, profile)?;
    if opts.verbosity >= 1 {
        eprintln!("{}: {:#?}", env.filepath.path().display(), profile);
    }
    Ok(output)
}

pub(crate) fn compile_from_text(hackc_opts: &mut crate::Opts, w: &mut impl Write) -> Result<()> {
    let files = hackc_opts.files.gather_input_files()?;
    for path in files {
        let source_text = std::fs::read(&path)?;
        let env = hackc_opts.native_env(path)?;
        let hhas = compile_impl(env, source_text, None)?;
        w.write_all(&hhas)?;
    }
    Ok(())
}

fn compile_impl<'d>(
    env: NativeEnv<'_>,
    source_text: Vec<u8>,
    decl_provider: Option<&'d dyn DeclProvider>,
) -> Result<Vec<u8>> {
    let text = SourceText::make(RcOc::new(env.filepath.clone()), &source_text);
    let mut hhas = Vec::new();
    let arena = bumpalo::Bump::new();
    compile::from_text(
        &arena,
        &mut hhas,
        text,
        &env,
        decl_provider,
        &mut Default::default(), // profile
    )?;
    Ok(hhas)
}

pub(crate) fn daemon(hackc_opts: &mut crate::Opts) -> Result<()> {
    crate::daemon_loop(|path, w| {
        hackc_opts.files.filenames = vec![path];
        compile_from_text(hackc_opts, w)
    })
}

pub(crate) fn test_decl_compile(hackc_opts: &mut crate::Opts, w: &mut impl Write) -> Result<()> {
    let files = hackc_opts.files.gather_input_files()?;
    for path in files {
        let source_text = std::fs::read(&path)?;

        // Parse decls
        let decl_opts = hackc_opts.decl_opts();
        let filename = RelativePath::make(Prefix::Root, path.clone());
        let arena = bumpalo::Bump::new();
        let parsed_file = direct_decl_parser::parse_decls_without_reference_text(
            &decl_opts,
            filename,
            &source_text,
            &arena,
        );
        let provider = SingleDeclProvider {
            arena: &arena,
            decls: match hackc_opts.use_serialized_decls {
                false => DeclsHolder::ByRef(parsed_file.decls),
                true => DeclsHolder::Ser(decl_provider::serialize_decls(&parsed_file.decls)?),
            },
            type_requests: Default::default(),
            func_requests: Default::default(),
            const_requests: Default::default(),
            module_requests: Default::default(),
        };
        let env = hackc_opts.native_env(path)?;
        let hhas = compile_impl(env, source_text, Some(&provider))?;
        if hackc_opts.log_decls_requested {
            for a in provider.type_requests.borrow().iter() {
                println!("type/{}: {}, {}", a.depth, a.symbol, a.found);
            }
            println!();
        } else {
            w.write_all(&hhas)?;
        }
    }
    Ok(())
}

pub(crate) fn test_decl_compile_daemon(hackc_opts: &mut crate::Opts) -> Result<()> {
    crate::daemon_loop(|path, w| {
        hackc_opts.files.filenames = vec![path];
        test_decl_compile(hackc_opts, w)
    })
}

#[derive(Debug)]
enum DeclsHolder<'a> {
    ByRef(Decls<'a>),
    Ser(Vec<u8>),
}

#[derive(Debug)]
struct Access {
    symbol: String,
    depth: u64,
    found: bool,
}

#[derive(Debug)]
struct SingleDeclProvider<'a> {
    arena: &'a bumpalo::Bump,
    decls: DeclsHolder<'a>,
    type_requests: RefCell<Vec<Access>>,
    func_requests: RefCell<Vec<Access>>,
    const_requests: RefCell<Vec<Access>>,
    module_requests: RefCell<Vec<Access>>,
}

impl<'a> DeclProvider for SingleDeclProvider<'a> {
    fn type_decl(&self, symbol: &str, depth: u64) -> Result<TypeDecl<'a>, Error> {
        let decl = self.find_decl(|decls| decl_provider::find_type_decl(decls, symbol));
        Self::record_access(&self.type_requests, symbol, depth, decl.is_ok());
        decl
    }

    fn func_decl(&self, symbol: &str) -> Result<&'a FunDecl<'a>, Error> {
        let decl = self.find_decl(|decls| decl_provider::find_func_decl(decls, symbol));
        Self::record_access(&self.func_requests, symbol, 0, decl.is_ok());
        decl
    }

    fn const_decl(&self, symbol: &str) -> Result<&'a ConstDecl<'a>, Error> {
        let decl = self.find_decl(|decls| decl_provider::find_const_decl(decls, symbol));
        Self::record_access(&self.const_requests, symbol, 0, decl.is_ok());
        decl
    }

    fn module_decl(&self, symbol: &str) -> Result<&'a ModuleDecl<'a>, Error> {
        let decl = self.find_decl(|decls| decl_provider::find_module_decl(decls, symbol));
        Self::record_access(&self.module_requests, symbol, 0, decl.is_ok());
        decl
    }
}

impl<'a> SingleDeclProvider<'a> {
    fn find_decl<T>(
        &self,
        mut find: impl FnMut(&Decls<'a>) -> Result<T, Error>,
    ) -> Result<T, Error> {
        match &self.decls {
            DeclsHolder::ByRef(decls) => find(decls),
            DeclsHolder::Ser(data) => find(&decl_provider::deserialize_decls(self.arena, data)?),
        }
    }

    fn record_access(log: &RefCell<Vec<Access>>, symbol: &str, depth: u64, found: bool) {
        log.borrow_mut().push(Access {
            symbol: symbol.into(),
            depth,
            found,
        });
    }
}
