// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::{format_ident, quote};

pub fn gen_ty_params(tys: impl Iterator<Item = syn::Ident>) -> TokenStream {
    let ty_idents = tys.map(|ty| quote! { P::#ty, }).collect::<Vec<_>>();
    if ty_idents.is_empty() {
        quote! {}
    } else {
        quote! {<#(#ty_idents)*>}
    }
}

pub fn gen_ty_params_with_self(tys: impl Iterator<Item = syn::Ident>) -> TokenStream {
    let ty_idents = tys
        .map(|ty| quote! { <Self::P as Params>::#ty, })
        .collect::<Vec<_>>();
    if ty_idents.is_empty() {
        quote! {}
    } else {
        quote! {<#(#ty_idents)*>}
    }
}

pub fn gen_module_uses(ms: impl Iterator<Item = impl AsRef<str>>) -> TokenStream {
    let mods = ms.map(|m| format_ident!("{}", m.as_ref()));
    quote! {
        use crate::{#(#mods::*,)*};
    }
}
