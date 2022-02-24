// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::{
    alloc,
    cache::NonEvictingCache,
    decl_defs::shallow,
    decl_parser::DeclParser,
    folded_decl_provider::{FoldedDeclProvider, LazyFoldedDeclProvider},
    naming_provider::SqliteNamingTable,
    reason::Reason,
    shallow_decl_provider::{
        EagerShallowDeclProvider, LazyShallowDeclProvider, ShallowDeclCache, ShallowDeclProvider,
    },
    special_names::SpecialNames,
    typing_decl_provider::{FoldingTypingDeclProvider, TypingDeclProvider},
};
use pos::{Prefix, RelativePath, RelativePathCtx};
use std::path::PathBuf;
use std::sync::Arc;
use structopt::StructOpt;

#[derive(StructOpt, Debug)]
struct CliOptions {
    /// The Hack source files to declare.
    #[structopt(value_name("FILEPATH"))]
    filenames: Vec<PathBuf>,

    /// The repository root (where .hhconfig is), e.g., ~/www
    #[structopt(long)]
    root: Option<PathBuf>,

    /// Path to a SQLite naming table (allowing declaration of a single file in a large repo).
    #[structopt(long)]
    naming_table: Option<PathBuf>,

    /// Allocate decls with positions instead of allocating position-free decls.
    #[structopt(long)]
    with_pos: bool,

    /// Print the shallow decls in the file.
    #[structopt(long)]
    shallow: bool,

    /// Print the folded decls of the classes in the file.
    #[structopt(long)]
    folded: bool,

    /// Print the typing decls (i.e., folded decls with member types) of the classes in the file.
    #[structopt(long)]
    typing: bool,
}

fn main() {
    let mut opts = CliOptions::from_args();

    let path_ctx = Arc::new(RelativePathCtx {
        root: opts.root.clone().unwrap_or_else(PathBuf::new),
        hhi: PathBuf::new(),
        dummy: PathBuf::new(),
        tmp: PathBuf::new(),
    });

    let (global_alloc, alloc, pos_alloc) = alloc::get_allocators_for_main();

    let filenames: Vec<RelativePath> = opts
        .filenames
        .iter()
        .map(|path| {
            if let Some(root) = opts.root.as_ref() {
                if let Ok(suffix) = path.strip_prefix(root) {
                    return alloc.relative_path(Prefix::Root, suffix);
                }
            }
            alloc.relative_path(Prefix::Dummy, path)
        })
        .collect();

    // If no modes were specified, print the shallow decls at least.
    if !opts.shallow && !opts.folded && !opts.typing {
        opts.shallow = true;
    }

    if opts.with_pos {
        decl_files(&opts, global_alloc, pos_alloc, path_ctx, &filenames);
    } else {
        decl_files(&opts, global_alloc, alloc, path_ctx, &filenames);
    }
}

fn decl_files<R: Reason>(
    opts: &CliOptions,
    global_alloc: &'static alloc::GlobalAllocator,
    alloc: &'static alloc::Allocator<R>,
    ctx: Arc<RelativePathCtx>,
    filenames: &[RelativePath],
) {
    let decl_parser = DeclParser::new(alloc, ctx);
    let shallow_decl_provider =
        make_shallow_decl_provider(opts, global_alloc, &decl_parser, filenames);
    let folded_decl_provider = Arc::new(LazyFoldedDeclProvider::new(
        Arc::new(NonEvictingCache::new()),
        alloc,
        SpecialNames::new(global_alloc),
        shallow_decl_provider,
    ));
    let typing_decl_provider = Arc::new(FoldingTypingDeclProvider::new(
        Arc::new(NonEvictingCache::new()),
        Arc::clone(&folded_decl_provider) as Arc<dyn FoldedDeclProvider<R>>,
    ));

    for &path in filenames {
        for decl in decl_parser.parse(path).unwrap() {
            match decl {
                shallow::Decl::Class(name, decl) => {
                    if opts.shallow {
                        println!("{:#?}", decl);
                    }
                    if opts.folded {
                        println!("{:#?}", folded_decl_provider.get_class(name).unwrap())
                    }
                    if opts.typing {
                        println!("{:#?}", typing_decl_provider.get_class(name).unwrap())
                    }
                }
                shallow::Decl::Fun(_, decl) => println!("{:#?}", decl),
                shallow::Decl::Typedef(_, decl) => println!("{:#?}", decl),
                shallow::Decl::Const(_, decl) => println!("{:#?}", decl),
            }
        }
    }
}

fn make_shallow_decl_provider<R: Reason>(
    opts: &CliOptions,
    global_alloc: &'static alloc::GlobalAllocator,
    decl_parser: &DeclParser<R>,
    filenames: &[RelativePath],
) -> Arc<dyn ShallowDeclProvider<R>> {
    let cache = Arc::new(ShallowDeclCache::with_no_eviction());
    for &path in filenames {
        let decls = decl_parser.parse(path).unwrap();
        cache.add_decls(decls);
    }
    if let Some(naming_table_path) = &opts.naming_table {
        Arc::new(LazyShallowDeclProvider::new(
            cache,
            Arc::new(SqliteNamingTable::new(global_alloc, naming_table_path)),
            decl_parser.clone(),
        ))
    } else {
        Arc::new(EagerShallowDeclProvider::new(cache))
    }
}
