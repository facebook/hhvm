// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::quote;

pub use crate::common::args::CommonArgs as Args;

pub fn run(args: &Args) -> anyhow::Result<Vec<(std::path::PathBuf, String)>> {
    let files = crate::common::parse_all(&args.input)?;

    let defs: Vec<(syn::Ident, syn::Item)> = files
        .into_iter()
        .flat_map(|(filename, items)| {
            // assuming file name is the module name
            let mod_name = filename.file_stem().and_then(|stem| stem.to_str()).unwrap();
            let mod_name = quote::format_ident!("{}", mod_name);
            items.into_iter().map(move |item| (mod_name.clone(), item))
        })
        .collect();

    let results = vec![("ast.rs", gen_ast(&defs)), ("nast.rs", gen_ast(&defs))];
    Ok(results
        .iter()
        .map(|(filename, source)| (args.output.join(filename), source.to_string()))
        .collect())
}

// Could be written as a function if we wrote a trait to access `ident` and
// `generics` on `syn::Item{Struct,Enum,Type}`. Duck typing is easier.
macro_rules! alias {
    ($module:ident, $ast:ident) => {
        if $ast.generics.params.is_empty() {
            None
        } else {
            let ty = &$ast.ident;
            let (_, ty_generics, _) = $ast.generics.split_for_impl();
            Some(quote!(pub type #ty = #$module :: #ty #ty_generics;))
        }
    }
}

fn gen_ast(defs: &[(syn::Ident, syn::Item)]) -> TokenStream {
    let types = defs.iter().filter_map(|(module, item)| match item {
        syn::Item::Struct(item) => alias!(module, item),
        syn::Item::Enum(item) => alias!(module, item),
        syn::Item::Type(item) => alias!(module, item),
        _ => None,
    });
    quote! {
        use crate::{ast_defs, aast_defs};
        pub use ast_defs::*;
        pub use aast_defs::*;

        /// Expressions have no type annotation.
        type Ex = ();

        /// Toplevel definitions and methods have no "environment" annotation.
        type En = ();

        #(#types)*
    }
}
