// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::path::PathBuf;
use std::rc::Rc;

use ocamlrep_derive::ToOcamlRep;
use oxidized::global_options::GlobalOptions;

use hackrs::alloc;
use hackrs::ast_provider::AstProvider;
use hackrs::folded_decl_provider::{FoldedDeclLocalCache, FoldedDeclProvider};
use hackrs::pos::{Prefix, RelativePathCtx};
use hackrs::shallow_decl_provider::{ShallowDeclLocalCache, ShallowDeclProvider};
use hackrs::special_names::SpecialNames;
use hackrs::tast;
use hackrs::typing_check_utils::TypingCheckUtils;
use hackrs::typing_ctx::TypingCtx;
use hackrs::typing_decl_provider::{TypingDeclLocalCache, TypingDeclProvider};

use hackrs::reason::Reason;
// use hackrs::reason::BReason;
use hackrs::reason::NReason;

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

#[no_mangle]
pub extern "C" fn stc_main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() != 2 {
        eprintln!("Usage: {} <FILENAME1> <FILENAME2> <...>", args[0]);
        std::process::exit(1);
    }
    let mut filenames = Vec::new();
    for arg in &args[1..args.len()] {
        filenames.push(PathBuf::from(arg));
    }

    let relative_path_ctx = Rc::new(RelativePathCtx {
        root: PathBuf::new(),
        hhi: PathBuf::new(),
        dummy: PathBuf::new(),
        tmp: PathBuf::new(),
    });

    let options = Rc::new(oxidized::global_options::GlobalOptions::default());
    let (global_alloc, alloc, _pos_alloc) = alloc::get_allocators_for_main();
    let special_names = SpecialNames::new(global_alloc);
    let ast_provider = AstProvider::new(
        Rc::clone(&relative_path_ctx),
        special_names,
        Rc::clone(&options),
    );
    let shallow_decl_cache = Rc::new(ShallowDeclLocalCache::new());
    let shallow_decl_provider = Rc::new(ShallowDeclProvider::new(
        shallow_decl_cache,
        alloc,
        relative_path_ctx,
    ));
    let folded_decl_cache = Rc::new(FoldedDeclLocalCache::new());
    let folded_decl_provider = Rc::new(FoldedDeclProvider::new(
        folded_decl_cache,
        Rc::clone(&shallow_decl_provider),
    ));
    let typing_decl_cache = Rc::new(TypingDeclLocalCache::new());
    let typing_decl_provider = Rc::new(TypingDeclProvider::new(
        typing_decl_cache,
        Rc::clone(&folded_decl_provider),
    ));
    let ctx = Rc::new(TypingCtx::new(
        alloc,
        folded_decl_provider,
        typing_decl_provider,
        special_names,
    ));

    let filenames: Vec<_> = filenames
        .into_iter()
        .map(|fln| alloc.relative_path(Prefix::Root, &fln))
        .collect();

    shallow_decl_provider
        .add_from_files(&mut filenames.iter())
        .unwrap();

    // println!("{:#?}", shallow_decl_provider);

    for fln in &filenames {
        let (ast, errs) = ast_provider.get_ast(fln).unwrap();
        let (tast, errs) = TypingCheckUtils::type_file::<NReason>(Rc::clone(&ctx), &ast, errs);
        if !errs.is_empty() {
            unimplemented!()
        }
        print_tast(&options, &tast);
    }
}
