// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hackrs::{decl_defs::shallow, decl_parser::DeclParser, folded_decl_provider, reason::BReason};
use ocamlrep_ocamlpool::ocaml_ffi_with_arena;
use pos::{Prefix, RelativePath, RelativePathCtx, ToOxidized};
use std::sync::Arc;

ocaml_ffi_with_arena! {
    fn decl_folded_classes_in_file_ffi<'a>(
        arena: &'a bumpalo::Bump,
        filename: &'a oxidized_by_ref::relative_path::RelativePath<'a>,
    ) -> Vec<oxidized_by_ref::decl_defs::DeclClassType<'a>> {
        let path_ctx = Arc::new(RelativePathCtx::default());
        let filename = RelativePath::new(Prefix::Dummy, filename.path());
        let decl_parser = DeclParser::new(path_ctx);
        let folded_decl_provider: Arc<dyn folded_decl_provider::FoldedDeclProvider<BReason>> =
            hackrs_test_utils::decl_provider::make_folded_decl_provider(
                None,
                &decl_parser,
                std::iter::once(filename),
            );

        decl_parser
            .parse::<BReason>(filename)
            .expect("failed to parse")
            .into_iter()
            .filter_map(|decl| match decl {
                shallow::Decl::Class(name, _) => Some(
                    folded_decl_provider
                        .get_class(name)
                        .expect("failed to fold class")
                        .expect("failed to look up class"),
                ),
                _ => None,
            })
            .map(|cls| cls.to_oxidized(arena))
            .collect()
    }
}
