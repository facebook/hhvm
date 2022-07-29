// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use syn::Meta::List;
use syn::Meta::NameValue;
use syn::NestedMeta::Lit;
use syn::NestedMeta::Meta;

static RUST_TO_OCAML: &str = "rust_to_ocaml";
static PREFIX: &str = "prefix";

#[derive(Clone, Debug)]
pub struct Container {
    pub prefix: Option<String>,
}

impl Container {
    pub fn from_ast(item: &syn::DeriveInput) -> Self {
        let mut prefix = None;

        for meta_item in item
            .attrs
            .iter()
            .flat_map(get_rust_to_ocaml_meta_items)
            .flatten()
        {
            match &meta_item {
                // Parse `#[rust_to_ocaml(prefix = "foo")]`
                Meta(NameValue(m)) if m.path.is_ident(PREFIX) => {
                    if let Ok(s) = get_lit_str(PREFIX, &m.lit) {
                        prefix = Some(s.value());
                    }
                }
                Meta(_meta_item) => {
                    // let path = meta_item
                    //     .path()
                    //     .into_token_stream()
                    //     .to_string()
                    //     .replace(' ', "");
                    // cx.error_spanned_by(
                    //     meta_item.path(),
                    //     format!("unknown rust_to_ocaml container attribute `{}`", path),
                    // );
                }
                Lit(_lit) => {
                    // cx.error_spanned_by(lit, "unexpected literal in rust_to_ocaml container attribute");
                }
            }
        }

        Self { prefix }
    }
}

#[derive(Clone, Debug)]
pub struct Variant {
    pub prefix: Option<String>,
}

impl Variant {
    pub fn from_ast(variant: &syn::Variant) -> Self {
        let mut prefix = None;

        for meta_item in variant
            .attrs
            .iter()
            .flat_map(get_rust_to_ocaml_meta_items)
            .flatten()
        {
            match &meta_item {
                // Parse `#[rust_to_ocaml(prefix = "foo")]`
                Meta(NameValue(m)) if m.path.is_ident(PREFIX) => {
                    if let Ok(s) = get_lit_str(PREFIX, &m.lit) {
                        prefix = Some(s.value());
                    }
                }
                Meta(_meta_item) => {
                    // let path = meta_item
                    //     .path()
                    //     .into_token_stream()
                    //     .to_string()
                    //     .replace(' ', "");
                    // cx.error_spanned_by(
                    //     meta_item.path(),
                    //     format!("unknown rust_to_ocaml variant attribute `{}`", path),
                    // );
                }
                Lit(_lit) => {
                    // cx.error_spanned_by(lit, "unexpected literal in rust_to_ocaml variant attribute");
                }
            }
        }

        Self { prefix }
    }
}

fn get_rust_to_ocaml_meta_items(attr: &syn::Attribute) -> Result<Vec<syn::NestedMeta>, ()> {
    if !attr.path.is_ident(RUST_TO_OCAML) {
        return Ok(vec![]);
    }

    match attr.parse_meta() {
        Ok(List(meta)) => Ok(meta.nested.into_iter().collect()),
        Ok(_other) => {
            // cx.error_spanned_by(other, "expected #[rust_to_ocaml(...)]");
            Err(())
        }
        Err(_err) => {
            // cx.syn_error(err);
            Err(())
        }
    }
}

fn get_lit_str<'a>(_attr_name: &'static str, lit: &'a syn::Lit) -> Result<&'a syn::LitStr, ()> {
    if let syn::Lit::Str(lit) = lit {
        Ok(lit)
    } else {
        // cx.error_spanned_by(
        //     lit,
        //     format!(
        //         "expected rust_to_ocaml {} attribute to be a string: `{} = \"...\"`",
        //         attr_name, attr_name
        //     ),
        // );
        Err(())
    }
}
