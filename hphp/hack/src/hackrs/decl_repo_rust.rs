// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;
use std::path::PathBuf;
use std::sync::Arc;

use decl_parser::DeclParser;
use decl_parser::DeclParserOptions;
use folded_decl_provider::FoldedDeclProvider;
use hackrs_test_utils::decl_provider::make_folded_decl_provider;
use hackrs_test_utils::serde_store::Compression;
use hackrs_test_utils::serde_store::StoreOpts;
use hackrs_test_utils::store::make_shallow_decl_store;
use hackrs_test_utils::store::populate_shallow_decl_store;
use indicatif::ParallelProgressIterator;
use jwalk::WalkDir;
use pos::Prefix;
use pos::RelativePath;
use pos::RelativePathCtx;
use pos::TypeName;
use rayon::prelude::*;
use tempfile::TempDir;
use ty::reason::BReason;
use ty::reason::NReason;
use ty::reason::Reason;

#[derive(clap::Parser, Debug)]
struct CliOptions {
    /// The repository root (where .hhconfig is), e.g., ~/www
    root: PathBuf,

    /// Set the number of threads to use for parsing and folding.
    ///
    /// If omitted or set to 0, uses the rayon default (the value of the
    /// `RAYON_NUM_THREADS` environment variable if set, or the number of
    /// logical CPUs otherwise).
    #[clap(long)]
    num_threads: Option<usize>,

    /// Path to a SQLite naming table (allowing parsing to be done lazily instead of up-front).
    #[clap(long)]
    naming_table: Option<PathBuf>,

    /// Allocate decls with positions instead of allocating position-free decls.
    #[clap(long)]
    with_pos: bool,

    /// In addition to parsing shallow decls, compute folded class decls.
    #[clap(long)]
    fold: bool,

    /// Keep all decls in memory rather than serializing and compressing them.
    #[clap(long)]
    no_serialize: bool,

    /// Use the given compression algorithm when serializing decls (if serialization is enabled).
    #[clap(default_value = "none", long)]
    compression: Compression,

    /// Output profiling results for folding (in JSON format) to a separate file.
    #[clap(long)]
    profile_output: Option<PathBuf>,
}

#[derive(serde::Serialize, Debug, Default)]
struct ProfileFoldResult {
    real_time: f64,
    rss: f64,
}

fn main() {
    let opts = <CliOptions as clap::Parser>::parse();

    if let Some(num_threads) = opts.num_threads {
        rayon::ThreadPoolBuilder::new()
            .num_threads(num_threads)
            .build_global()
            .unwrap();
    }

    let hhi_root = TempDir::with_prefix("rupro_decl_repo_hhi.").unwrap();
    hhi::write_hhi_files(hhi_root.path()).unwrap();

    let path_ctx = Arc::new(RelativePathCtx {
        root: opts.root.clone(),
        hhi: hhi_root.path().into(),
        dummy: PathBuf::new(),
        tmp: PathBuf::new(),
    });

    if opts.with_pos {
        decl_repo::<BReason>(&opts, path_ctx, hhi_root);
    } else {
        decl_repo::<NReason>(&opts, path_ctx, hhi_root);
    }
}

fn decl_repo<R: Reason>(opts: &CliOptions, ctx: Arc<RelativePathCtx>, hhi_root: TempDir) {
    use oxidized::parser_options::ParserOptions;
    let names = collect_file_or_class_names(opts, &ctx);

    let file_provider: Arc<dyn file_provider::FileProvider> = Arc::new(
        file_provider::DiskProvider::new(Arc::clone(&ctx), Some(hhi_root)),
    );
    let parser_opts = ParserOptions::default();
    let parser = DeclParser::new(
        file_provider,
        DeclParserOptions::from_parser_options(&parser_opts),
        parser_opts.po_deregister_php_stdlib,
    );
    let shallow_decl_store = make_shallow_decl_store::<R>(if opts.no_serialize {
        StoreOpts::Unserialized
    } else {
        StoreOpts::Serialized(opts.compression)
    });
    let classes = match names {
        Names::Classnames(classes) => classes,
        Names::Filenames(filenames) => {
            let (classes, time_taken) = time(|| {
                populate_shallow_decl_store(&shallow_decl_store, parser.clone(), &filenames)
            });
            println!(
                "Parsed {} classes in repo in {:?}",
                classes.len(),
                time_taken
            );
            print_rss();
            classes
        }
    };

    let folded_decl_provider = make_folded_decl_provider(
        if opts.no_serialize {
            StoreOpts::Unserialized
        } else {
            StoreOpts::Serialized(opts.compression)
        },
        opts.naming_table.as_ref(),
        shallow_decl_store,
        Arc::new(parser_opts),
        parser,
    );
    if opts.fold {
        let len = classes.len();
        let ((), time_taken) = time(|| fold(&folded_decl_provider, classes));
        println!("Folded {} classes in repo in {:?}", len, time_taken);
        let rss = print_rss();
        if let Some(output_path) = &opts.profile_output {
            write_profile_fold_result(
                output_path,
                ProfileFoldResult {
                    real_time: time_taken.as_secs_f64(),
                    rss,
                },
            )
        }
    }

    // Avoid running the decl provider's destructor or destructors for hcons
    // tables, since our stores are huge and full of Arcs which reference one
    // another.
    std::process::exit(0);
}

