// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::{quote, ToTokens};
use syn::*;

pub enum RefKind {
    Ref,
    RefMut,
    Owned,
}

impl RefKind {
    pub fn mk_value(
        &self,
        var: &impl ToTokens,
        is_box: bool,
        tuple_accessor: Option<usize>,
    ) -> TokenStream {
        let accessor = tuple_accessor.map(Index::from);
        match (self, is_box, accessor) {
            (_, false, _) => quote! { #var },

            (RefKind::Ref, true, None) => quote! { &#var },
            (RefKind::Ref, true, Some(i)) => quote! { &#var.#i },

            (RefKind::RefMut, true, None) => quote! { #var.as_mut() },
            (RefKind::RefMut, true, Some(i)) => quote! { &mut #var.#i },

            (RefKind::Owned, true, None) => quote! { *#var },
            (RefKind::Owned, true, Some(i)) => quote! { (*#var).#i },
        }
    }

    pub fn mk_ty(&self, ty: &impl ToTokens) -> proc_macro2::TokenStream {
        match self {
            RefKind::Ref => quote! { &#ty },
            RefKind::RefMut => quote! { &mut #ty },
            RefKind::Owned => quote! { #ty },
        }
    }
}
