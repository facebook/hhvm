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
use std::sync::Arc;

use anyhow::Context;
use anyhow::Result;
use clap::Args;
use clap::Parser;
use compile::EnvFlags;
use compile::NativeEnv;
use compile::Profile;
use decl_parser::DeclParser;
use decl_parser::DeclParserOptions;
use decl_provider::ConstDecl;
use decl_provider::DeclProvider;
use decl_provider::Error;
use decl_provider::FunDecl;
use decl_provider::ModuleDecl;
use decl_provider::SelfProvider;
use decl_provider::TypeDecl;
use direct_decl_parser::Decls;
use hackrs_test_utils::serde_store::StoreOpts;
use hackrs_test_utils::store::make_shallow_decl_store;
use hhvm_options::HhvmOptions;
use multifile_rust as multifile;
use naming_provider::SqliteNamingTable;
use options::Hhvm;
use options::ParserOptions;
use parking_lot::Mutex;
use parser_core_types::source_text::SourceText;
use pos::ConstName;
use pos::FunName;
use pos::RelativePathCtx;
use pos::ToOxidized;
use pos::TypeName;
use rayon::prelude::*;
use relative_path::Prefix;
use relative_path::RelativePath;
use shallow_decl_provider::LazyShallowDeclProvider;
use shallow_decl_provider::ShallowDeclProvider;
use tempfile::TempDir;
use ty::reason::NReason;
use ty::reason::Reason;

use crate::util::SyncWrite;
use crate::FileOpts;

#[derive(Args, Debug)]
pub struct Opts {
    /// Output file. Creates it if necessary
    #[clap(short = 'o')]
    output_file: Option<PathBuf>,

    #[command(flatten)]
    files: FileOpts,

    #[command(flatten)]
    single_file_opts: SingleFileOpts,
}

#[derive(Parser, Debug)]
pub(crate) struct SingleFileOpts {
    #[command(flatten)]
    pub(crate) env_flags: EnvFlags,

    #[command(flatten)]
    hhvm_options: HhvmOptions,

    /// Unwrap concurrent blocks as a regular sequence of awaits.
    ///
    /// Currently, used only for infer/textual.
    #[clap(long, action, hide(true))]
    pub(crate) unwrap_concurrent: bool,

    /// The level of verbosity (can be set multiple times)
    #[clap(long = "verbose", action(clap::ArgAction::Count))]
    pub(crate) verbosity: u8,
}

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
                    writeln!(w.lock(), "Error in file {}: {}", f.display(), e)?;
                    Err(e)
                }
                Ok(output) => {
                    w.lock().write_all(&output)?;
                    Ok(())
                }
            }
        })
        .collect();

    results.into_iter().collect()
}

pub(crate) fn native_env(filepath: RelativePath, opts: &SingleFileOpts) -> Result<NativeEnv> {
    let hhvm_options = &opts.hhvm_options;
    let hhvm_config = hhvm_options.to_config()?;
    let parser_options = ParserOptions {
        po_auto_namespace_map: auto_namespace_map().collect(),
        po_unwrap_concurrent: opts.unwrap_concurrent,
        ..hhvm_config::parser_options(&hhvm_config)?
    };
    let hhbc_flags = hhvm_config::hhbc_flags(&hhvm_config)?;

    Ok(NativeEnv {
        filepath,
        hhbc_flags,
        hhvm: Hhvm {
            include_roots: Default::default(),
            renamable_functions: Default::default(),
            non_interceptable_functions: Default::default(),
            parser_options,
            jit_enable_rename_function: hhvm_config::jit_enable_rename_function(&hhvm_config)?,
        },
        flags: opts.env_flags.clone(),
    })
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
    let source_text = SourceText::make(Arc::new(filepath.clone()), &content);
    let env = native_env(filepath, opts)?;
    let mut output = Vec::new();
    let decl_arena = bumpalo::Bump::new();
    let decl_provider = SelfProvider::wrap_existing_provider(
        None,
        env.to_decl_parser_options(),
        source_text.clone(),
        &decl_arena,
    );
    compile::from_text(&mut output, source_text, &env, decl_provider, profile)?;
    if opts.verbosity >= 1 {
        eprintln!("{}: {:#?}", env.filepath.path().display(), profile);
    }
    Ok(output)
}

