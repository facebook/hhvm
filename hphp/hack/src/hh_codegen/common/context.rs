// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::VecDeque;
use std::path::Path;

use anyhow::Result;
use anyhow::anyhow;
use hash::IndexMap;
use hash::IndexSet;
use synstructure::Structure;

use crate::common::syn_helpers;

/// A simplified version of `crate::gen_visitor::context::Context`. Contains all
/// of the ASTs for `syn::Item` definitions provided in the constructor, except
/// those whose types are not reachable from the given root type.
pub struct Context {
    defs: IndexMap<String, syn::DeriveInput>,
    mods: IndexSet<String>,
}

impl Context {
    /// Construct a `Context` containing the ASTs of all type definitions
    /// reachable from the `root` type. Each type must have a unique name (even
    /// if the types are declared in different modules).
    pub fn new(files: &[(&Path, Vec<syn::Item>)], root: &str) -> Result<Self> {
        Self::with_extern_files(files, &[], root)
    }

    /// Construct a `Context` containing the ASTs of all type definitions
    /// reachable from the `root` type. Each type must have a unique name (even
    /// if the types are declared in different modules).
    ///
    /// `extern_files` is used to provide the definitions of types which are
    /// declared in `extern_files` and re-exported in `files` (e.g., when using
    /// `oxidized_by_ref` for `files`, use `oxidized` for `extern_files`, since
    /// `oxidized_by_ref` re-exports types defined in `oxidized`).
    pub fn with_extern_files(
        files: &[(&Path, Vec<syn::Item>)],
        extern_files: &[(&Path, Vec<syn::Item>)],
        root: &str,
    ) -> Result<Self> {
        let mut defs = IndexMap::default();
        let mut mods = IndexSet::default();
        for (filename, items) in files {
            eprintln!("Processing {:?}", filename);
            for item in items.iter() {
                if let Ok(name) = syn_helpers::get_ty_def_name(item) {
                    if defs.contains_key(&name) {
                        return Err(anyhow!("Type {} already exists, file {:?}", name, filename));
                    }
                    defs.insert(name, item);
                }
            }
            // assuming file name is the module name
            mods.insert(
                filename
                    .file_stem()
                    .and_then(|stem| stem.to_str())
                    .unwrap()
                    .into(),
            );
        }
        // The "extern" files provide the definitions of types which were
        // imported from the oxidized crate to the oxidized_by_ref crate via
        // an extern_types.txt file.
        for (filename, items) in extern_files {
            eprintln!("Processing extern file {:?}", filename);
            for item in items.iter() {
                if let Ok(name) = syn_helpers::get_ty_def_name(item) {
                    // Don't overwrite a definition if one is already there--we
                    // only need to fill in the ones which are missing (because
                    // they were re-exported from oxidized).
                    defs.entry(name).or_insert(item);
                }
            }
        }
        let reachable = Self::get_all_tys(&defs, root)?;
        let defs = defs
            .into_iter()
            .filter(|(ty_name, _)| reachable.contains(ty_name.as_str()))
            .filter_map(|(ty_name, item)| {
                use syn::Item::*;
                match item {
                    Struct(item_struct) => Some((ty_name, item_struct.clone().into())),
                    Enum(item_enum) => Some((ty_name, item_enum.clone().into())),
                    _ => None,
                }
            })
            .collect();
        Ok(Self { defs, mods })
    }

    /// Return all the names of modules provided in the `files` argument to
    /// `Self::new`. Assumes that each file has the same name as the module it
    /// declares (i.e., no mod.rs files).
    pub fn modules(&self) -> impl Iterator<Item = &str> {
        self.mods.iter().map(|s| s.as_ref())
    }

    pub fn types(&self) -> impl Iterator<Item = &syn::DeriveInput> {
        self.defs.values()
    }

    pub fn type_structures(&self) -> impl Iterator<Item = Structure<'_>> {
        self.types().map(Structure::new)
    }

    fn get_all_tys(defs: &IndexMap<String, &syn::Item>, root: &str) -> Result<IndexSet<String>> {
        let defined_types = defs.keys().map(|s| s.as_str()).collect();
        let mut visited = IndexSet::<String>::default();
        let mut q = VecDeque::new();
        q.push_back(root.into());
        while let Some(ty) = q.pop_front() {
            let item = defs
                .get(&ty)
                .ok_or_else(|| anyhow!("Type {} not found", ty))?;
            visited.insert(ty);
            let deps = syn_helpers::get_dep_tys(&defined_types, item)?;
            for d in deps.into_iter() {
                if !visited.contains(&d) {
                    q.push_back(d);
                }
            }
        }
        Ok(visited)
    }
}
