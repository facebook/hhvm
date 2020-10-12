// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![recursion_limit = "128"]

use proc_macro2::TokenStream;
use quote::quote;
use synstructure::decl_derive;

decl_derive!([NoPosHash] => derive_no_pos_hash);

fn derive_no_pos_hash(mut s: synstructure::Structure) -> TokenStream {
    // By default, if you are deriving an impl of trait Foo for generic type
    // X<T>, synstructure will add Foo as a bound not only for the type
    // parameter T, but also for every type which appears as a field in X. This
    // is not necessary for our use case--we can just require that the type
    // parameters implement our trait.
    s.add_bounds(synstructure::AddBounds::Generics);

    let body = no_pos_hash_body(&s);

    s.gen_impl(quote! {
        gen impl ::no_pos_hash::NoPosHash for @Self {
            fn hash<__H: ::core::hash::Hasher>(&self, state: &mut __H) {
                match self { #body }
            }
        }
    })
}

fn no_pos_hash_body(s: &synstructure::Structure) -> TokenStream {
    let hash = quote! { ::no_pos_hash::NoPosHash::hash };
    match &s.ast().data {
        syn::Data::Struct(_) => s.each(|bi| quote! { #hash(#bi, state); }),
        syn::Data::Enum(_) => s.each_variant(|v| {
            v.bindings().iter().fold(
                quote! { #hash(&::std::mem::discriminant(self), state); },
                |acc, bi| quote! { #acc #hash(#bi, state); },
            )
        }),
        syn::Data::Union(_) => panic!("untagged unions not supported"),
    }
}