fn write_profile_fold_result(output_path: &Path, profile: ProfileFoldResult) {
    let mut output_file = std::fs::File::create(output_path).unwrap();
    serde_json::to_writer(&mut output_file, &profile).unwrap();
}
enum Names {
    Filenames(Vec<RelativePath>),
    Classnames(Vec<TypeName>),
}

fn collect_file_or_class_names(opts: &CliOptions, ctx: &RelativePathCtx) -> Names {
    if let Some(naming_table) = &opts.naming_table {
        let (classes, time_taken) = time(|| {
            let conn = rusqlite::Connection::open(naming_table).unwrap();
            let mut stmt = conn
                .prepare("select classes from naming_file_info")
                .unwrap();
            let mut rows = stmt.query(rusqlite::params![]).unwrap();
            let mut classes = vec![];
            while let Some(row) = rows.next().unwrap() {
                let row: Option<String> = row.get(0).unwrap();
                if let Some(row) = row {
                    classes.extend(row.split('|').filter(|s| !s.is_empty()).map(TypeName::from));
                }
            }
            classes
        });
        println!(
            "Queried {} classes from naming table in {:?}",
            classes.len(),
            time_taken
        );

        Names::Classnames(classes)
    } else {
        let mut filenames: Vec<RelativePath> = find_hack_files(&ctx.hhi)
            .map(|path| RelativePath::new(Prefix::Hhi, path.strip_prefix(&ctx.hhi).unwrap()))
            .collect();

        let ((), time_taken) = time(|| {
            filenames.extend(find_hack_files(&opts.root).map(|path| {
                match path.strip_prefix(&ctx.root) {
                    Ok(suffix) => RelativePath::new(Prefix::Root, suffix),
                    Err(..) => RelativePath::new(Prefix::Dummy, &path),
                }
            }))
        });
        println!(
            "Collected {} filenames in {:?}",
            filenames.len(),
            time_taken
        );
        Names::Filenames(filenames)
    }
}

fn fold<R: Reason>(provider: &impl FoldedDeclProvider<R>, classes: Vec<TypeName>) {
    let len = classes.len();
    classes
        .into_par_iter()
        .progress_count(len as u64)
        .for_each(|class| {
            provider
                .get_class(class)
                .unwrap_or_else(|e| panic!("failed to fold class {}: {:?}", class, e))
                .unwrap_or_else(|| panic!("failed to look up class {}", class));
        })
}

fn find_hack_files(path: impl AsRef<Path>) -> impl Iterator<Item = PathBuf> {
    WalkDir::new(path)
        .into_iter()
        .filter_map(|e| e.ok())
        .filter(|e| !e.file_type().is_dir())
        .map(|e| e.path())
        .filter(|path| find_utils::is_hack(path))
}

fn print_rss() -> f64 {
    let me = procfs::process::Process::myself().unwrap();
    let page_size = procfs::page_size();
    let rss = (me.stat().unwrap().rss * page_size) as f64 / 1024.0 / 1024.0 / 1024.0;
    println!("RSS: {:.3}GiB", rss);
    rss
}

fn time<T>(f: impl FnOnce() -> T) -> (T, std::time::Duration) {
    let start = std::time::Instant::now();
    let result = f();
    let time_taken = start.elapsed();
    (result, time_taken)
}
