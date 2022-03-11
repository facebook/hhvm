// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::{
    decl_defs::{folded::FoldedClass, shallow},
    decl_parser::DeclParser,
    folded_decl_provider,
    reason::BReason,
};
use ocamlrep_ocamlpool::{ocaml_ffi_with_arena, Bump};
use oxidized_by_ref::decl_defs::DeclClassType;
use pos::{RelativePath, RelativePathCtx, ToOxidized};
use std::collections::BTreeMap;
use std::path::Path;
use std::sync::Arc;

ocaml_ffi_with_arena! {
    fn fold_classes_in_files_ffi<'a>(
        arena: &'a Bump,
        root: &'a Path,
        files: &'a [oxidized_by_ref::relative_path::RelativePath<'a>],
    ) -> BTreeMap<RelativePath, Vec<DeclClassType<'a>>> {
        let files: Vec<RelativePath> = files.iter().map(Into::into).collect();
        let path_ctx = Arc::new(RelativePathCtx {
            root: root.into(),
            ..Default::default()
        });
        let decl_parser = DeclParser::new(path_ctx);
        let folded_decl_provider: Arc<dyn folded_decl_provider::FoldedDeclProvider<BReason>> =
            hackrs_test_utils::decl_provider::make_folded_decl_provider(
                None,
                &decl_parser,
                files.iter().copied(),
            );
        files
            .into_iter()
            .map(|filename| {
                (
                    filename,
                    decl_parser
                        .parse(filename)
                        .expect("failed to parse")
                        .into_iter()
                        .filter_map(|decl| match decl {
                            shallow::Decl::Class(name, _) => Some(
                                folded_decl_provider
                                    .get_class(name.into(), name)
                                    .expect("failed to fold class")
                                    .expect("failed to look up class"),
                            ),
                            _ => None,
                        })
                        .map(|cls| cls.to_oxidized(arena))
                        .collect(),
                )
            })
            .collect()
    }

    fn show_decl_class_type_ffi<'a>(arena: &'a Bump, decl: &'a DeclClassType<'a>) -> String {
        let decl = <FoldedClass<BReason>>::from(decl);
        format!("{:#?}", decl)
    }
}
