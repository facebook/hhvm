// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::collections::HashMap;

use bumpalo::{collections::Vec, Bump};

use arena_collections::AssocListMut;
use decl_provider_rust::{ClassDecl, ClassType, DeclProvider, FunDecl};
use oxidized::relative_path::RelativePath;
use oxidized_by_ref::ast;
use oxidized_by_ref::direct_decl_parser::Decls;
use oxidized_by_ref::s_map::SMap;
use oxidized_by_ref::s_set::SSet;
use oxidized_by_ref::shallow_decl_defs::ShallowClass;
use oxidized_by_ref::typing_defs_core::{ConsistentKind, Tparam, Ty, Ty_};
use oxidized_by_ref::typing_reason::Reason;

type StringMap<'a, T> = arena_collections::SortedAssocList<'a, &'a str, T>;

/// Dummy declaration provider. Implements the functionality needed to run hh_check.
/// Assumes that all the declarations come from a single file.
pub struct DummyDeclProvider<'a> {
    decls: &'a Decls<'a>,
    class_types: HashMap<String, &'a ClassType<'a>>,
}

impl<'a> DummyDeclProvider<'a> {
    pub fn new(path: &RelativePath, text: &'a [u8], arena: &'a Bump) -> Self {
        let decls = match decl_rust::direct_decl_parser::parse_decls(path.clone(), text, arena) {
            Err(e) => panic!("{:?}", e),
            Ok(decls) => arena.alloc(decls),
        };

        // Unlike the existing OCaml decl provider, classes in the direct decl parser don't seem to
        // have namespace qualification
        // TODO: fix this
        // decls.classes = decls
        //     .classes
        //     .into_iter()
        //     .map(|(k, v)| (format!("\\{}", k), v))
        //     .collect();

        let class_types = build_class_types(arena, &decls.classes);

        Self { decls, class_types }
    }
}

impl DeclProvider for DummyDeclProvider<'_> {
    fn get_fun(&self, name: &str) -> Option<&FunDecl<'_>> {
        let name = format!("\\{}", name);
        self.decls.funs.get(&*name)
    }

    fn get_shallow_class(&self, name: &str) -> Option<&ClassDecl<'_>> {
        self.decls.classes.get(&*name)
    }

    fn get_class(&self, name: &str) -> Option<&ClassType<'_>> {
        self.class_types.get(name).copied()
    }
}

/// Flatten out the class hierarchy.
fn build_class_types<'a>(
    arena: &'a Bump,
    decls: &'a StringMap<ShallowClass<'a>>,
) -> HashMap<String, &'a ClassType<'a>> {
    let mut class_types = HashMap::new();
    for (name, _) in decls.iter() {
        get_class_types_impl(arena, &mut class_types, decls, &name);
    }
    class_types
}

/// Recursively populate `class_types`.
fn get_class_types_impl<'a, 'b>(
    arena: &'a Bump,
    class_types: &mut HashMap<String, &'a ClassType<'a>>,
    decls: &StringMap<ShallowClass<'a>>,
    name: &str,
) -> Option<&'a ClassType<'a>> {
    match class_types.get(name) {
        Some(class_type) => Some(class_type),
        None => {
            // TODO: all this logic should be moved to where it belongs?

            let self_shallow = decls.get(name)?;
            // TODO(hrust): maybe inherit other tys?
            let mut ancestors = AssocListMut::new_in(arena);
            for cls in self_shallow.extends.iter() {
                let (_r, cls_name, tparaml) = unwrap_class_type(*cls);
                if let Some(cls_type) = get_class_types_impl(arena, class_types, decls, &cls_name.1)
                {
                    let subst = make_subst(arena, cls_type.tparams, tparaml);
                    for (ancestor_name, ancestor_ty) in cls_type.ancestors.iter() {
                        let ancestor_ty = instantiate(arena, &subst, *ancestor_ty);
                        ancestors.insert(*ancestor_name, ancestor_ty);
                    }
                }
                ancestors.insert(cls_name.1, cls.clone());
            }

            // TODO(hrust): build more fields
            let class_type = ClassType {
                need_init: false,
                members_fully_known: false,
                abstract_: false,
                final_: self_shallow.final_,
                const_: false,
                ppl: false,
                deferred_init_members: SSet::empty(),
                kind: self_shallow.kind,
                is_xhp: self_shallow.is_xhp,
                has_xhp_keyword: self_shallow.has_xhp_keyword,
                is_disposable: false,
                name: self_shallow.name.1,
                pos: self_shallow.name.0,
                tparams: self_shallow.tparams.clone(),
                where_constraints: self_shallow.where_constraints.clone(),
                consts: SMap::empty(),
                typeconsts: SMap::empty(),
                pu_enums: SMap::empty(),
                props: SMap::empty(),
                sprops: SMap::empty(),
                methods: SMap::empty(),
                smethods: SMap::empty(),
                construct: (None, ConsistentKind::Inconsistent),
                ancestors: SMap::from(arena, ancestors.iter().map(|(&k, &v)| (k, v))),
                req_ancestors: &[],
                req_ancestors_extends: oxidized_by_ref::s_set::SSet::empty(),
                extends: oxidized_by_ref::s_set::SSet::empty(),
                enum_type: None,
                sealed_whitelist: None,
                decl_errors: None,
            };
            let class_type = arena.alloc(class_type);

            class_types.insert(name.to_string(), class_type);
            Some(class_type)
        }
    }
}

fn make_subst<'a>(
    arena: &'a Bump,
    tparams: &'a [Tparam<'a>],
    paraml: &'a [Ty<'a>],
) -> StringMap<'a, Ty<'a>> {
    let mut subst = AssocListMut::new_in(arena);
    // TODO(hrust): tany
    for (tparam, ty) in tparams.into_iter().zip(paraml.iter()) {
        subst.insert(tparam.name.1, *ty);
    }
    subst.into()
}

fn instantiate<'a>(arena: &'a Bump, subst: &StringMap<'a, Ty<'a>>, ty: Ty<'a>) -> Ty<'a> {
    if subst.is_empty() {
        ty
    } else {
        match ty.unpack() {
            (r, Ty_::Tgeneric(x)) => match subst.get(x) {
                None => ty,
                Some(x_ty) => Ty(arena.alloc(Reason::instantiate(&x_ty.0, x, r)), x_ty.1),
            },
            (r, ty) => {
                let ty = instantiate_(arena, subst, ty);
                Ty(r, arena.alloc(ty))
            }
        }
    }
}

fn instantiate_<'a>(arena: &'a Bump, subst: &StringMap<'a, Ty<'a>>, ty: &Ty_<'a>) -> Ty_<'a> {
    use Ty_::*;
    match ty {
        Tgeneric(_) => panic!("impossible"),
        Tapply(&(x, tyl)) => {
            let tyl = Vec::from_iter_in(tyl.iter().map(|ty| instantiate(arena, subst, *ty)), arena);
            let tyl = arena.alloc(tyl);
            Tapply(arena.alloc((x, tyl.as_slice())))
        }
        _ => unimplemented!("{:?}", ty),
    }
}

fn unwrap_class_type<'a>(ty: Ty<'a>) -> (&'a Reason, ast::Sid<'a>, &'a [Ty<'a>]) {
    match ty.unpack() {
        (r, Ty_::Tapply(&(name, tparaml))) => (r, name, tparaml),
        _ => {
            // TODO(hrust): error handling
            unimplemented!("{:?}", ty)
        }
    }
}
