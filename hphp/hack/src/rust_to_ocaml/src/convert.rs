// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::ir;
use crate::ir::Def;
use crate::ir::File;
use anyhow::bail;
use anyhow::ensure;
use anyhow::Context;
use anyhow::Result;
use convert_case::Case;
use convert_case::Casing;

pub fn convert_file(file: &syn::File) -> Result<String> {
    let defs = (file.items.iter())
        .filter_map(|item| convert_item(item).transpose())
        .collect::<Result<_>>()?;
    let file = File { defs };
    Ok(file.to_string())
}

fn convert_item(item: &syn::Item) -> Result<Option<(String, Def)>> {
    use syn::Item;
    match item {
        Item::Type(item) => {
            Ok(Some(convert_item_type(item).with_context(|| {
                format!("Failed to convert type {}", item.ident)
            })?))
        }
        Item::Struct(item) => {
            Ok(Some(convert_item_struct(item).with_context(|| {
                format!("Failed to convert type {}", item.ident)
            })?))
        }
        Item::Enum(item) => {
            Ok(Some(convert_item_enum(item).with_context(|| {
                format!("Failed to convert type {}", item.ident)
            })?))
        }
        _ => Ok(None),
    }
}

fn convert_item_type(item: &syn::ItemType) -> Result<(String, Def)> {
    let name = item.ident.to_string().to_case(Case::Snake);
    let ty = convert_type(&item.ty)?;
    Ok((name, Def::Alias { ty }))
}

fn convert_item_struct(item: &syn::ItemStruct) -> Result<(String, Def)> {
    let name = item.ident.to_string().to_case(Case::Snake);
    let attrs = attr_parser::Container::from_ast(&item.to_owned().into());
    match &item.fields {
        syn::Fields::Named(fields) => {
            let fields = (fields.named.iter())
                .map(|field| {
                    let mut name = field.ident.as_ref().unwrap().to_string();
                    if let Some(prefix) = attrs.prefix.as_deref() {
                        name = format!("{}{}", prefix, name);
                    };
                    let ty = convert_type(&field.ty)?;
                    Ok((name, ty))
                })
                .collect::<Result<Vec<_>>>()?;
            Ok((name, Def::Record { fields }))
        }
        _ => todo!(),
    }
}

fn convert_item_enum(item: &syn::ItemEnum) -> Result<(String, Def)> {
    let name = item.ident.to_string().to_case(Case::Snake);
    Ok((name, Def::Type))
}

fn convert_type(ty: &syn::Type) -> Result<ir::Type> {
    match ty {
        syn::Type::Path(ty) => Ok(ir::Type::Path(convert_type_path(ty)?)),
        syn::Type::Tuple(ty) => Ok(ir::Type::Tuple(ir::TypeTuple {
            elems: ty.elems.iter().map(convert_type).collect::<Result<_>>()?,
        })),
        _ => bail!("Not supported: {:?}", ty),
    }
}

fn convert_type_path(ty: &syn::TypePath) -> Result<ir::TypePath> {
    ensure!(ty.qself.is_none(), "Qualified self in paths not supported");
    let last_seg = ty.path.segments.last().unwrap();
    if ty.path.segments.len() == 1 && last_seg.arguments.is_empty() {
        // The impls of ToOcamlRep and FromOcamlRep for integer types do checked
        // conversions, so we'll fail at runtime if our int value doesn't fit
        // into OCaml's integer width.
        match last_seg.ident.to_string().as_str() {
            "i8" | "u8" | "i16" | "u16" | "i32" | "u32" | "i64" | "u64" | "i128" | "u128"
            | "isize" | "usize" => return Ok(ir::TypePath::simple("int")),
            _ => {}
        }
    }
    Ok(ir::TypePath {
        idents: ty
            .path
            .segments
            .iter()
            .map(|seg| {
                ensure!(seg.arguments.is_empty(), "Type args in paths not supported");
                let ident = seg.ident.to_string();
                if std::ptr::eq(seg, last_seg) {
                    Ok(ident.to_case(Case::Snake))
                } else {
                    Ok(ident.to_case(Case::UpperCamel))
                }
            })
            .collect::<Result<Vec<_>>>()?,
    })
}
