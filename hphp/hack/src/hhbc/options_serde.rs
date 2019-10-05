// Copyright (c) 2019, Facebook, Inc.
// Copyright (c) 2019 Jonathan Sundqvist
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
// SOFTWARE.

extern crate proc_macro;

use proc_macro::{Ident, Span, TokenStream, TokenTree};
use quote::ToTokens;
use syn::{
    parse_macro_input, parse_quote, Attribute, Data, DataEnum, DataStruct, DeriveInput, LitStr,
};

fn create_attribute(prefix: &str, field_name: &str) -> Attribute {
    let serde_name = format!("{}{}", prefix, field_name);
    parse_quote! {
        #[serde(rename = #serde_name)]
    }
}

fn prefix_all_struct(input: &mut DataStruct, prefix: &str) {
    for field in &mut input.fields {
        let field_name = field.ident.to_token_stream().to_string();
        field.attrs.push(create_attribute(prefix, &field_name));
    }
}

fn prefix_all_enum(input: &mut DataEnum, prefix: &str) {
    for variant in &mut input.variants {
        let variant_name = variant.ident.to_string();
        variant.attrs.push(create_attribute(prefix, &variant_name));
    }
}

#[proc_macro_attribute]
pub fn prefix_all(args: TokenStream, input: TokenStream) -> TokenStream {
    let prefix = parse_macro_input!(args as LitStr).value();
    let mut input = parse_macro_input!(input as DeriveInput);

    match &mut input.data {
        Data::Struct(data) => prefix_all_struct(data, &prefix),
        Data::Enum(data) => prefix_all_enum(data, &prefix),
        Data::Union(_) => panic!("prefix_all does not support unions"),
    }

    input.into_token_stream().into()
}

/// Expands idents in the input stream into concatenation of:
/// - prefix
/// - lowercase string without underscores (unsnakecase)
#[proc_macro]
pub fn field_to_config_name(input: TokenStream) -> TokenStream {
    // let prefix = parse_macro_input!(input as LitStr).value();
    input
        .into_iter()
        .map(|t| match t {
            TokenTree::Ident(ident) => {
                Ident::new(
                    // TODO(leoo) prepend prefix
                    &ident.to_string().to_lowercase(),
                    Span::call_site(),
                )
                .into()
            }
            _ => t,
        })
        .collect()
}
