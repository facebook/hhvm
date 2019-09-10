// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![recursion_limit = "128"]

use proc_macro2::TokenStream;
use quote::quote;
use synstructure::decl_derive;

decl_derive!([OcamlRep] => derive_ocamlrep);

fn derive_ocamlrep(mut s: synstructure::Structure) -> TokenStream {
    // By default, if you are deriving an impl of trait Foo for generic type
    // X<T>, synstructure will add Foo as a bound not only for the type
    // parameter T, but also for every type which appears as a field in X. This
    // is not necessary for our use case--we can just require that the type
    // parameters implement our trait.
    s.add_bounds(synstructure::AddBounds::Generics);

    // Generate pattern bindings for match arms such that the fields are moved
    // out of the scrutinee. This is what we want for an "into" method (which
    // consumes `self`)--we want to move the fields into local variables (so
    // that we can then convert them into Values, etc).
    s.bind_with(|_| synstructure::BindStyle::Move);

    let body = match &s.ast().data {
        syn::Data::Struct(struct_data) => struct_impl(&s, struct_data),
        syn::Data::Enum(_) => enum_impl(&s),
        syn::Data::Union(_) => panic!("untagged unions not supported"),
    };

    s.gen_impl(quote! {
        gen impl ::ocamlrep::OcamlRep for @Self {
            fn into_ocamlrep<'a>(self, arena: &::ocamlrep::Arena<'a>) -> ::ocamlrep::Value<'a> {
                match self { #body }
            }
        }
    })
}

fn struct_impl(s: &synstructure::Structure, struct_data: &syn::DataStruct) -> TokenStream {
    match struct_data.fields {
        syn::Fields::Unit => {
            // Represent unit structs with unit.
            s.each_variant(|_| quote! { arena.add(()) })
        }
        syn::Fields::Unnamed(ref fields) if fields.unnamed.len() == 1 => {
            // For the newtype pattern (a tuple struct with a single field),
            // don't allocate a block--just use the inner value directly.
            s.each(|bi| quote! { arena.add(#bi) })
        }
        syn::Fields::Named(_) | syn::Fields::Unnamed(_) => {
            // Otherwise, we have a record-like struct or a tuple struct. Both
            // are represented with a block.
            s.each_variant(|v| allocate_block(v, 0))
        }
    }
}

fn enum_impl(s: &synstructure::Structure) -> TokenStream {
    // For tagging purposes, variant constructors of zero arguments are numbered
    // separately from variant constructors of one or more arguments, so we need
    // to count them separately to learn their tags.
    let mut nullary_variants = vec![];
    let mut block_variants = vec![];
    for variant in s.variants().iter() {
        let ident = variant.ast().ident.to_string();
        if variant.bindings().len() == 0 {
            nullary_variants.push((ident, nullary_variants.len() as u8));
        } else {
            block_variants.push((ident, block_variants.len() as u8));
        };
    }
    nullary_variants.append(&mut block_variants);
    let all_variants = nullary_variants;

    s.each_variant(|v| {
        let size = v.bindings().len();
        let tag = {
            let ident = v.ast().ident.to_string();
            all_variants
                .iter()
                .find(|(id, _)| *id == ident)
                .map(|(_, tag)| *tag)
                .unwrap()
        };
        if size == 0 {
            quote!(::ocamlrep::Value::int(#tag as isize))
        } else {
            allocate_block(v, tag)
        }
    })
}

fn allocate_block(variant: &synstructure::VariantInfo, tag: u8) -> TokenStream {
    let size = variant.bindings().len();
    let mut fields = TokenStream::new();
    for (i, bi) in variant.bindings().iter().enumerate() {
        fields.extend(quote! { block[#i] = arena.add(#bi); });
    }
    quote! {
        let mut block = arena.block_with_size_and_tag(#size, #tag);
        #fields
        block.build()
    }
}
