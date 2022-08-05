// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::ir;
use crate::ir::Def;
use crate::ir::FieldName;
use crate::ir::File;
use crate::ir::TypeName;
use crate::ir::VariantName;
use anyhow::bail;
use anyhow::ensure;
use anyhow::Context;
use anyhow::Result;
use convert_case::Case;
use convert_case::Casing;

pub fn convert_file(file: &syn::File) -> Result<String> {
    let defs = (file.items.iter())
        .filter_map(|item| ItemConverter::convert_item(item).transpose())
        .collect::<Result<_>>()?;
    let mut file = File { defs };
    crate::rewrite_types::rewrite_file(&mut file);
    Ok(file.to_string())
}

struct ItemConverter {
    tparams: Vec<String>,
}

impl ItemConverter {
    fn convert_item(item: &syn::Item) -> Result<Option<(TypeName, Def)>> {
        use syn::Item;
        match item {
            Item::Type(item) => {
                let this = ItemConverter::new(&item.generics);
                Ok(Some(this.convert_item_type(item).with_context(|| {
                    format!("Failed to convert type {}", item.ident)
                })?))
            }
            Item::Struct(item) => {
                let this = ItemConverter::new(&item.generics);
                Ok(Some(this.convert_item_struct(item).with_context(|| {
                    format!("Failed to convert type {}", item.ident)
                })?))
            }
            Item::Enum(item) => {
                let this = ItemConverter::new(&item.generics);
                Ok(Some(this.convert_item_enum(item).with_context(|| {
                    format!("Failed to convert type {}", item.ident)
                })?))
            }
            _ => Ok(None),
        }
    }

    fn new(generics: &syn::Generics) -> Self {
        let tparams = generics
            .type_params()
            .map(|tparam| tparam.ident.to_string())
            .collect();
        Self { tparams }
    }

    fn convert_item_type(self, item: &syn::ItemType) -> Result<(TypeName, Def)> {
        let name = TypeName(item.ident.to_string().to_case(Case::Snake));
        let doc = attr_parser::get_doc_comment(&item.attrs);
        let ty = self.convert_type(&item.ty)?;
        Ok((
            name,
            Def::Alias {
                doc,
                tparams: self.tparams,
                ty,
            },
        ))
    }

    fn convert_item_struct(self, item: &syn::ItemStruct) -> Result<(TypeName, Def)> {
        let name = TypeName(item.ident.to_string().to_case(Case::Snake));
        let container_attrs = attr_parser::Container::from_ast(&item.to_owned().into());
        match &item.fields {
            syn::Fields::Named(fields) => {
                let fields = (fields.named.iter())
                    .map(|field| {
                        let field_attrs = attr_parser::Field::from_ast(field);
                        let name =
                            field_name(field.ident.as_ref(), container_attrs.prefix.as_deref());
                        let ty = self.convert_type(&field.ty)?;
                        let doc = field_attrs.doc;
                        Ok(ir::Field { name, ty, doc })
                    })
                    .collect::<Result<Vec<_>>>()?;
                let doc = container_attrs.doc;
                Ok((
                    name,
                    Def::Record {
                        doc,
                        tparams: self.tparams,
                        fields,
                    },
                ))
            }
            _ => todo!(),
        }
    }

    fn convert_item_enum(self, item: &syn::ItemEnum) -> Result<(TypeName, Def)> {
        let name = TypeName(item.ident.to_string().to_case(Case::Snake));
        let container_attrs = attr_parser::Container::from_ast(&item.to_owned().into());
        let variants = item
            .variants
            .iter()
            .map(|variant| {
                let name = variant_name(&variant.ident, container_attrs.prefix.as_deref());
                let variant_attrs = attr_parser::Variant::from_ast(variant);
                let fields = match &variant.fields {
                    syn::Fields::Unit => None,
                    syn::Fields::Unnamed(fields) => Some(ir::VariantFields::Unnamed(
                        (fields.unnamed.iter())
                            .map(|field| self.convert_type(&field.ty))
                            .collect::<Result<_>>()?,
                    )),
                    syn::Fields::Named(fields) => Some(ir::VariantFields::Named(
                        (fields.named.iter())
                            .map(|field| {
                                let field_attrs = attr_parser::Field::from_ast(field);
                                let name = field_name(
                                    field.ident.as_ref(),
                                    variant_attrs.prefix.as_deref(),
                                );
                                let ty = self.convert_type(&field.ty)?;
                                let doc = field_attrs.doc;
                                Ok(ir::Field { name, ty, doc })
                            })
                            .collect::<Result<_>>()?,
                    )),
                };
                let doc = variant_attrs.doc;
                Ok(ir::Variant { name, fields, doc })
            })
            .collect::<Result<Vec<_>>>()?;
        let doc = container_attrs.doc;
        Ok((
            name,
            Def::Variant {
                doc,
                tparams: self.tparams,
                variants,
            },
        ))
    }

    fn convert_type(&self, ty: &syn::Type) -> Result<ir::Type> {
        match ty {
            syn::Type::Path(ty) => Ok(ir::Type::Path(self.convert_type_path(ty)?)),
            syn::Type::Tuple(ty) => Ok(ir::Type::Tuple(ir::TypeTuple {
                elems: (ty.elems.iter())
                    .map(|e| self.convert_type(e))
                    .collect::<Result<_>>()?,
            })),
            _ => bail!("Not supported: {:?}", ty),
        }
    }

    fn convert_type_path(&self, ty: &syn::TypePath) -> Result<ir::TypePath> {
        ensure!(ty.qself.is_none(), "Qualified self in paths not supported");
        let last_seg = ty.path.segments.last().unwrap();
        if ty.path.segments.len() == 1 && last_seg.arguments.is_empty() {
            let ident = last_seg.ident.to_string();
            if self.tparams.contains(&ident) {
                let tparam = format!("'{}", ident.to_case(Case::Snake));
                return Ok(ir::TypePath::simple(tparam));
            }
        }
        let segments_except_last = ty.path.segments.iter().rev().skip(1).rev();
        Ok(ir::TypePath {
            modules: segments_except_last
                .map(|seg| {
                    ensure!(
                        seg.arguments.is_empty(),
                        "Type args only supported in last path segment"
                    );
                    Ok(ir::ModuleName(seg.ident.to_string()))
                })
                .collect::<Result<_>>()?,
            ty: TypeName(last_seg.ident.to_string()),
            targs: match &last_seg.arguments {
                syn::PathArguments::AngleBracketed(args) => (args.args.iter())
                    .filter_map(|arg| match arg {
                        syn::GenericArgument::Type(arg) => Some(self.convert_type(arg)),
                        _ => None,
                    })
                    .collect::<Result<_>>()?,
                _ => vec![],
            },
        })
    }
}

fn field_name(ident: Option<&syn::Ident>, prefix: Option<&str>) -> FieldName {
    FieldName(format!("{}{}", prefix.unwrap_or_default(), ident.unwrap()))
}

fn variant_name(ident: &syn::Ident, prefix: Option<&str>) -> VariantName {
    VariantName(format!("{}{}", prefix.unwrap_or_default(), ident))
}
