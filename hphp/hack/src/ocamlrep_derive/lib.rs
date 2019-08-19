// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![recursion_limit = "128"]

use quote::quote;
use synstructure::decl_derive;

decl_derive!([IntoOcamlRep] => derive_ocamlrep);

fn derive_ocamlrep(mut s: synstructure::Structure) -> proc_macro2::TokenStream {
    s.add_bounds(synstructure::AddBounds::Generics);

    let num_fields = s.fold(0usize, |acc, _| quote! { #acc + 1 });

    match &s.ast().data {
        syn::Data::Struct(struct_data) => {
            s.bind_with(|_| synstructure::BindStyle::Move);

            let variant = &s.variants()[0];
            let size = variant.bindings().len();

            match struct_data.fields {
                syn::Fields::Unit => s.gen_impl(quote! {
                    gen impl ::ocamlrep::IntoOcamlRep for @Self {
                        fn into_ocamlrep<'a>(self, arena: &::ocamlrep::Arena<'a>) -> ::ocamlrep::Value<'a> {
                            ().into_ocamlrep(arena)
                        }
                    }
                }),
                // For the newtype pattern (a tuple struct with a single field),
                // don't allocate a block--just use the inner value directly.
                syn::Fields::Unnamed(_) if size == 1 => {
                    let body = s.each(|bi| quote! { #bi.into_ocamlrep(arena) });
                    s.gen_impl(quote! {
                        gen impl ::ocamlrep::IntoOcamlRep for @Self {
                            fn into_ocamlrep<'a>(self, arena: &::ocamlrep::Arena<'a>) -> ::ocamlrep::Value<'a> {
                                match self { #body }
                            }
                        }
                    })
                }
                syn::Fields::Named(_) | syn::Fields::Unnamed(_) => {
                    let body = s.each(|bi| {
                        quote! {
                            block[idx] = #bi.into_ocamlrep(arena);
                            idx += 1;
                        }
                    });
                    s.gen_impl(quote! {
                        gen impl ::ocamlrep::IntoOcamlRep for @Self {
                            fn into_ocamlrep<'a>(self, arena: &::ocamlrep::Arena<'a>) -> ::ocamlrep::Value<'a> {
                                let mut block = arena.block_with_size(#size);
                                let mut idx = 0;
                                match self { #body }
                                block.build()
                            }
                        }
                    })
                }
            }
        }
        syn::Data::Enum(_) => {
            let mut nullary_variants = vec![];
            let mut block_variants = vec![];
            for variant in s.variants().iter() {
                let ident = variant.ast().ident.to_string();
                let num_bindings = variant.bindings().len();
                if num_bindings == 0 {
                    nullary_variants.push((ident, num_bindings, nullary_variants.len() as u8));
                } else {
                    block_variants.push((ident, num_bindings, block_variants.len() as u8));
                };
            }
            nullary_variants.append(&mut block_variants);
            let all_variants = nullary_variants;

            let tags = s.each_variant(|v| {
                let ident = v.ast().ident.to_string();
                let (_, _, tag) = all_variants.iter().find(|(id, _, _)| *id == ident).unwrap();
                quote! { #tag }
            });

            s.bind_with(|_| synstructure::BindStyle::Move);

            let body = s.each(|bi| {
                quote! {
                    block[idx] = #bi.into_ocamlrep(arena);
                    idx += 1;
                }
            });

            s.gen_impl(quote! {
                gen impl ::ocamlrep::IntoOcamlRep for @Self {
                    fn into_ocamlrep<'a>(self, arena: &::ocamlrep::Arena<'a>) -> ::ocamlrep::Value<'a> {
                        let size = match self { #num_fields };
                        let tag = match self { #tags };
                        if size == 0 {
                            ::ocamlrep::Value::int(tag as isize)
                        } else {
                            let mut block = arena.block_with_size_and_tag(size, tag);
                            let mut idx = 0;
                            match self { #body }
                            block.build()
                        }
                    }
                }
            })
        }
        syn::Data::Union(_) => panic!(),
    }
}
