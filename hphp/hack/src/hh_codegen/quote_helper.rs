// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::{quote, ToTokens};

pub fn join(mut tokens: impl Iterator<Item = impl ToTokens>, sep: impl ToTokens) -> TokenStream {
    let mut acc: Vec<TokenStream> = vec![];
    if let Some(t) = tokens.next() {
        acc.push(quote! {#t});
    }
    for t in tokens {
        acc.push(quote! {#sep #t});
    }
    quote! { #(#acc)* }
}

pub fn with_paren(tokens: impl ToTokens) -> TokenStream {
    quote! {( #tokens )}
}
