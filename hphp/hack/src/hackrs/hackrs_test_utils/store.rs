// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::sync::Arc;

use datastore::NonEvictingStore;
use decl_parser::DeclParser;
use indicatif::ParallelProgressIterator;
use pos::RelativePath;
use pos::TypeName;
use rayon::iter::IntoParallelRefIterator;
use rayon::iter::ParallelIterator;
use shallow_decl_provider::ShallowDeclStore;
use ty::reason::Reason;

use crate::serde_store::StoreOpts;
use crate::SerializingStore;

pub fn make_shallow_decl_store<R: Reason>(opts: StoreOpts) -> ShallowDeclStore<R> {
    match opts {
        StoreOpts::Serialized(compression_type) => {
            ShallowDeclStore::new(
                Arc::new(SerializingStore::with_compression(compression_type)), // classes
                Arc::new(SerializingStore::with_compression(compression_type)), // typedefs
                Arc::new(SerializingStore::with_compression(compression_type)), // funs
                Arc::new(SerializingStore::with_compression(compression_type)), // consts
                Arc::new(SerializingStore::with_compression(compression_type)), // modules
                Arc::new(SerializingStore::with_compression(compression_type)), // properties
                Arc::new(SerializingStore::with_compression(compression_type)), // static_properties
                Arc::new(SerializingStore::with_compression(compression_type)), // methods
                Arc::new(SerializingStore::with_compression(compression_type)), // static_methods
                Arc::new(SerializingStore::with_compression(compression_type)), // constructors
            )
        }
        StoreOpts::Unserialized => ShallowDeclStore::with_no_member_stores(
            Arc::new(NonEvictingStore::default()), // classes
            Arc::new(NonEvictingStore::default()), // typedefs
            Arc::new(NonEvictingStore::default()), // funs
            Arc::new(NonEvictingStore::default()), // consts
            Arc::new(NonEvictingStore::default()), // modules
        ),
    }
}

pub fn make_non_evicting_shallow_decl_store<R: Reason>() -> ShallowDeclStore<R> {
    make_shallow_decl_store(StoreOpts::Unserialized)
}

pub fn populate_shallow_decl_store<R: Reason>(
    shallow_decl_store: &ShallowDeclStore<R>,
    decl_parser: DeclParser<R>,
    filenames: &[RelativePath],
) -> Vec<TypeName> {
    let len = filenames.len();
    filenames
        .par_iter()
        .progress_count(len as u64)
        .flat_map_iter(|path| {
            let (mut decls, summary) = decl_parser.parse_and_summarize(*path).unwrap();
            decls.reverse(); // To match OCaml behavior for name collisions
            shallow_decl_store.add_decls(decls).unwrap();
            summary
                .classes()
                .map(|(class, _hash)| TypeName::new(class))
                .collect::<Vec<_>>()
                .into_iter()
        })
        .collect()
}
