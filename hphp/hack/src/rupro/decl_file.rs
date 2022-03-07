// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::{
    decl_defs::shallow,
    decl_parser::DeclParser,
    folded_decl_provider::FoldedDeclProvider,
    reason::{BReason, NReason, Reason},
    typing_decl_provider::{FoldingTypingDeclProvider, TypingDeclProvider},
};
use jwalk::WalkDir;
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

    let hhi_root = tempdir::TempDir::new("rupro_decl_file_hhi").unwrap();
    hhi::write_hhi_files(hhi_root.path()).unwrap();

    let path_ctx = Arc::new(RelativePathCtx {
        root: opts.root.clone().unwrap_or_else(PathBuf::new),
        hhi: hhi_root.path().into(),
        dummy: PathBuf::new(),
        tmp: PathBuf::new(),
    });

    let filenames: Vec<RelativePath> = opts
        .filenames
        .iter()
        .map(|path| {
            if let Some(root) = opts.root.as_ref() {
                if let Ok(suffix) = path.strip_prefix(root) {
                    return RelativePath::new(Prefix::Root, suffix);
                }
            }
            RelativePath::new(Prefix::Dummy, path)
        })
        .collect();

    // If no modes were specified, print the shallow decls at least.
    if !opts.shallow && !opts.folded && !opts.typing {
        opts.shallow = true;
    }

    if opts.with_pos {
        decl_files::<BReason>(&opts, path_ctx, &filenames);
    } else {
        decl_files::<NReason>(&opts, path_ctx, &filenames);
    }
}

fn decl_files<R: Reason>(opts: &CliOptions, ctx: Arc<RelativePathCtx>, filenames: &[RelativePath]) {
    let hhi_filenames = WalkDir::new(&ctx.hhi)
        .into_iter()
        .filter_map(|e| e.ok())
        .filter(|e| !e.file_type().is_dir())
        .map(|e| RelativePath::new(Prefix::Hhi, e.path().strip_prefix(&ctx.hhi).unwrap()))
        .collect::<Vec<_>>();
    let decl_parser = DeclParser::new(ctx);
    let folded_decl_provider = hackrs_test_utils::decl_provider::make_folded_decl_provider(
        opts.naming_table.as_ref(),
        &decl_parser,
        hhi_filenames.into_iter().chain(filenames.iter().copied()),
    );
    let typing_decl_provider = Arc::new(FoldingTypingDeclProvider::new(
        Box::new(hackrs_test_utils::cache::NonEvictingLocalCache::new()),
        Arc::clone(&folded_decl_provider) as Arc<dyn FoldedDeclProvider<R>>,
    ));

    let mut saw_err = false;

    for &path in filenames {
        for decl in decl_parser.parse(path).unwrap() {
            match decl {
                shallow::Decl::Class(name, decl) => {
                    if opts.shallow {
                        println!("{:#?}", decl);
                    }
                    if opts.folded {
                        match folded_decl_provider.get_class(name) {
                            Ok(decl) => println!("{:#?}", decl),
                            Err(e) => {
                                saw_err = true;
                                eprintln!("Error: {}", e);
                            }
                        }
                    }
                    if opts.typing {
                        match typing_decl_provider.get_class(name) {
                            Ok(decl) => println!("{:#?}", decl),
                            Err(e) => {
                                saw_err = true;
                                eprintln!("Error: {}", e);
                            }
                        }
                    }
                }
                shallow::Decl::Fun(_, decl) => println!("{:#?}", decl),
                shallow::Decl::Typedef(_, decl) => println!("{:#?}", decl),
                shallow::Decl::Const(_, decl) => println!("{:#?}", decl),
            }
        }
    }

    if saw_err {
        std::process::exit(1);
    }
}
