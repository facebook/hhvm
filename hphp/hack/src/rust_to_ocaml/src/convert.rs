// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::bail;
use anyhow::ensure;
use anyhow::Context;
use anyhow::Result;
use convert_case::Case;
use convert_case::Casing;

pub fn convert_item(item: &syn::Item) -> Result<Option<String>> {
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

fn convert_item_type(item: &syn::ItemType) -> Result<String> {
    let name = item.ident.to_string().to_case(Case::Snake);
    let ty = convert_type(&item.ty)?;
    Ok(format!("type {} = {}\n", name, ty))
}

fn convert_item_struct(item: &syn::ItemStruct) -> Result<String> {
    let name = item.ident.to_string().to_case(Case::Snake);
    Ok(format!("type {}\n", name))
}

fn convert_item_enum(item: &syn::ItemEnum) -> Result<String> {
    let name = item.ident.to_string().to_case(Case::Snake);
    Ok(format!("type {}\n", name))
}

fn convert_type(ty: &syn::Type) -> Result<String> {
    use syn::Type;
    match ty {
        Type::Path(ty) => convert_type_path(ty),
        _ => bail!("Not supported: {:?}", ty),
    }
}

fn convert_type_path(ty: &syn::TypePath) -> Result<String> {
    ensure!(ty.qself.is_none(), "Qualified self in paths not supported");
    let last_seg = ty.path.segments.last().unwrap();
    Ok((ty.path.segments.iter())
        .map(|seg| {
            ensure!(seg.arguments.is_empty(), "Type args in paths not supported");
            let ident = seg.ident.to_string();
            if std::ptr::eq(seg, last_seg) {
                Ok(ident.to_case(Case::Snake))
            } else {
                Ok(ident.to_case(Case::UpperCamel))
            }
        })
        .collect::<Result<Vec<_>>>()?
        .join("."))
}
