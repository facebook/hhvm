// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::{decl_parser::DeclParser, folded_decl_provider};
use ocamlrep_ocamlpool::{ocaml_ffi_with_arena, Bump};
use oxidized_by_ref::{decl_defs::DeclClassType, parser_options::ParserOptions};
use pos::{RelativePath, RelativePathCtx, ToOxidized, TypeName};
use std::collections::BTreeMap;
use std::path::Path;
use std::sync::Arc;
use ty::decl::{folded::FoldedClass, shallow};
use ty::reason::BReason;

ocaml_ffi_with_arena! {
    fn fold_classes_in_files_ffi<'a>(
        arena: &'a Bump,
        root: &'a Path,
        opts: &'a ParserOptions<'a>,
        files: &'a [oxidized_by_ref::relative_path::RelativePath<'a>],
    ) -> Result<BTreeMap<RelativePath, Vec<DeclClassType<'a>>>, String> {
        let files: Vec<RelativePath> = files.iter().map(Into::into).collect();
        let path_ctx = Arc::new(RelativePathCtx {
            root: root.into(),
            ..Default::default()
        });
        let decl_parser = DeclParser::with_options(path_ctx, opts);
        let folded_decl_provider: Arc<dyn folded_decl_provider::FoldedDeclProvider<BReason>> =
            hackrs_test_utils::decl_provider::make_folded_decl_provider(
                None,
                &decl_parser,
                // Reverse to match the OCaml behavior
                files.iter().copied().rev(),
            );
        files.into_iter().map(|filename| {
            let classes: Vec<TypeName> = decl_parser
                .parse(filename)
                .map_err(|e| format!("Failed to parse {:?}: {:?}", filename, e))?
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
                folded_decl_provider.get_class(name.into(), name)
                    .map_err(|e| format!("Failed to fold class {}: {:?}", name, e))?;
            }
            Ok((filename, classes.into_iter().map(|name| {
                Ok(folded_decl_provider
                    .get_class(name.into(), name)
                    .map_err(|e| format!("Failed to fold class {}: {:?}", name, e))?
                    .ok_or_else(|| format!("Decl not found: class {}", name))?
                    .to_oxidized(arena))
            }).collect::<Result<_, String>>()?))
        })
        .collect()
    }

    fn show_decl_class_type_ffi<'a>(arena: &'a Bump, decl: &'a DeclClassType<'a>) -> String {
        let decl = <FoldedClass<BReason>>::from(decl);
        format!("{:#?}", decl)
    }
}
