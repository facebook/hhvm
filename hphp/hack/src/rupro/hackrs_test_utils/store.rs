// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::serde_store::StoreOpts;
use crate::SerializingStore;
use datastore::NonEvictingStore;
use hackrs::{decl_parser::DeclParser, shallow_decl_provider::ShallowDeclStore};
use indicatif::ParallelProgressIterator;
use pos::{RelativePath, TypeName};
use rayon::iter::{IntoParallelRefIterator, ParallelIterator};
use std::sync::Arc;
use ty::reason::Reason;

pub fn make_shallow_decl_store<R: Reason>(opts: StoreOpts) -> ShallowDeclStore<R> {
    match opts {
        StoreOpts::Serialized(compression_type) => {
            ShallowDeclStore::new(
                Arc::new(SerializingStore::with_compression(compression_type)), // types
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
            Arc::new(NonEvictingStore::default()),
            Arc::new(NonEvictingStore::default()),
            Arc::new(NonEvictingStore::default()),
            Arc::new(NonEvictingStore::default()),
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
