// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io;
use std::path::Path;
use std::path::PathBuf;

use anyhow::Result;
use bumpalo::Bump;
use ocamlrep_ocamlpool::ocaml_ffi;
use oxidized::decl_parser_options::DeclParserOptions;
use oxidized::file_info::FileInfo;
use oxidized::search_types::SiAddendum;
use oxidized_by_ref::direct_decl_parser::ParsedFileWithHashes;
use rayon::prelude::*;
use relative_path::RelativePath;
use unwrap_ocaml::UnwrapOcaml;

ocaml_ffi! {
    fn batch_index_hackrs_ffi_root_relative_paths_only(
        parser_options: DeclParserOptions,
        deregister_php_stdlib_if_hhi: bool,
        root: PathBuf,
        filenames: Vec<(RelativePath, Option<Option<Vec<u8>>>)>,
    ) -> Vec<(RelativePath, Option<(FileInfo, Vec<SiAddendum>)>)> {
        let filenames_and_contents = par_read_file_root_only(&root, filenames).unwrap_ocaml();
        let filenames_and_contents: Vec<_> = filenames_and_contents
            .into_par_iter()
            .map(|(relpath, contents)| {
                let contents = match contents {
                    Some(contents) => contents,
                    None => return (relpath, None)
                };

                let arena = Bump::new();

                let parsed_file = direct_decl_parser::parse_decls_for_typechecking(
                    &parser_options,
                    relpath.clone(),
                    &contents,
                    &arena,
                );

                let with_hashes = ParsedFileWithHashes::new(
                    parsed_file,
                    deregister_php_stdlib_if_hhi,
                    relpath.prefix(),
                    &arena,
                );

                let addenda = si_addendum::get_si_addenda(&with_hashes);
                let file_info: hackrs_provider_backend::FileInfo = with_hashes.into();

                (relpath, Some((file_info, addenda)))
            })
            .collect();
        filenames_and_contents.into_iter().map(|(relpath, contents)| {
            (relpath, contents.map(|(with_hashes, addenda)| {
                (with_hashes.into(), addenda)}))
            }).collect()
    }
}

// For each file in filenames, return a tuple of its path followed by `Some` of
// its contents if the file is found, otherwise `None`.
// The filenames are either [("file1.php", None); ("absent.php", None)] to
// indicate that we here in Rust should try to fetch the contents,
// or [("file1.php", Some(Some(present))); ("absent.php", Some(None))] to
// indicate that the content was supplied by our ocaml caller (used for
// testing only, since the ocaml TestDisk isn't available to Rust).
fn par_read_file_root_only(
    root: &Path,
    filenames: Vec<(RelativePath, Option<Option<Vec<u8>>>)>,
) -> Result<Vec<(RelativePath, Option<Vec<u8>>)>> {
    filenames
        .into_par_iter()
        .map(|(relpath, test_contents)| {
            if let Some(test_contents) = test_contents {
                Ok((relpath, test_contents))
            } else {
                let prefix = relpath.prefix();
                let abspath = match prefix {
                    relative_path::Prefix::Root => root.join(relpath.path()),
                    _ => panic!("should only be reading files relative to root"),
                };
                match std::fs::read(abspath) {
                    Ok(text) => Ok((relpath, Some(text))),
                    Err(e) if e.kind() == io::ErrorKind::NotFound => Ok((relpath, None)),
                    Err(e) => Err(e.into()),
                }
            }
        })
        .collect()
}
