// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::{decl_parser::DeclParser, folded_decl_provider};
use ocamlrep_ocamlpool::{ocaml_ffi_with_arena, Bump};
use oxidized_by_ref::decl_defs::DeclClassType;
use pos::{RelativePath, RelativePathCtx, ToOxidized, TypeName};
use std::collections::BTreeMap;
use std::path::Path;
use std::sync::Arc;
use ty::decl_defs::{folded::FoldedClass, shallow};
use ty::reason::BReason;

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
                let classes: Vec<TypeName> = decl_parser
                    .parse(filename)
                    .expect("failed to parse")
                    .into_iter()
                    .filter_map(|decl| match decl {
                        shallow::Decl::Class(name, _) => Some(name),
                        _ => None,
                    })
                    .collect();
                // Declare the classes in the reverse of their order in the file, to
                // match the OCaml behavior. This should only matter when emitting
                // errors for cyclic definitions.
                for &name in classes.iter().rev() {
                    folded_decl_provider
                        .get_class(name.into(), name)
                        .expect("failed to fold class");
                }
                (
                    filename,
                    classes
                        .into_iter()
                        .map(|name| {
                            folded_decl_provider
                                .get_class(name.into(), name)
                                .expect("failed to fold class")
                                .expect("failed to look up class")
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
