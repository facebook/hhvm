// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::ir;

pub fn rewrite_file(file: &mut ir::File) {
    file.defs.values_mut().for_each(rewrite_def)
}

fn rewrite_def(def: &mut ir::Def) {
    match def {
        ir::Def::Alias { ty, .. } => rewrite_type(ty),
        ir::Def::Record { fields, .. } => fields.iter_mut().for_each(rewrite_field),
        ir::Def::Variant { variants, .. } => variants.iter_mut().for_each(rewrite_variant),
    }
}

fn rewrite_field(field: &mut ir::Field) {
    rewrite_type(&mut field.ty)
}

fn rewrite_variant(variant: &mut ir::Variant) {
    variant.fields.iter_mut().for_each(rewrite_variant_fields)
}

fn rewrite_variant_fields(fields: &mut ir::VariantFields) {
    match fields {
        ir::VariantFields::Unnamed(tys) => tys.iter_mut().for_each(rewrite_type),
        ir::VariantFields::Named(fields) => fields.iter_mut().for_each(rewrite_field),
    }
}

fn rewrite_type(ty: &mut ir::Type) {
    match ty {
        ir::Type::Path(path) => {
            let modules: Vec<_> = path.modules.iter().map(String::as_str).collect();
            match (modules.as_slice(), path.ty.as_str(), path.targs.as_slice()) {
                // Remove pointer types; every block is behind an indirection in OCaml
                ([] | ["std", "boxed"], "Box", [_targ])
                | ([] | ["std", "rc"], "Rc", [_targ])
                | ([] | ["std", "sync"], "Arc", [_targ])
                | ([] | ["ocamlrep", "rc"], "RcOc", [_targ]) => {
                    *ty = path.targs.pop().unwrap();
                    rewrite_type(ty);
                }
                _ => rewrite_type_path(path),
            }
        }
        ir::Type::Tuple(tuple) => rewrite_type_tuple(tuple),
    }
}

fn rewrite_type_path(path: &mut ir::TypePath) {
    path.targs.iter_mut().for_each(rewrite_type)
}

fn rewrite_type_tuple(tuple: &mut ir::TypeTuple) {
    tuple.elems.iter_mut().for_each(rewrite_type)
}
