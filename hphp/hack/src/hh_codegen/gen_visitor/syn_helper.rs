// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::common::Result;
use quote::format_ident;
use std::collections::HashSet;
use syn::*;

pub fn is_alias(i: &Item) -> bool {
    match i {
        Item::Type(_) => true,
        _ => false,
    }
}

pub fn get_ty_def_name(i: &Item) -> Result<String> {
    use Item::*;
    match i {
        Enum(ItemEnum { ident, .. })
        | Struct(ItemStruct { ident, .. })
        | Type(ItemType { ident, .. }) => Ok(ident.to_string()),
        _ => Err(format!("Not supported {:?}", i).into()),
    }
}

pub fn get_ty_params(i: &Item) -> Result<Vec<String>> {
    use Item::*;
    match i {
        Enum(ItemEnum { generics, .. })
        | Struct(ItemStruct { generics, .. })
        | Type(ItemType { generics, .. }) => Ok(TypeParamCollector::on_generics(generics)),
        _ => Err(format!("Not supported {:?}", i).into()),
    }
}

pub fn get_ty_param_idents(i: &Item) -> Result<impl Iterator<Item = Ident>> {
    Ok(get_ty_params(i)?
        .into_iter()
        .map(|t| format_ident!("{}", t)))
}

pub fn get_dep_tys(defined_types: &HashSet<&str>, i: &Item) -> Result<Vec<String>> {
    use Item::*;
    match i {
        Enum(ItemEnum { variants, .. }) => Ok(variants
            .iter()
            .fold(HashSet::<String>::new(), |mut a, v| {
                for ty in LeafTyCellector::on_fields(Some(defined_types), &v.fields) {
                    a.insert(ty);
                }
                a
            })
            .into_iter()
            .collect()),
        Type(ItemType { ty, .. }) => {
            Ok(LeafTyCellector::on_type(Some(defined_types), ty.as_ref()).collect())
        }
        Struct(ItemStruct { fields, .. }) => {
            Ok(LeafTyCellector::on_fields(Some(defined_types), fields).collect())
        }
        _ => Err(format!("Not supported {:?}", i).into()),
    }
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

pub struct LeafTyCellector {
    pub discovered_types: HashSet<String>,
}

impl LeafTyCellector {
    pub fn new() -> Self {
        Self {
            discovered_types: HashSet::new(),
        }
    }

    pub fn on_type<'a>(
        filter: Option<&'a HashSet<&'a str>>,
        ty: &Type,
    ) -> impl Iterator<Item = String> + 'a {
        let mut collector = Self::new();
        visit::visit_type(&mut collector, ty);
        collector
            .discovered_types
            .into_iter()
            .filter(move |s| filter.map_or(true, |f| f.contains(s.as_str())))
    }

    pub fn on_fields<'a>(
        filter: Option<&'a HashSet<&'a str>>,
        fields: &Fields,
    ) -> impl Iterator<Item = String> + 'a {
        let mut collector = Self::new();
        visit::visit_fields(&mut collector, fields);
        collector
            .discovered_types
            .into_iter()
            .filter(move |s| filter.map_or(true, |f| f.contains(s.as_str())))
    }
}

impl<'ast> visit::Visit<'ast> for LeafTyCellector {
    fn visit_path_segment(&mut self, node: &'ast PathSegment) {
        let ty = node.ident.to_string();
        self.discovered_types.insert(ty);
        visit::visit_path_segment(self, node);
    }
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
