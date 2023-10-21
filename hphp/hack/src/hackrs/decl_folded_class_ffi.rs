// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp;
use std::collections::BTreeMap;
use std::path::Path;
use std::path::PathBuf;
use std::sync::Arc;

use bumpalo::Bump;
use decl_parser::DeclParser;
use decl_parser::DeclParserOptions;
use folded_decl_provider::FoldedDeclProvider;
use hackrs_test_utils::serde_store::Compression;
use hackrs_test_utils::serde_store::StoreOpts;
use hackrs_test_utils::store::make_shallow_decl_store;
use hackrs_test_utils::store::populate_shallow_decl_store;
use indicatif::ParallelProgressIterator;
use jwalk::WalkDir;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use ocamlrep_ocamlpool::ocaml_ffi;
use ocamlrep_ocamlpool::ocaml_ffi_with_arena;
use oxidized::parser_options::ParserOptions;
use oxidized_by_ref::decl_defs::DeclClassType;
use pos::Prefix;
use pos::RelativePath;
use pos::RelativePathCtx;
use pos::ToOxidized;
use pos::TypeName;
use rayon::iter::IntoParallelIterator;
use rayon::iter::ParallelIterator;
use ty::decl::folded::FoldedClass;
use ty::decl::shallow;
use ty::reason::BReason;

fn find_hack_files(path: impl AsRef<Path>) -> impl Iterator<Item = PathBuf> {
    WalkDir::new(path)
        .into_iter()
        .filter_map(|e| e.ok())
        .filter(|e| !e.file_type().is_dir())
        .map(|e| e.path())
        .filter(|path| find_utils::is_hack(path))
}

/// Panic if the (possibly-handwritten) impl of ToOcamlRep doesn't match the
/// result of invoking to_oxidized followed by to_ocamlrep (since oxidized types
/// have a generated ToOcamlRep impl with stronger correctness guarantees).
fn verify_to_ocamlrep<'a, T>(bump: &'a Bump, value: &'a T)
where
    T: ToOcamlRep + FromOcamlRep,
    T: ToOxidized<'a> + From<<T as ToOxidized<'a>>::Output>,
    T: std::fmt::Debug + PartialEq,
    <T as ToOxidized<'a>>::Output: std::fmt::Debug + PartialEq + FromOcamlRepIn<'a>,
{
    let alloc = &ocamlrep::Arena::new();
    let oxidized_val = value.to_oxidized(bump);
    let ocaml_val = unsafe { ocamlrep::Value::from_bits(value.to_ocamlrep(alloc).to_bits()) };
    let ocamlrep_round_trip_val =
        <T as ToOxidized<'_>>::Output::from_ocamlrep_in(ocaml_val, bump).unwrap();
    let type_name = std::any::type_name::<T>();
    assert_eq!(
        ocamlrep_round_trip_val, oxidized_val,
        "{}::to_ocamlrep does not match {}::to_oxidized",
        type_name, type_name
    );
    let from_ocaml_val = T::from_ocamlrep(ocaml_val).unwrap();
    assert_eq!(
        from_ocaml_val,
        T::from(ocamlrep_round_trip_val),
        "{}::from_ocamlrep does not match From<oxidized_by_ref> value",
        type_name
    );
}

