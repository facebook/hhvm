// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::{decl_parser::DeclParser, folded_decl_provider::FoldedDeclProvider};
use ocamlrep_custom::Custom;
use ocamlrep_ocamlpool::{ocaml_ffi_with_arena, Bump};
use oxidized_by_ref::parser_options::ParserOptions;
use pos::RelativePathCtx;
use std::path::Path;
use std::sync::Arc;
use ty::reason::BReason;

struct ProviderBackend {
    #[allow(dead_code)]
    path_ctx: Arc<RelativePathCtx>,
    #[allow(dead_code)]
    folded_decl_provider: Arc<dyn FoldedDeclProvider<BReason>>,
}

impl ocamlrep_custom::CamlSerialize for ProviderBackend {
    ocamlrep_custom::caml_serialize_default_impls!();
}

ocaml_ffi_with_arena! {
    fn hh_rust_provider_backend_make<'a>(
        arena: &'a Bump,
        root: &'a Path,
        hhi_root: &'a Path,
        tmp: &'a Path,
        opts: &'a ParserOptions<'a>,
    ) -> Custom<ProviderBackend> {
        let path_ctx = Arc::new(RelativePathCtx {
            root: root.into(),
            hhi: hhi_root.into(),
            tmp: tmp.into(),
            ..Default::default()
        });
        let folded_decl_provider: Arc<dyn FoldedDeclProvider<BReason>> =
            hackrs_test_utils::decl_provider::make_folded_decl_provider(
                None,
                &DeclParser::with_options(Arc::clone(&path_ctx), opts),
                std::iter::empty(),
            );
        Custom::from(ProviderBackend {
            path_ctx,
            folded_decl_provider,
        })
    }
}
