// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hackrs::decl_parser::DeclParser;
use hackrs::folded_decl_provider::FoldedDeclProvider;
use hackrs_test_utils::serde_store::{Compression, StoreOpts};
use hackrs_test_utils::store::{make_shallow_decl_store, populate_shallow_decl_store};
use indicatif::ParallelProgressIterator;
use jwalk::WalkDir;
use ocamlrep_ocamlpool::{ocaml_ffi_with_arena, Bump};
use oxidized_by_ref::{decl_defs::DeclClassType, parser_options::ParserOptions};
use pos::{Prefix, RelativePath, RelativePathCtx, ToOxidized, TypeName};
use rayon::iter::{IntoParallelIterator, ParallelIterator};
use std::cmp;
use std::collections::BTreeMap;
use std::path::{Path, PathBuf};
use std::sync::Arc;
use ty::decl::{folded::FoldedClass, shallow};
use ty::reason::BReason;

fn find_hack_files(path: impl AsRef<Path>) -> impl Iterator<Item = PathBuf> {
    WalkDir::new(path)
        .into_iter()
        .filter_map(|e| e.ok())
        .filter(|e| !e.file_type().is_dir())
        .map(|e| e.path())
        .filter(|path| find_utils::is_hack(path))
}

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
        let file_provider: Arc<dyn file_provider::FileProvider> =
            Arc::new(file_provider::PlainFileProvider::with_no_cache(path_ctx));
        let decl_parser = DeclParser::with_options(file_provider, opts);
        let shallow_decl_store = make_shallow_decl_store(StoreOpts::Unserialized);

        let reverse_files = files.iter().copied().rev().collect::<Vec<_>>();
        for path in &reverse_files {
            let mut decls = decl_parser.parse(*path).unwrap();
            decls.reverse(); // To match OCaml behavior for name collisions
            shallow_decl_store.add_decls(decls).unwrap();
        };

        let folded_decl_provider =
            hackrs_test_utils::decl_provider::make_folded_decl_provider(
                StoreOpts::Unserialized,
                None,
                shallow_decl_store,
                decl_parser.clone(),
            );

        files.into_iter().map(|filename| {
            let classes: Vec<TypeName> = decl_parser
                .parse(filename)
                .map_err(|e| format!("Failed to parse {:?}: {:?}", filename, e))?
                .into_iter()
                .filter_map(|decl: ty::decl::shallow::Decl<BReason>| match decl {
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

    // Due to memory constraints when folding over a large directory, it may be necessary to
    // fold over www in parts. This is achieved by finding the shallow decls of all of the files
    // in the directory but then only folding over a portion of the classes in those files
    // num_partitions controls the numbers of partitions (>= 1) the classes are divided into
    // and partition_index controls which partition is being folded
    // Returns a SMap from class_name to folded_decl
    fn partition_and_fold_dir_ffi<'a>(
        arena: &'a Bump,
        www_root: &'a Path,
        opts: &'a ParserOptions<'a>,
        num_partitions: &'a usize,
        partition_index: &'a usize,
    ) -> BTreeMap<String, DeclClassType<'a>> {
        // Collect hhi files
        let hhi_root = tempdir::TempDir::new("rupro_decl_repo_hhi").unwrap();
        hhi::write_hhi_files(hhi_root.path()).unwrap();
        let hhi_root_path: PathBuf = hhi_root.path().into();
        let mut filenames: Vec<RelativePath> = find_hack_files(&hhi_root_path)
            .map(|path| RelativePath::new(Prefix::Hhi, path.strip_prefix(&hhi_root_path).unwrap()))
            .collect();
        // Collect www files
        filenames.extend(
            find_hack_files(&www_root).map(|path| match path.strip_prefix(&www_root) {
                Ok(suffix) => RelativePath::new(Prefix::Root, suffix),
                Err(..) => RelativePath::new(Prefix::Dummy, &path),
            }),
        );

        let path_ctx = Arc::new(RelativePathCtx {
            root: www_root.into(),
            hhi: hhi_root_path,
            ..Default::default()
        });

        // Parse and gather shallow decls
        let file_provider: Arc<dyn file_provider::FileProvider> =
            Arc::new(file_provider::PlainFileProvider::with_no_cache(path_ctx));
        let decl_parser: DeclParser<BReason> = DeclParser::with_options(file_provider, opts);
        let shallow_decl_store = make_shallow_decl_store(StoreOpts::Serialized(Compression::default()));
        let mut classes =
            populate_shallow_decl_store(&shallow_decl_store, decl_parser.clone(), &filenames);
        classes.sort();
        let folded_decl_provider = hackrs_test_utils::decl_provider::make_folded_decl_provider(
            StoreOpts::Serialized(Compression::default()),
            None,
            shallow_decl_store,
            decl_parser,
        );

        let len = classes.len();
        // Add 1 to size to ensure that we only have num_partitions
        let size_of_slices = (len / num_partitions) + 1;
        // Account for edge case where partition_index * size goes out of bounds of the array
        let start_index = cmp::min(len, partition_index * size_of_slices);
        // end_index is exclusive, so no need to say len - 1
        let end_index = cmp::min((partition_index + 1) * size_of_slices, len);
        // If start_index = end_index, this will be empty vec
        let s: Vec<(String, Arc<FoldedClass<BReason>>)> = (&classes[start_index..end_index])
            .into_par_iter()
            .progress_count(len.try_into().unwrap())
            .map(|class| {
                let folded_decl = folded_decl_provider
                    .get_class((*class).into(), *class)
                    .expect("failed to fold class")
                    .expect("failed to look up class");
                (class.as_str().into(), folded_decl)
            })
            .collect();
        s.into_iter()
            .map(|(name, fc)| (name, fc.to_oxidized(arena)))
            .collect()
    }

    fn decls_equal_ffi<'a>(arena: &'a Bump, ocaml_decl: &'a DeclClassType<'a>, rust_decl: &'a DeclClassType<'a>) -> bool {
        let ocaml_decl = <FoldedClass<BReason>>::from(ocaml_decl);
        let rust_decl = <FoldedClass<BReason>>::from(rust_decl);
        ocaml_decl == rust_decl
    }
}
