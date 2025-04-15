// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::TokenStream;
use quote::quote;
use synstructure::Structure;
use synstructure::decl_derive;

decl_derive!([EqModuloPos] => derive_eq_modulo_pos);

fn workaround_non_local_def(impl_block: TokenStream) -> TokenStream {
    // We need to upgrade synstructure to remove this warning, but doing so will also require upgrading
    // to syn2 and rewriting to handle the API changes.
    quote! {
        #[allow(non_local_definitions)]
        #impl_block
    }
}

fn derive_eq_modulo_pos(mut s: Structure<'_>) -> TokenStream {
    // By default, if you are deriving an impl of trait Foo for generic type
    // X<T>, synstructure will add Foo as a bound not only for the type
    // parameter T, but also for every type which appears as a field in X. This
    // is not necessary for our use case--we can just require that the type
    // parameters implement our trait.
    s.add_bounds(synstructure::AddBounds::Generics);

    let eq_modulo_pos = derive_eq_modulo_pos_body(&s);
    let eq_modulo_pos_and_reason = derive_eq_modulo_pos_and_reason_body(&s);
    workaround_non_local_def(s.gen_impl(quote! {
        gen impl EqModuloPos for @Self {
            fn eq_modulo_pos(&self, rhs: &Self) -> bool {
                match self { #eq_modulo_pos }
            }
            fn eq_modulo_pos_and_reason(&self, rhs: &Self) -> bool {
                match self { #eq_modulo_pos_and_reason }
            }
        }
    }))
}

fn derive_eq_modulo_pos_body(s: &Structure<'_>) -> TokenStream {
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

fn derive_eq_modulo_pos_and_reason_body(s: &Structure<'_>) -> TokenStream {
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
            inner = quote! { #inner && #bi.eq_modulo_pos_and_reason(#bi_rhs) }
        }
        quote!(
            match rhs {
                #arm => { #inner }
                _ => false,
            }
        )
    })
}
