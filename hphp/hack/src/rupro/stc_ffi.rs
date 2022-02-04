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
use hackrs::folded_decl_provider::{FoldedDeclGlobalCache, FoldedDeclProvider};
use hackrs::reason::{NReason, Reason};
use hackrs::shallow_decl_provider::{ShallowDeclGlobalCache, ShallowDeclProvider};
use hackrs::special_names::SpecialNames;
use hackrs::tast;
use hackrs::typing_check_utils::TypingCheckUtils;
use hackrs::typing_ctx::TypingCtx;
use hackrs::typing_decl_provider::{TypingDeclGlobalCache, TypingDeclProvider};
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
    let (global_alloc, alloc, _pos_alloc) = alloc::get_allocators_for_main();
    let special_names = SpecialNames::new(global_alloc);
    let ast_provider = AstProvider::new(
        Arc::clone(&relative_path_ctx),
        special_names,
        Arc::clone(&options),
    );
    let shallow_decl_cache = Arc::new(ShallowDeclGlobalCache::new());
    let shallow_decl_provider = Arc::new(ShallowDeclProvider::new(
        shallow_decl_cache,
        alloc,
        relative_path_ctx,
    ));
    let folded_decl_cache = Arc::new(FoldedDeclGlobalCache::new());
    let folded_decl_provider = Arc::new(FoldedDeclProvider::new(
        folded_decl_cache,
        special_names,
        Arc::clone(&shallow_decl_provider),
    ));
    let typing_decl_cache = Arc::new(TypingDeclGlobalCache::new());
    let typing_decl_provider = Arc::new(TypingDeclProvider::new(
        typing_decl_cache,
        Arc::clone(&folded_decl_provider),
    ));
    let ctx = Arc::new(TypingCtx::new(
        alloc,
        folded_decl_provider,
        typing_decl_provider,
        special_names,
    ));

    let filenames: Vec<RelativePath> = cli_options
        .filenames
        .into_iter()
        .map(|fln| alloc.relative_path(Prefix::Root, &fln))
        .collect();

    shallow_decl_provider
        .add_from_files(filenames.iter().copied())
        .unwrap();

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
