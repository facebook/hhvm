// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::ir;
use crate::Config;

pub fn rewrite_file(config: &'static Config, file: &mut ir::File) {
    let rewriter = Rewriter { config };
    rewriter.rewrite_module(&mut file.root)
}

struct Rewriter {
    config: &'static Config,
}

impl Rewriter {
    fn rewrite_module(&self, module: &mut ir::Module) {
        module.defs.iter_mut().for_each(|def| self.rewrite_def(def))
    }

    fn rewrite_def(&self, def: &mut ir::Def) {
        match def {
            ir::Def::Module(module) => self.rewrite_module(module),
            ir::Def::Alias { ty, .. } => self.rewrite_type(ty),
            ir::Def::Record { fields, .. } => fields.iter_mut().for_each(|f| self.rewrite_field(f)),
            ir::Def::Variant { variants, .. } => {
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
            ir::Type::Path(path) => self.rewrite_type_path(path),
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
        path.modules.iter_mut().for_each(|m| {
            if let Some(name) = self.config.get_renamed_module(m) {
                *m = name;
            }
        });
        path.targs.iter_mut().for_each(|ty| self.rewrite_type(ty))
    }

    fn rewrite_type_tuple(&self, tuple: &mut ir::TypeTuple) {
        tuple.elems.iter_mut().for_each(|ty| self.rewrite_type(ty))
    }
}