pub(crate) fn compile_from_text(hackc_opts: &mut crate::Opts, w: &mut impl Write) -> Result<()> {
    let files = hackc_opts.files.gather_input_files()?;
    for path in files {
        let source_text = std::fs::read(&path)
            .with_context(|| format!("Unable to read file '{}'", path.display()))?;
        let env = hackc_opts.native_env(path)?;
        let decl_arena = bumpalo::Bump::new();
        let text = SourceText::make(Arc::new(env.filepath.clone()), &source_text);
        let decl_provider = SelfProvider::wrap_existing_provider(
            None,
            env.to_decl_parser_options(),
            text,
            &decl_arena,
        );
        let hhas = compile_impl(env, source_text, decl_provider)?;
        w.write_all(&hhas)?;
    }
    Ok(())
}

fn compile_impl<'d>(
    env: NativeEnv,
    source_text: Vec<u8>,
    decl_provider: Option<Arc<dyn DeclProvider<'d> + 'd>>,
) -> Result<Vec<u8>> {
    let text = SourceText::make(Arc::new(env.filepath.clone()), &source_text);
    let mut hhas = Vec::new();
    compile::from_text(
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
        let parsed_file = direct_decl_parser::parse_decls_for_bytecode(
            &decl_opts,
            filename,
            &source_text,
            &arena,
        );
        let provider: Arc<SingleDeclProvider<'_, NReason>> = Arc::new(SingleDeclProvider::make(
            &arena,
            parsed_file.decls,
            hackc_opts,
        )?);
        let env = hackc_opts.native_env(path)?;
        let hhas = compile_impl(env, source_text, Some(provider.clone()))?;
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

// TODO (T118266805): get these from nearest .hhconfig enclosing each file.
pub(crate) fn auto_namespace_map() -> impl Iterator<Item = (String, String)> {
    [
        ("Async", "HH\\Lib\\Async"),
        ("C", "FlibSL\\C"),
        ("Dict", "FlibSL\\Dict"),
        ("File", "HH\\Lib\\File"),
        ("IO", "HH\\Lib\\IO"),
        ("Keyset", "FlibSL\\Keyset"),
        ("Locale", "FlibSL\\Locale"),
        ("Math", "FlibSL\\Math"),
        ("OS", "HH\\Lib\\OS"),
        ("PHP", "FlibSL\\PHP"),
        ("PseudoRandom", "FlibSL\\PseudoRandom"),
        ("Regex", "FlibSL\\Regex"),
        ("SecureRandom", "FlibSL\\SecureRandom"),
        ("Str", "FlibSL\\Str"),
        ("Vec", "FlibSL\\Vec"),
    ]
    .into_iter()
    .map(|(k, v)| (k.into(), v.into()))
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
struct SingleDeclProvider<'a, R: Reason> {
    arena: &'a bumpalo::Bump,
    decls: DeclsHolder<'a>,
    shallow_decl_provider: Option<Arc<dyn ShallowDeclProvider<R>>>,
    type_requests: RefCell<Vec<Access>>,
    func_requests: RefCell<Vec<Access>>,
    const_requests: RefCell<Vec<Access>>,
    module_requests: RefCell<Vec<Access>>,
}

