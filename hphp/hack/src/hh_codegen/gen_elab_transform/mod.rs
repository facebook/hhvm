// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod pass_generator;
mod transform_generator;

use syn::Token;
use syn::punctuated::Punctuated;

pub use crate::common::args::CommonArgs as Args;
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
        ("pass.rs", pass_generator::r#gen(&ctx)),
        ("transform.rs", transform_generator::r#gen(&ctx)),
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

fn contains_ocaml_attr(attrs: &[syn::Attribute], value: &str) -> bool {
    for attr in attrs {
        if attr.path().is_ident("rust_to_ocaml")
            && let Ok(inner) =
                attr.parse_args_with(Punctuated::<syn::Meta, Token![,]>::parse_terminated)
        {
            for meta in inner {
                if let syn::Meta::NameValue(meta) = meta
                    && meta.path.is_ident("attr")
                    && let syn::Expr::Lit(meta_value) = &meta.value
                    && let syn::Lit::Str(lit) = &meta_value.lit
                {
                    return lit.value() == value;
                }
            }
        }
    }

    false
}
