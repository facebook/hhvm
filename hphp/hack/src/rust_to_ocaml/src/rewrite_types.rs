// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use convert_case::Case;
use convert_case::Casing;

use crate::ir;

pub fn rewrite_file(file: &mut ir::File) {
    Rewriter::rewrite_module(&mut file.root)
}

struct Rewriter {
    module_name: ir::ModuleName,
}

impl Rewriter {
    fn rewrite_module(module: &mut ir::Module) {
        let this = Self {
            module_name: module.name.clone(),
        };
        module.defs.iter_mut().for_each(|def| this.rewrite_def(def))
    }

    fn rewrite_def(&self, def: &mut ir::Def) {
        let rewrite_name = |name: &mut ir::TypeName| {
            if name.as_str() == self.module_name.as_str() {
                *name = ir::TypeName(String::from("t"));
            }
        };
        match def {
            ir::Def::Module(module) => Self::rewrite_module(module),
            ir::Def::Alias { name, ty, .. } => {
                rewrite_name(name);
                self.rewrite_type(ty)
            }
            ir::Def::Record { name, fields, .. } => {
                rewrite_name(name);
                fields.iter_mut().for_each(|f| self.rewrite_field(f))
            }
            ir::Def::Variant { name, variants, .. } => {
                rewrite_name(name);
                variants.iter_mut().for_each(|v| self.rewrite_variant(v))
            }
        }
    }

    fn rewrite_field(&self, field: &mut ir::Field) {
        self.rewrite_type(&mut field.ty)
    }

    fn rewrite_variant(&self, variant: &mut ir::Variant) {
        variant
            .fields
            .iter_mut()
            .for_each(|f| self.rewrite_variant_fields(f))
    }

    fn rewrite_variant_fields(&self, fields: &mut ir::VariantFields) {
        match fields {
            ir::VariantFields::Unnamed(tys) => tys.iter_mut().for_each(|ty| self.rewrite_type(ty)),
            ir::VariantFields::Named(fields) => {
                fields.iter_mut().for_each(|f| self.rewrite_field(f))
            }
        }
    }

    fn rewrite_type(&self, ty: &mut ir::Type) {
        match ty {
            ir::Type::Path(path) => {
                let modules: Vec<_> = path.modules.iter().map(ir::ModuleName::as_str).collect();
                match (modules.as_slice(), path.ty.as_str(), path.targs.as_slice()) {
                    // Remove pointer types; every block is behind an indirection in OCaml
                    ([] | ["std", "boxed"], "Box", [_targ])
                    | ([] | ["std", "rc"], "Rc", [_targ])
                    | ([] | ["std", "sync"], "Arc", [_targ])
                    | ([] | ["ocamlrep", "rc"], "RcOc", [_targ]) => {
                        *ty = path.targs.pop().unwrap();
                        self.rewrite_type(ty);
                    }
                    ([] | ["std", "vec"], "Vec", [_targ]) => {
                        path.modules = vec![];
                        path.ty = ir::TypeName(String::from("list"));
                        path.targs.iter_mut().for_each(|ty| self.rewrite_type(ty));
                    }
                    _ => self.rewrite_type_path(path),
                }
            }
            ir::Type::Tuple(tuple) => self.rewrite_type_tuple(tuple),
        }
    }

    fn rewrite_type_path(&self, path: &mut ir::TypePath) {
        match path.modules.get(0).map(ir::ModuleName::as_str) {
            Some("crate" | "super") => {
                path.modules.remove(0);
            }
            _ => {}
        }
        let modules: Vec<_> = path.modules.iter().map(ir::ModuleName::as_str).collect();
        match (modules.as_slice(), path.ty.as_str(), path.targs.as_slice()) {
            // Convert all integer types to `int`. The impls of ToOcamlRep
            // and FromOcamlRep for integer types do checked conversions, so
            // we'll fail at runtime if our int value doesn't fit into
            // OCaml's integer width.
            (
                [],
                "i8" | "u8" | "i16" | "u16" | "i32" | "u32" | "i64" | "u64" | "i128" | "u128"
                | "isize" | "usize",
                [],
            ) => path.ty = ir::TypeName(String::from("int")),
            _ => {
                let ty = path.ty.as_str().to_case(Case::Snake);
                let ty_matches_last_module_in_path =
                    modules.last().map_or(false, |&module| ty == module);
                if ty_matches_last_module_in_path || ty == self.module_name.as_str() {
                    path.ty = ir::TypeName(String::from("t"));
                }
                path.targs.iter_mut().for_each(|ty| self.rewrite_type(ty))
            }
        }
    }

    fn rewrite_type_tuple(&self, tuple: &mut ir::TypeTuple) {
        tuple.elems.iter_mut().for_each(|ty| self.rewrite_type(ty))
    }
}