impl<'a, R: Reason> DeclProvider<'a> for SingleDeclProvider<'a, R> {
    fn type_decl(&self, symbol: &str, depth: u64) -> Result<TypeDecl<'a>, Error> {
        let decl = {
            let decl = self.find_decl(|decls| decl_provider::find_type_decl(decls, symbol));
            match (&decl, self.shallow_decl_provider.as_ref()) {
                (Ok(_), _) => decl,
                (Err(Error::NotFound), Some(shallow_decl_provider)) => {
                    if let Ok(Some(type_decl)) =
                        shallow_decl_provider.get_type(TypeName::new(symbol))
                    {
                        match type_decl {
                            shallow_decl_provider::TypeDecl::Class(c) => {
                                Ok(decl_provider::TypeDecl::Class(c.to_oxidized(self.arena)))
                            }
                            shallow_decl_provider::TypeDecl::Typedef(ty) => {
                                Ok(decl_provider::TypeDecl::Typedef(ty.to_oxidized(self.arena)))
                            }
                        }
                    } else {
                        decl
                    }
                }
                (_, _) => decl,
            }
        };
        Self::record_access(&self.type_requests, symbol, depth, decl.is_ok());
        decl
    }

    fn func_decl(&self, symbol: &str) -> Result<&'a FunDecl<'a>, Error> {
        let decl = {
            let decl = self.find_decl(|decls| decl_provider::find_func_decl(decls, symbol));
            match (&decl, self.shallow_decl_provider.as_ref()) {
                (Ok(_), _) => decl,
                (Err(Error::NotFound), Some(shallow_decl_provider)) => {
                    if let Ok(Some(fun_decl)) = shallow_decl_provider.get_fun(FunName::new(symbol))
                    {
                        Ok(fun_decl.to_oxidized(self.arena))
                    } else {
                        decl
                    }
                }
                (_, _) => decl,
            }
        };
        Self::record_access(&self.func_requests, symbol, 0, decl.is_ok());
        decl
    }

    fn const_decl(&self, symbol: &str) -> Result<&'a ConstDecl<'a>, Error> {
        let decl = {
            let decl = self.find_decl(|decls| decl_provider::find_const_decl(decls, symbol));
            match (&decl, self.shallow_decl_provider.as_ref()) {
                (Ok(_), _) => decl,
                (Err(Error::NotFound), Some(shallow_decl_provider)) => {
                    if let Ok(Some(const_decl)) =
                        shallow_decl_provider.get_const(ConstName::new(symbol))
                    {
                        Ok(const_decl.to_oxidized(self.arena))
                    } else {
                        decl
                    }
                }
                (_, _) => decl,
            }
        };
        Self::record_access(&self.const_requests, symbol, 0, decl.is_ok());
        decl
    }

    fn module_decl(&self, symbol: &str) -> Result<&'a ModuleDecl<'a>, Error> {
        let decl = self.find_decl(|decls| decl_provider::find_module_decl(decls, symbol));
        // TODO: ShallowDeclProvider doesn't have a get_module equivalent
        Self::record_access(&self.module_requests, symbol, 0, decl.is_ok());
        decl
    }
}

fn make_naming_table_powered_shallow_decl_provider<R: Reason>(
    hackc_opts: &crate::Opts,
) -> Result<impl ShallowDeclProvider<R>> {
    let hhi_root = TempDir::with_prefix("rupro_decl_repo_hhi.")?;
    hhi::write_hhi_files(hhi_root.path()).unwrap();

    let root = hackc_opts
        .naming_table_root
        .as_ref()
        .ok_or_else(|| anyhow::Error::msg("Did not find naming table root path"))?
        .clone();

    let ctx = Arc::new(RelativePathCtx {
        root,
        hhi: hhi_root.path().into(),
        dummy: PathBuf::new(),
        tmp: PathBuf::new(),
    });

    let file_provider: Arc<dyn file_provider::FileProvider> = Arc::new(
        file_provider::DiskProvider::new(Arc::clone(&ctx), Some(hhi_root)),
    );
    let parser = DeclParser::new(
        file_provider,
        DeclParserOptions::default(),
        false, // deregister_php_stdlib
    );

    let shallow_decl_store = make_shallow_decl_store::<R>(StoreOpts::Unserialized);

    let naming_table_path = hackc_opts
        .naming_table
        .as_ref()
        .ok_or_else(|| anyhow::Error::msg("Did not find naming table"))?
        .clone();

    Ok(LazyShallowDeclProvider::new(
        Arc::new(shallow_decl_store),
        Arc::new(SqliteNamingTable::new(naming_table_path).unwrap()),
        parser,
    ))
}

impl<'a, R: Reason> SingleDeclProvider<'a, R> {
    fn make(arena: &'a bumpalo::Bump, decls: Decls<'a>, hackc_opts: &crate::Opts) -> Result<Self> {
        Ok(SingleDeclProvider {
            arena,
            decls: match hackc_opts.use_serialized_decls {
                false => DeclsHolder::ByRef(decls),
                true => DeclsHolder::Ser(decl_provider::serialize_decls(&decls)?),
            },
            shallow_decl_provider: if hackc_opts.naming_table.is_some()
                && hackc_opts.naming_table_root.is_some()
            {
                Some(Arc::new(make_naming_table_powered_shallow_decl_provider(
                    hackc_opts,
                )?))
            } else {
                None
            },
            type_requests: Default::default(),
            func_requests: Default::default(),
            const_requests: Default::default(),
            module_requests: Default::default(),
        })
    }

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
