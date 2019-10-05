// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//

use quote::quote;
use syn::{Type, TypePath};
use synstructure::decl_derive;

decl_derive!([Ocamlvalue] => derive_ocamlvalue);

fn get_boxed_tuple_len(bi: &synstructure::BindingInfo) -> usize {
    if let Type::Path(TypePath { path, .. }) = &bi.ast().ty {
        if let Some(path_seg) = path.segments.first() {
            if path_seg.ident == "Box" {
                if let syn::PathArguments::AngleBracketed(args) = &path_seg.arguments {
                    if let Some(syn::GenericArgument::Type(Type::Tuple(syn::TypeTuple {
                        elems,
                        ..
                    }))) = args.args.first()
                    {
                        return elems.len();
                    }
                }
            }
        }
    }
    0
}

fn derive_ocamlvalue(mut s: synstructure::Structure) -> proc_macro2::TokenStream {
    s.add_bounds(synstructure::AddBounds::Generics);
    match &s.ast().data {
        syn::Data::Struct(st) => {
            let variant = &s.variants()[0];
            let size = variant.bindings().len();
            match st.fields {
                syn::Fields::Unit => s.gen_impl(quote! {
                    gen impl ocamlpool_rust::ocamlvalue::Ocamlvalue for @Self{
                        fn ocamlvalue(&self) -> ocaml::core::mlvalues::Value {
                            ().ocamlvalue()
                        }
                    }
                }),
                syn::Fields::Unnamed(_) if size == 1 => {
                    s.bind_with(|_| synstructure::BindStyle::Move);
                    s.gen_impl(quote! {
                        gen impl ocamlpool_rust::ocamlvalue::Ocamlvalue for @Self{
                            fn ocamlvalue(&self) -> ocaml::core::mlvalues::Value {
                                self.0.ocamlvalue()
                            }
                        }
                    })
                }
                syn::Fields::Named(_) | syn::Fields::Unnamed(_) => {
                    s.bind_with(|_| synstructure::BindStyle::Move);
                    let mut pattern_acc = quote! {};
                    for v in s.variants().iter() {
                        let p = v.pat();
                        let list = v.bindings().iter().fold(quote! {}, |acc, bi| {
                            quote! {#acc #bi.ocamlvalue() , }
                        });
                        pattern_acc = quote! { #pattern_acc #p => { ocamlpool_rust::utils::caml_tuple( &[ #list ]) } };
                    }
                    s.gen_impl(quote! {
                        gen impl ocamlpool_rust::ocamlvalue::Ocamlvalue for @Self{
                            fn ocamlvalue(&self) -> ocaml::core::mlvalues::Value {
                                match self {
                                 #pattern_acc
                                }
                            }
                        }
                    })
                }
            }
        }
        syn::Data::Enum(_) => {
            s.bind_with(|_| synstructure::BindStyle::Move);
            let mut pattern_acc = quote! {};
            let mut variant_with_param_tag: u8 = 0;
            let mut variant_with_no_param_tag: u8 = 0;
            for v in s.variants().iter() {
                let binding_len = v.bindings().len();
                let p = v.pat();
                if binding_len > 0 {
                    let first_boxed_tuple_len = get_boxed_tuple_len(&v.bindings()[0]);
                    if binding_len == 1 && first_boxed_tuple_len > 0 {
                        let bi = &v.bindings()[0];
                        let mut list = quote! {};
                        for i in 0..first_boxed_tuple_len {
                            let idx = syn::Index::from(i);
                            list = quote! {#list #bi.#idx.ocamlvalue() , }
                        }
                        pattern_acc = quote! {
                            #pattern_acc #p => {
                                ocamlpool_rust::utils::caml_block(
                                    #variant_with_param_tag,
                                    &[ #list ]
                                )
                            }
                        }
                    } else {
                        let list = v.bindings().iter().fold(quote! {}, |acc, bi| {
                            quote! {#acc #bi.ocamlvalue() , }
                        });
                        pattern_acc = quote! {
                            #pattern_acc #p => {
                                ocamlpool_rust::utils::caml_block(
                                    #variant_with_param_tag,
                                    &[ #list ]
                                )
                            }
                        }
                    }
                    variant_with_param_tag += 1;
                } else {
                    pattern_acc = quote! {
                        #pattern_acc #p => {
                            ocamlpool_rust::utils::u8_to_ocaml(
                                #variant_with_no_param_tag
                            )
                        }
                    };
                    variant_with_no_param_tag += 1;
                }
            }
            s.gen_impl(
                quote! {  gen impl ocamlpool_rust::ocamlvalue::Ocamlvalue for @Self{
                    fn ocamlvalue(&self) -> ocaml::core::mlvalues::Value {
                        match self {
                         #pattern_acc
                        }
                    }
                } },
            )
        }
        _ => panic!(),
    }
}
