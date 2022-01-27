// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::rc::Rc;
use std::{fs, io};

use bumpalo::Bump;

use hcons::Hc;

use crate::alloc::Allocator;
use crate::decl_defs::{ShallowClass, ShallowFun, ShallowMethod};
use crate::pos::{RelativePath, RelativePathCtx, Symbol};
use crate::reason::Reason;
use crate::shallow_decl_provider::ShallowDeclCache;

#[derive(Debug)]
pub struct ShallowDeclProvider<R: Reason> {
    cache: Rc<dyn ShallowDeclCache<Reason = R>>,
    alloc: &'static Allocator<R>,
    relative_path_ctx: Rc<RelativePathCtx>,
}

impl<R: Reason> ShallowDeclProvider<R> {
    pub fn new(
        cache: Rc<dyn ShallowDeclCache<Reason = R>>,
        alloc: &'static Allocator<R>,
        relative_path_ctx: Rc<RelativePathCtx>,
    ) -> Self {
        Self {
            cache,
            alloc,
            relative_path_ctx,
        }
    }

    pub fn get_shallow_class(&self, name: &Symbol) -> Option<Rc<ShallowClass<R>>> {
        self.cache.get_shallow_class(name)
    }

    pub fn add_from_oxidized_class(&self, sc: &oxidized_by_ref::shallow_decl_defs::ClassDecl<'_>) {
        let res = Rc::new(self.utils().shallow_class(sc));
        self.cache.put_shallow_class(Hc::clone(res.name.id()), res);
    }

    pub fn add_from_oxidized_fun(
        &self,
        name: &str,
        sf: &oxidized_by_ref::shallow_decl_defs::FunDecl<'_>,
    ) {
        let res = Rc::new(self.utils().shallow_fun(sf));
        let name = self.alloc.symbol(name);
        self.cache.put_shallow_fun(name, res);
    }

    pub fn add_from_oxidized_decls(&self, decls: &oxidized_by_ref::direct_decl_parser::Decls<'_>) {
        for (name, decl) in decls.iter() {
            use oxidized_by_ref::direct_decl_parser::Decl::*;
            match decl {
                Class(sc) => drop(self.add_from_oxidized_class(sc)),
                Fun(sf) => drop(self.add_from_oxidized_fun(name, sf)),
                decl => unimplemented!("new_local_with_decls: {:?}", decl),
            }
        }
    }

    pub fn add_from_files(
        &self,
        filenames: &mut dyn Iterator<Item = &RelativePath>,
    ) -> io::Result<()> {
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

    fn utils(&self) -> ShallowDeclUtils<R> {
        ShallowDeclUtils::new(self.alloc)
    }
}

struct ShallowDeclUtils<R: Reason> {
    alloc: &'static Allocator<R>,
}

impl<R: Reason> ShallowDeclUtils<R> {
    fn new(alloc: &'static Allocator<R>) -> Self {
        Self { alloc }
    }

    fn shallow_method(
        &self,
        sm: &oxidized_by_ref::shallow_decl_defs::ShallowMethod<'_>,
    ) -> ShallowMethod<R> {
        ShallowMethod {
            name: self.alloc.pos_id_from_decl(sm.name),
            ty: self.alloc.decl_ty_from_ast(sm.type_),
            visibility: sm.visibility,
        }
    }

    fn shallow_class(
        &self,
        sc: &oxidized_by_ref::shallow_decl_defs::ClassDecl<'_>,
    ) -> ShallowClass<R> {
        ShallowClass {
            name: self.alloc.pos_id_from_decl(sc.name),
            extends: sc
                .extends
                .iter()
                .map(|ty| self.alloc.decl_ty_from_ast(ty))
                .collect(),
            methods: sc
                .methods
                .iter()
                .map(|sm| self.shallow_method(sm))
                .collect(),
            module: sc
                .module
                .as_ref()
                .map(|id| self.alloc.pos_id_from_ast_ref(id)),
        }
    }

    fn shallow_fun(&self, sf: &oxidized_by_ref::shallow_decl_defs::FunDecl<'_>) -> ShallowFun<R> {
        ShallowFun {
            pos: self.alloc.pos_from_decl(sf.pos),
            ty: self.alloc.decl_ty_from_ast(sf.type_),
        }
    }
}
