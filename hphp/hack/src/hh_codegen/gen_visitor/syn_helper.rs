// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::{anyhow, Result};
use quote::format_ident;
use syn::*;

pub use crate::common::syn_helpers::{get_dep_tys, get_ty_def_name};

pub fn is_alias(i: &Item) -> bool {
    match i {
        Item::Type(_) => true,
        _ => false,
    }
}

pub fn get_ty_params(i: &Item) -> Result<Vec<String>> {
    use Item::*;
    match i {
        Enum(ItemEnum { generics, .. })
        | Struct(ItemStruct { generics, .. })
        | Type(ItemType { generics, .. }) => Ok(TypeParamCollector::on_generics(generics)),
        _ => Err(anyhow!("Not supported {:?}", i)),
    }
}

pub fn get_ty_param_idents(i: &Item) -> Result<impl Iterator<Item = Ident>> {
    Ok(get_ty_params(i)?
        .into_iter()
        .map(|t| format_ident!("{}", t)))
}

pub fn get_field_and_type_from_named<'a>(
    FieldsNamed { named, .. }: &'a FieldsNamed,
) -> Vec<(String, &'a Type)> {
    named
        .iter()
        .map(|f| (f.ident.as_ref().unwrap().to_string(), &f.ty))
        .collect()
}

pub fn get_field_and_type_from_unnamed(
    FieldsUnnamed { unnamed, .. }: &FieldsUnnamed,
) -> impl Iterator<Item = (usize, &Type)> {
    itertools::enumerate(unnamed.into_iter().map(|f| &f.ty))
}

struct TypeParamCollector(Vec<String>);

impl TypeParamCollector {
    pub fn on_generics(g: &Generics) -> Vec<String> {
        let mut collector = Self(vec![]);
        visit::visit_generics(&mut collector, g);
        collector.0
    }
}

impl<'ast> visit::Visit<'ast> for TypeParamCollector {
    fn visit_type_param(&mut self, node: &'ast TypeParam) {
        self.0.push(node.ident.to_string())
    }
}

pub fn try_get_types_from_box_tuple(
    FieldsUnnamed { unnamed, .. }: &FieldsUnnamed,
) -> Option<impl Iterator<Item = (usize, &Type)>> {
    let fields = unnamed.into_iter().collect::<Vec<_>>();
    if fields.len() == 1 {
        if let Type::Path(TypePath { path, .. }) = &fields[0].ty {
            if let Some(path_seg) = path.segments.first() {
                if path_seg.ident == "Box" {
                    if let syn::PathArguments::AngleBracketed(args) = &path_seg.arguments {
                        if let Some(GenericArgument::Type(Type::Tuple(syn::TypeTuple {
                            elems,
                            ..
                        }))) = args.args.first()
                        {
                            return Some(itertools::enumerate(elems.iter()));
                        }
                    }
                }
            }
        }
    }
    None
}

pub fn try_simple_type(ty: &Type) -> Option<String> {
    if let Type::Path(TypePath { path, .. }) = ty {
        if path.segments.len() == 1 {
            let ty = path.segments.first().unwrap();
            if ty.arguments.is_empty() {
                return Some(ty.ident.to_string());
            }
        }
    }
    None
}