ocaml_ffi! {
    fn fold_classes_in_files_ffi(
        root: PathBuf,
        opts: ParserOptions,
        files: Vec<relative_path::RelativePath>,
    ) -> Result<BTreeMap<RelativePath, Vec<Arc<FoldedClass<BReason>>>>, String> {
        let files: Vec<RelativePath> = files.iter().map(Into::into).collect();
        let path_ctx = Arc::new(RelativePathCtx {
            root,
            ..Default::default()
        });
        let file_provider: Arc<dyn file_provider::FileProvider> =
            Arc::new(file_provider::DiskProvider::new(path_ctx, None));
        let decl_parser = DeclParser::new(file_provider,
                                          DeclParserOptions::from_parser_options(&opts),
                                          opts.po_deregister_php_stdlib);
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
                Arc::new(opts),
                decl_parser.clone(),
            );

        files.into_iter().map(|filename| {
            let decls = decl_parser
                .parse(filename)
                .map_err(|e| format!("Failed to parse {:?}: {:?}", filename, e))?;
            for decl in decls.iter() {
                match decl {
                    shallow::NamedDecl::Class(_, decl)   => verify_to_ocamlrep(&Bump::new(), decl),
                    shallow::NamedDecl::Fun(_, decl)     => verify_to_ocamlrep(&Bump::new(), decl),
                    shallow::NamedDecl::Typedef(_, decl) => verify_to_ocamlrep(&Bump::new(), decl),
                    shallow::NamedDecl::Const(_, decl)   => verify_to_ocamlrep(&Bump::new(), decl),
                    shallow::NamedDecl::Module(_, decl)  => verify_to_ocamlrep(&Bump::new(), decl),
                }
            }
            let classes: Vec<TypeName> = decls
                .into_iter()
                .filter_map(|decl: ty::decl::shallow::NamedDecl<BReason>| match decl {
                    shallow::NamedDecl::Class(name, _) => Some(name),
                    _ => None,
                })
                .collect();
            // Declare the classes in the reverse of their order in the file, to
            // match the OCaml behavior. This should only matter when emitting
            // errors for cyclic definitions.
            for &name in classes.iter().rev() {
                folded_decl_provider.get_class(name)
                    .map_err(|e| format!("Failed to fold class {}: {:?}", name, e))?;
            }
            Ok((filename, classes.into_iter().map(|name| {
                let folded_class = folded_decl_provider
                    .get_class(name)
                    .map_err(|e| format!("Failed to fold class {}: {:?}", name, e))?
                    .ok_or_else(|| format!("Decl not found: class {}", name))?;
                verify_to_ocamlrep(&Bump::new(), &*folded_class);
                Ok(folded_class)
            }).collect::<Result<_, String>>()?))
        })
        .collect()
    }
}

ocaml_ffi_with_arena! {
    fn show_decl_class_type_ffi<'a>(arena: &'a Bump, decl: &'a DeclClassType<'a>) -> String {
        let decl = <FoldedClass<BReason>>::from(decl);
        format!("{:#?}", decl)
    }

    fn decls_equal_ffi<'a>(
        arena: &'a Bump,
        ocaml_decl: &'a DeclClassType<'a>,
        rust_decl: &'a DeclClassType<'a>
    ) -> bool {
        let ocaml_decl = <FoldedClass<BReason>>::from(ocaml_decl);
        let rust_decl = <FoldedClass<BReason>>::from(rust_decl);
        ocaml_decl == rust_decl
    }
}

ocaml_ffi! {
    // Due to memory constraints when folding over a large directory, it may be necessary to
    // fold over www in parts. This is achieved by finding the shallow decls of all of the files
    // in the directory but then only folding over a portion of the classes in those files
    // num_partitions controls the numbers of partitions (>= 1) the classes are divided into
    // and partition_index controls which partition is being folded
    // Returns a SMap from class_name to folded_decl
    fn partition_and_fold_dir_ffi(
        www_root: PathBuf,
        opts: ParserOptions,
        num_partitions: usize,
        partition_index: usize,
    ) -> BTreeMap<String, Arc<FoldedClass<BReason>>> {
        // Collect hhi files
        let hhi_root = tempfile::TempDir::with_prefix("rupro_decl_repo_hhi.").unwrap();
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
            root: www_root,
            hhi: hhi_root_path,
            ..Default::default()
        });

        // Parse and gather shallow decls
        let file_provider: Arc<dyn file_provider::FileProvider> =
            Arc::new(file_provider::DiskProvider::new(path_ctx, Some(hhi_root)));
        let decl_parser: DeclParser<BReason> = DeclParser::new(
            file_provider,
            DeclParserOptions::from_parser_options(&opts),
            opts.po_deregister_php_stdlib
        );
        let shallow_decl_store =
            make_shallow_decl_store(StoreOpts::Serialized(Compression::default()));
        let mut classes =
            populate_shallow_decl_store(&shallow_decl_store, decl_parser.clone(), &filenames);
        classes.sort();
        let folded_decl_provider = hackrs_test_utils::decl_provider::make_folded_decl_provider(
            StoreOpts::Serialized(Compression::default()),
            None,
            shallow_decl_store,
            Arc::new(opts),
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
                    .get_class(*class)
                    .expect("failed to fold class")
                    .expect("failed to look up class");
                (class.as_str().into(), folded_decl)
            })
            .collect();
        s.into_iter()
            .map(|(name, fc)| (name, fc))
            .collect()
    }
}
