// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::quote;
use synstructure::{decl_derive, Structure};

decl_derive!([EqModuloPos] => derive_eq_modulo_pos);

fn derive_eq_modulo_pos(s: Structure) -> TokenStream {
    let body = derive_body(&s);
    s.gen_impl(quote! {
        gen impl EqModuloPos for @Self {
            fn eq_modulo_pos(&self, rhs: &Self) -> bool {
                match self { #body }
            }
        }
    })
}

fn derive_body(s: &Structure) -> TokenStream {
    s.each_variant(|v| {
        let mut s_rhs = s.clone();
        let v_rhs = s_rhs
            .variants_mut()
            .iter_mut()
            .find(|v2| v2.ast().ident == v.ast().ident)
            .unwrap();
        for (i, binding) in v_rhs.bindings_mut().iter_mut().enumerate() {
            let name = format!("rhs{}", i);
            binding.binding = proc_macro2::Ident::new(&name, binding.binding.span());
        }
        let arm = v_rhs.pat();
        let mut inner = quote! {true};
        for (bi, bi_rhs) in v.bindings().iter().zip(v_rhs.bindings().iter()) {
            inner = quote! { #inner && #bi.eq_modulo_pos(#bi_rhs) }
        }
        quote!(
            match rhs {
                #arm => { #inner }
                _ => false,
            }
        )
    })
}
