// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use syn::Meta::List;
use syn::Meta::NameValue;
use syn::Meta::Path;
use syn::NestedMeta::Lit;
use syn::NestedMeta::Meta;

static DOC: &str = "doc";
static RUST_TO_OCAML: &str = "rust_to_ocaml";
static PREFIX: &str = "prefix";
static ATTR: &str = "attr";
static NAME: &str = "name";
static INLINE_TUPLE: &str = "inline_tuple";

#[derive(Clone, Debug)]
pub struct Attrs {
    pub doc: Vec<String>,
    pub prefix: Option<String>,
    pub attrs: Vec<String>,
    pub name: Option<String>,
    pub inline_tuple: bool,
}

impl Attrs {
    pub fn from_type(item: &syn::ItemType) -> Self {
        Self::from_attributes(&item.attrs, AttrKind::Container)
    }
    pub fn from_struct(item: &syn::ItemStruct) -> Self {
        Self::from_attributes(&item.attrs, AttrKind::Container)
    }
    pub fn from_enum(item: &syn::ItemEnum) -> Self {
        Self::from_attributes(&item.attrs, AttrKind::Container)
    }

    pub fn from_variant(variant: &syn::Variant) -> Self {
        Self::from_attributes(&variant.attrs, AttrKind::Variant)
    }
    pub fn from_field(field: &syn::Field) -> Self {
        Self::from_attributes(&field.attrs, AttrKind::Field)
    }

    fn from_attributes(attrs: &[syn::Attribute], kind: AttrKind) -> Self {
        let doc = get_doc_comment(attrs);
        let mut prefix = None;
        let mut ocaml_attrs = vec![];
        let mut name = None;
        let mut inline_tuple = false;

        for meta_item in attrs
            .iter()
            .flat_map(get_rust_to_ocaml_meta_items)
            .flatten()
        {
            match &meta_item {
                // Parse `#[rust_to_ocaml(prefix = "foo")]`
                Meta(NameValue(m)) if m.path.is_ident(PREFIX) => {
                    // TODO: emit error for AttrKind::Field (should use the
                    // `name` meta item instead)
                    if let Ok(s) = get_lit_str(PREFIX, &m.lit) {
                        prefix = Some(s.value());
                    }
                }
                // Parse `#[rust_to_ocaml(attr = "deriving eq")]`
                Meta(NameValue(m)) if m.path.is_ident(ATTR) => {
                    if let Ok(s) = get_lit_str(ATTR, &m.lit) {
                        ocaml_attrs.push(s.value());
                    }
                }
                // Parse `#[rust_to_ocaml(name = "foo")]`
                Meta(NameValue(m)) if m.path.is_ident(NAME) => {
                    // TODO: emit error for AttrKind::Container (should add to
                    // types.rename config instead)
                    if let Ok(s) = get_lit_str(NAME, &m.lit) {
                        name = Some(s.value());
                    }
                }
                // Parse `#[rust_to_ocaml(inline_tuple)]`
                Meta(Path(word)) if word.is_ident(INLINE_TUPLE) => {
                    // TODO: emit an error instead
                    assert_eq!(kind, AttrKind::Variant);
                    inline_tuple = true;
                }
                Meta(_meta_item) => {
                    // let path = meta_item
                    //     .path()
                    //     .into_token_stream()
                    //     .to_string()
                    //     .replace(' ', "");
                    // cx.error_spanned_by(
                    //     meta_item.path(),
                    //     format!("unknown rust_to_ocaml {} attribute `{}`", kind, path),
                    // );
                }
                Lit(_lit) => {
                    // cx.error_spanned_by(lit, format!("unexpected literal in rust_to_ocaml {} attribute", kind));
                }
            }
        }

        Self {
            doc,
            prefix,
            attrs: ocaml_attrs,
            name,
            inline_tuple,
        }
    }
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
enum AttrKind {
    Container,
    Variant,
    Field,
}

impl std::fmt::Display for AttrKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Container => write!(f, "container"),
            Self::Variant => write!(f, "variant"),
            Self::Field => write!(f, "field"),
        }
    }
}

pub fn get_doc_comment(attrs: &[syn::Attribute]) -> Vec<String> {
    attrs
        .iter()
        .filter_map(|attr| {
            if !attr.path.is_ident(DOC) {
                return None;
            }
            match attr.parse_meta() {
                Ok(syn::Meta::NameValue(meta)) => {
                    if let syn::Lit::Str(s) = meta.lit {
                        Some(s.value())
                    } else {
                        None
                    }
                }
                _ => None,
            }
        })
        .collect()
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
