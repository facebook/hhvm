// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::alloc;
use hackrs::ast_provider::AstProvider;
use hackrs::cache::NonEvictingCache;
use hackrs::decl_parser::DeclParser;
use hackrs::folded_decl_provider::LazyFoldedDeclProvider;
use hackrs::reason::{NReason, Reason};
use hackrs::shallow_decl_provider::{EagerShallowDeclProvider, ShallowDeclCache};
use hackrs::special_names::SpecialNames;
use hackrs::tast;
use hackrs::typing_check_utils::TypingCheckUtils;
use hackrs::typing_ctx::TypingCtx;
use hackrs::typing_decl_provider::FoldingTypingDeclProvider;
use ocamlrep_derive::ToOcamlRep;
use oxidized::global_options::GlobalOptions;
use pos::{Prefix, RelativePath, RelativePathCtx};
use std::path::PathBuf;
use std::sync::Arc;
use structopt::StructOpt;

// fn create_nast(path: PathBuf) -> oxidized::aast::Program<(), ()> {}

fn print_tast<R: Reason>(opts: &GlobalOptions, tast: &tast::Program<R>) {
    #[derive(ToOcamlRep)]
    struct StcFfiPrintTastArgs<'a, R: Reason> {
        opts: &'a GlobalOptions,
        tast: &'a tast::Program<R>,
    }

    let stc_ffi_print_tast = unsafe { ocaml_runtime::named_value("stc_ffi_print_tast").unwrap() };

    let args = StcFfiPrintTastArgs { opts, tast };

    unsafe {
        ocaml_runtime::callback_exn(stc_ffi_print_tast, ocamlrep_ocamlpool::to_ocaml(&args))
            .unwrap();
    }
}

#[derive(StructOpt, Debug)]
struct CliOptions {
    /// Hack source files
    #[structopt(value_name("FILEPATH"))]
    filenames: Vec<PathBuf>,
}

#[no_mangle]
pub extern "C" fn stc_main() {
    let cli_options = CliOptions::from_args();

    let relative_path_ctx = Arc::new(RelativePathCtx {
        root: PathBuf::new(),
        hhi: PathBuf::new(),
        dummy: PathBuf::new(),
        tmp: PathBuf::new(),
    });

    let options = Arc::new(oxidized::global_options::GlobalOptions::default());
    let (alloc, _pos_alloc) = alloc::get_allocators_for_main();
    let special_names = SpecialNames::new();
    let ast_provider = AstProvider::new(
        Arc::clone(&relative_path_ctx),
        special_names,
        Arc::clone(&options),
    );
    let decl_parser = DeclParser::new(alloc, relative_path_ctx);
    let shallow_decl_cache = Arc::new(ShallowDeclCache::with_no_eviction());
    let shallow_decl_provider = Arc::new(EagerShallowDeclProvider::new(Arc::clone(
        &shallow_decl_cache,
    )));
    let folded_decl_cache = Arc::new(NonEvictingCache::new());
    let folded_decl_provider = Arc::new(LazyFoldedDeclProvider::new(
        folded_decl_cache,
        alloc,
        special_names,
        shallow_decl_provider,
    ));
    let typing_decl_cache = Arc::new(NonEvictingCache::new());
    let typing_decl_provider = Arc::new(FoldingTypingDeclProvider::new(
        typing_decl_cache,
        alloc,
        folded_decl_provider,
    ));
    let ctx = Arc::new(TypingCtx::new(alloc, typing_decl_provider, special_names));

    let filenames: Vec<RelativePath> = cli_options
        .filenames
        .into_iter()
        .map(|fln| RelativePath::new(Prefix::Root, &fln))
        .collect();

    for &filename in &filenames {
        let decls = decl_parser.parse(filename).unwrap();
        shallow_decl_cache.add_decls(decls);
    }

    // println!("{:#?}", shallow_decl_provider);

    for fln in filenames {
        let (ast, errs) = ast_provider.get_ast(fln).unwrap();
        let (tast, errs) = TypingCheckUtils::type_file::<NReason>(Arc::clone(&ctx), &ast, errs);
        if !errs.is_empty() {
            unimplemented!()
        }
        print_tast(&options, &tast);
    }
}
