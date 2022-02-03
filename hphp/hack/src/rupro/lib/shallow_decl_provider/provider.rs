// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::alloc::Allocator;
use crate::decl_defs::ShallowClass;
use crate::reason::Reason;
use crate::shallow_decl_provider::ShallowDeclCache;
use bumpalo::Bump;
use pos::{RelativePath, RelativePathCtx, TypeName};
use std::sync::Arc;
use std::{fs, io};

#[derive(Debug)]
pub struct ShallowDeclProvider<R: Reason> {
    cache: Arc<dyn ShallowDeclCache<Reason = R>>,
    alloc: &'static Allocator<R>,
    relative_path_ctx: Arc<RelativePathCtx>,
}

impl<R: Reason> ShallowDeclProvider<R> {
    pub fn new(
        cache: Arc<dyn ShallowDeclCache<Reason = R>>,
        alloc: &'static Allocator<R>,
        relative_path_ctx: Arc<RelativePathCtx>,
    ) -> Self {
        Self {
            cache,
            alloc,
            relative_path_ctx,
        }
    }

    pub fn get_shallow_class(&self, name: TypeName) -> Option<Arc<ShallowClass<R>>> {
        self.cache.get_shallow_class(name)
    }

    pub fn add_from_oxidized_class(&self, sc: &oxidized_by_ref::shallow_decl_defs::ClassDecl<'_>) {
        let res = Arc::new(self.alloc.shallow_class(sc));
        self.cache.put_shallow_class(res.name.id(), res);
    }

    pub fn add_from_oxidized_fun(
        &self,
        name: &str,
        sf: &oxidized_by_ref::shallow_decl_defs::FunDecl<'_>,
    ) {
        let res = Arc::new(self.alloc.shallow_fun(sf));
        let name = self.alloc.symbol(name);
        self.cache.put_shallow_fun(name, res);
    }

    pub fn add_from_oxidized_decls(&self, decls: &oxidized_by_ref::direct_decl_parser::Decls<'_>) {
        for (name, decl) in decls.iter() {
            use oxidized_by_ref::direct_decl_parser::Decl::*;
            match decl {
                Class(sc) => self.add_from_oxidized_class(sc),
                Fun(sf) => self.add_from_oxidized_fun(name, sf),
                decl => unimplemented!("new_local_with_decls: {:?}", decl),
            }
        }
    }

    pub fn add_from_files(&self, filenames: impl Iterator<Item = RelativePath>) -> io::Result<()> {
        for rel_fln in filenames {
            let arena = Bump::new();
            let fln = rel_fln.to_absolute(&self.relative_path_ctx);
            let text = arena.alloc_slice_clone(fs::read_to_string(&fln)?.as_bytes());
            let rel_path = oxidized::relative_path::RelativePath::make(
                oxidized::relative_path::Prefix::Dummy,
                fln,
            );
            let parsed_file = stack_limit::with_elastic_stack(|stack_limit| {
                direct_decl_parser::parse_decls(
                    oxidized_by_ref::decl_parser_options::DeclParserOptions::DEFAULT,
                    rel_path.clone(),
                    text,
                    &arena,
                    Some(stack_limit),
                )
            })
            .unwrap_or_else(|failure| {
                panic!(
                    "Rust decl parser FFI exceeded maximum allowed stack of {} KiB",
                    failure.max_stack_size_tried / stack_limit::KI
                );
            });
            self.add_from_oxidized_decls(&parsed_file.decls);
        }
        Ok(())
    }
}
