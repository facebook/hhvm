// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::{Path, PathBuf};
use std::sync::Arc;

use indicatif::ParallelProgressIterator;
use jwalk::WalkDir;
use rayon::prelude::*;
use structopt::StructOpt;
use tempdir::TempDir;

use ty::reason::{BReason, NReason, Reason};

use hackrs::{
    decl_parser::DeclParser, folded_decl_provider::FoldedDeclProvider,
    shallow_decl_provider::ShallowDeclCache,
};
use hackrs_test_utils::cache::NonEvictingCache;
use hackrs_test_utils::serde_cache::{Compression, SerializingCache};
use pos::{Prefix, RelativePath, RelativePathCtx, TypeName};

#[derive(StructOpt, Debug)]
struct CliOptions {
    /// The repository root (where .hhconfig is), e.g., ~/www
    root: PathBuf,

    /// Set the number of threads to use for parsing and folding.
    ///
    /// If omitted or set to 0, uses the rayon default (the value of the
    /// `RAYON_NUM_THREADS` environment variable if set, or the number of
    /// logical CPUs otherwise).
    #[structopt(long)]
    num_threads: Option<usize>,

    /// Path to a SQLite naming table (allowing parsing to be done lazily instead of up-front).
    #[structopt(long)]
    naming_table: Option<PathBuf>,

    /// Allocate decls with positions instead of allocating position-free decls.
    #[structopt(long)]
    with_pos: bool,

    /// In addition to parsing shallow decls, compute folded class decls.
    #[structopt(long)]
    fold: bool,

    /// Store decls in a data store which serializes and compresses them.
    #[structopt(long)]
    serialize: bool,

    /// If `--serialize` was given, use the given compression algorithm.
    #[structopt(default_value, long)]
    compression: Compression,
}

fn main() {
    let opts = CliOptions::from_args();

    if let Some(num_threads) = opts.num_threads {
        rayon::ThreadPoolBuilder::new()
            .num_threads(num_threads)
            .build_global()
            .unwrap();
    }

    let hhi_root = TempDir::new("rupro_decl_repo_hhi").unwrap();
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
    let names = collect_file_or_class_names(opts, &ctx);

    let parser = DeclParser::new(ctx);
    let shallow_decl_cache = make_shallow_cache::<R>(opts);
    let classes = match names {
        Names::Classnames(classes) => classes,
        Names::Filenames(filenames) => {
            let (classes, time_taken) =
                time(|| parse(parser.clone(), &shallow_decl_cache, filenames));
            println!("Parsed repo in {:?}", time_taken);
            print_rss();
            classes
        }
    };

    let folded_decl_provider = make_folded_provider(opts, shallow_decl_cache, parser);
    if opts.fold {
        let ((), time_taken) = time(|| fold(&folded_decl_provider, classes));
        println!("Folded repo in {:?}", time_taken);
        print_rss();
    }

    // Avoid running the decl provider's destructor or destructors for hcons
    // tables, since our caches are huge and full of Arcs which reference one
    // another. Do destroy the hhi_root, so the tempdir gets cleaned up.
    drop(hhi_root);
    std::process::exit(0);
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
        println!("Queried all classes from naming table in {:?}", time_taken);
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

fn parse<R: Reason>(
    decl_parser: DeclParser<R>,
    shallow_decl_cache: &ShallowDeclCache<R>,
    filenames: Vec<RelativePath>,
) -> Vec<TypeName> {
    let len = filenames.len();
    filenames
        .into_par_iter()
        .progress_count(len as u64)
        .flat_map_iter(|path| {
            let (decls, summary) = decl_parser.parse_and_summarize(path).unwrap();
            shallow_decl_cache.add_decls(decls);
            summary
                .classes()
                .map(|(class, _hash)| TypeName::new(class))
                .collect::<Vec<_>>()
                .into_iter()
        })
        .collect()
}

fn fold<R: Reason>(provider: &impl FoldedDeclProvider<R>, classes: Vec<TypeName>) {
    let len = classes.len();
    classes
        .into_par_iter()
        .progress_count(len as u64)
        .for_each(|class| {
            provider
                .get_class(class.into(), class)
                .unwrap_or_else(|e| panic!("failed to fold class {}: {:?}", class, e))
                .unwrap_or_else(|| panic!("failed to look up class {}", class));
        })
}

fn make_shallow_cache<R: Reason>(opts: &CliOptions) -> ShallowDeclCache<R> {
    if opts.serialize {
        ShallowDeclCache::new(
            Arc::new(SerializingCache::with_compression(opts.compression)), // types
            Box::new(SerializingCache::with_compression(opts.compression)), // funs
            Box::new(SerializingCache::with_compression(opts.compression)), // consts
            Box::new(SerializingCache::with_compression(opts.compression)), // modules
            Box::new(SerializingCache::with_compression(opts.compression)), // properties
            Box::new(SerializingCache::with_compression(opts.compression)), // static_properties
            Box::new(SerializingCache::with_compression(opts.compression)), // methods
            Box::new(SerializingCache::with_compression(opts.compression)), // static_methods
            Box::new(SerializingCache::with_compression(opts.compression)), // constructors
        )
    } else {
        ShallowDeclCache::with_no_member_caches(
            Arc::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
        )
    }
}

fn make_folded_provider<R: Reason>(
    opts: &CliOptions,
    shallow_decl_cache: ShallowDeclCache<R>,
    decl_parser: DeclParser<R>,
) -> impl FoldedDeclProvider<R> {
    hackrs::folded_decl_provider::LazyFoldedDeclProvider::new(
        Default::default(),
        if opts.serialize {
            Arc::new(SerializingCache::with_compression(opts.compression))
        } else {
            Arc::new(NonEvictingCache::default())
        },
        if let Some(naming_table) = &opts.naming_table {
            let naming_table =
                hackrs::naming_provider::SqliteNamingTable::new(naming_table).unwrap();
            Arc::new(hackrs::shallow_decl_provider::LazyShallowDeclProvider::new(
                Arc::new(shallow_decl_cache),
                Arc::new(naming_table),
                decl_parser,
            ))
        } else {
            Arc::new(
                hackrs::shallow_decl_provider::EagerShallowDeclProvider::new(Arc::new(
                    shallow_decl_cache,
                )),
            )
        },
        Arc::new(hackrs_test_utils::registrar::DependencyGraph::new()),
    )
}

fn find_hack_files(path: impl AsRef<Path>) -> impl Iterator<Item = PathBuf> {
    WalkDir::new(path)
        .into_iter()
        .filter_map(|e| e.ok())
        .filter(|e| !e.file_type().is_dir())
        .map(|e| e.path())
        .filter(|path| find_utils::is_hack(path))
}

fn print_rss() {
    let me = procfs::process::Process::myself().unwrap();
    let page_size = procfs::page_size().unwrap();
    println!(
        "RSS: {:.3}GiB",
        (me.stat.rss * page_size) as f64 / 1024.0 / 1024.0 / 1024.0
    );
}

fn time<T>(f: impl FnOnce() -> T) -> (T, std::time::Duration) {
    let start = std::time::Instant::now();
    let result = f();
    let time_taken = start.elapsed();
    (result, time_taken)
}
