// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod pass_generator;
mod transform_generator;

pub use crate::common::args::Args;
use crate::common::context::Context;
use crate::common::to_snake;

pub enum Direction {
    TopDown,
    BottomUp,
}
impl Direction {
    pub fn to_string(&self) -> &str {
        match self {
            Direction::TopDown => "top_down",
            Direction::BottomUp => "bottom_up",
        }
    }
}

pub fn run(args: &Args) -> anyhow::Result<Vec<(std::path::PathBuf, String)>> {
    let files = crate::common::parse_all(&args.input)?;

    let ctx = Context::new(files.as_slice(), &args.root)?;

    let results = vec![
        ("pass.rs", pass_generator::gen(&ctx)),
        ("transform.rs", transform_generator::gen(&ctx)),
    ];
    Ok(results
        .iter()
        .map(|(filename, source)| (args.output.join(filename), source.to_string()))
        .collect())
}

fn gen_pass_method_name(ty: impl AsRef<str>, dir: Direction) -> syn::Ident {
    quote::format_ident!("on_ty_{}_{}", to_snake(ty.as_ref()), dir.to_string())
}

fn gen_pass_fld_method_name(
    ty: impl AsRef<str>,
    field: impl AsRef<str>,
    dir: Direction,
) -> syn::Ident {
    quote::format_ident!(
        "on_fld_{}_{}_{}",
        to_snake(ty.as_ref()),
        to_snake(field.as_ref()),
        dir.to_string(),
    )
}

fn gen_pass_ctor_method_name(
    ty: impl AsRef<str>,
    field: impl AsRef<str>,
    dir: Direction,
) -> syn::Ident {
    quote::format_ident!(
        "on_ctor_{}_{}_{}",
        to_snake(ty.as_ref()),
        to_snake(field.as_ref()),
        dir.to_string()
    )
}

fn contains_ocaml_attr(attrs: &[syn::Attribute], attr: &'static str) -> bool {
    fn get_rust_to_ocaml_meta_items(attr: &syn::Attribute) -> Option<Vec<syn::NestedMeta>> {
        if !attr.path.is_ident("rust_to_ocaml") {
            return None;
        }
        match attr.parse_meta() {
            Ok(syn::Meta::List(meta)) => Some(meta.nested.into_iter().collect()),
            _ => None,
        }
    }
    fn get_lit_str<'a>(_attr_name: &'static str, lit: &'a syn::Lit) -> Option<&'a syn::LitStr> {
        if let syn::Lit::Str(lit) = lit {
            Some(lit)
        } else {
            None
        }
    }
    attrs
        .iter()
        .flat_map(get_rust_to_ocaml_meta_items)
        .flatten()
        .any(|item| {
            use syn::Meta::NameValue;
            use syn::NestedMeta::Meta;
            match item {
                Meta(NameValue(m)) if m.path.is_ident("attr") => {
                    if let Some(s) = get_lit_str("attr", &m.lit) {
                        return s.value() == attr;
                    }
                    false
                }
                _ => false,
            }
        })
}
