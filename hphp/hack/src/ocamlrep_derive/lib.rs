// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![recursion_limit = "128"]

use proc_macro2::TokenStream;
use quote::quote;
use synstructure::{decl_derive, BindingInfo, VariantInfo};

decl_derive!([OcamlRep] => derive_ocamlrep);

fn derive_ocamlrep(mut s: synstructure::Structure) -> TokenStream {
    // By default, if you are deriving an impl of trait Foo for generic type
    // X<T>, synstructure will add Foo as a bound not only for the type
    // parameter T, but also for every type which appears as a field in X. This
    // is not necessary for our use case--we can just require that the type
    // parameters implement our trait.
    s.add_bounds(synstructure::AddBounds::Generics);

    let (to_body, from_body) = match &s.ast().data {
        syn::Data::Struct(struct_data) => struct_impl(&s, struct_data),
        syn::Data::Enum(_) => enum_impl(&s),
        syn::Data::Union(_) => panic!("untagged unions not supported"),
    };

    s.gen_impl(quote! {
        use ::ocamlrep::{Allocator, OcamlRep};
        gen impl ::ocamlrep::OcamlRep for @Self {
            fn to_ocamlrep<'a, Alloc: ::ocamlrep::Allocator>(&self, arena: &'a Alloc) -> ::ocamlrep::Value<'a> {
                match self { #to_body }
            }

            fn from_ocamlrep(value: ::ocamlrep::Value<'_>) -> ::std::result::Result<Self, ::ocamlrep::FromError> {
                #from_body
            }
        }
    })
}

fn struct_impl(
    s: &synstructure::Structure,
    struct_data: &syn::DataStruct,
) -> (TokenStream, TokenStream) {
    let variant = &s.variants()[0];
    match struct_data.fields {
        syn::Fields::Unit => {
            // Represent unit structs with unit.
            let to = s.each_variant(|_| quote! { arena.add(&()) });
            let constructor = variant.construct(|_, _| quote!(unreachable!()));
            let from = quote! { <()>::from_ocamlrep(value)?; Ok(#constructor) };
            (to, from)
        }
        syn::Fields::Unnamed(ref fields) if fields.unnamed.len() == 1 => {
            // For the newtype pattern (a tuple struct with a single field),
            // don't allocate a block--just use the inner value directly.
            let to = s.each(|bi| quote! { arena.add(#bi) });
            let constructor = variant.construct(|field, _| {
                let ty = &field.ty;
                quote! { <#ty>::from_ocamlrep(value)? }
            });
            let from = quote! { Ok(#constructor) };
            (to, from)
        }
        syn::Fields::Named(_) | syn::Fields::Unnamed(_) => {
            // Otherwise, we have a record-like struct or a tuple struct. Both
            // are represented with a block.
            let to = s.each_variant(|v| allocate_block(v, 0));
            let size = variant.bindings().len();
            let constructor =
                variant.construct(|_, i| quote! { ::ocamlrep::from::field(block, #i)? });
            let from = quote! {
                let mut block = ::ocamlrep::from::expect_tuple(value, #size)?;
                Ok(#constructor)
            };
            (to, from)
        }
    }
}

fn enum_impl(s: &synstructure::Structure) -> (TokenStream, TokenStream) {
    // For tagging purposes, variant constructors of zero arguments are numbered
    // separately from variant constructors of one or more arguments, so we need
    // to count them separately to learn their tags.
    let mut nullary_variants = vec![];
    let mut block_variants = vec![];
    for variant in s.variants().iter() {
        if variant.bindings().len() == 0 {
            nullary_variants.push((variant, nullary_variants.len() as isize));
        } else {
            block_variants.push((variant, block_variants.len() as isize));
        };
    }
    // Block tags larger than this value indicate specific OCaml types (and tags
    // larger than 255 wouldn't fit in a u8 anyway).
    // See https://github.com/ocaml/ocaml/blob/3.08/utils/config.mlp#L55
    assert!(
        block_variants.len() <= 246,
        "Too many non-constant enum variants -- maximum is 246"
    );

    let mut all_variants = nullary_variants.clone();
    all_variants.extend(block_variants.iter().cloned());

    let to = enum_to_ocamlrep(s, all_variants);
    let from = enum_from_ocamlrep(nullary_variants, block_variants);
    (to, from)
}

fn enum_to_ocamlrep(
    s: &synstructure::Structure,
    all_variants: Vec<(&synstructure::VariantInfo<'_>, isize)>,
) -> TokenStream {
    s.each_variant(|v| {
        let size = v.bindings().len();
        let tag = {
            all_variants
                .iter()
                .find(|(var, _)| *var == v)
                .map(|(_, tag)| *tag)
                .unwrap()
        };
        if size == 0 {
            quote!(::ocamlrep::Value::int(#tag))
        } else {
            let tag = tag as u8;
            match get_boxed_tuple_len(v) {
                None => allocate_block(v, tag),
                Some(len) => boxed_tuple_variant_to_block(&v.bindings()[0], tag, len),
            }
        }
    })
}

fn enum_from_ocamlrep(
    nullary_variants: Vec<(&synstructure::VariantInfo<'_>, isize)>,
    block_variants: Vec<(&synstructure::VariantInfo<'_>, isize)>,
) -> TokenStream {
    let max_nullary_tag = nullary_variants.len().saturating_sub(1);
    let max_block_tag = block_variants.len().saturating_sub(1) as u8;

    let mut nullary_arms = TokenStream::new();
    for (variant, tag) in nullary_variants.iter() {
        let constructor = variant.construct(|_, _| quote!(unreachable!()));
        nullary_arms.extend(quote! { #tag => Ok(#constructor), });
    }
    nullary_arms.extend(quote! {
        tag => Err(::ocamlrep::FromError::NullaryVariantTagOutOfRange {
            max: #max_nullary_tag,
            actual: tag,
        })
    });

    let mut block_arms = TokenStream::new();
    for (variant, tag) in block_variants.iter() {
        let tag = *tag as u8;
        let size = variant.bindings().len();
        let constructor = match get_boxed_tuple_len(variant) {
            None => variant.construct(|_, i| quote! { ::ocamlrep::from::field(block, #i)? }),
            Some(len) => boxed_tuple_variant_constructor(variant, len),
        };
        block_arms.extend(quote! { #tag => {
            ::ocamlrep::from::expect_block_size(block, #size)?;
            Ok(#constructor)
        } });
    }
    block_arms.extend(quote! {
        tag => Err(::ocamlrep::FromError::BlockTagOutOfRange {
            max: #max_block_tag,
            actual: tag,
        })
    });

    match (nullary_variants.is_empty(), block_variants.is_empty()) {
        // An enum with no variants is not instantiable.
        (true, true) => panic!("cannot derive OcamlRep for non-instantiable enum"),
        // Nullary variants only.
        (false, true) => quote! {
            match ::ocamlrep::from::expect_int(value)? { #nullary_arms }
        },
        // Block variants only.
        (true, false) => quote! {
            let block = ::ocamlrep::from::expect_block(value)?;
            match block.tag() { #block_arms }
        },
        // Both nullary and block variants.
        (false, false) => quote! {
            if value.is_immediate() {
                match value.as_int().unwrap() { #nullary_arms }
            } else {
                let block = value.as_block().unwrap();
                match block.tag() { #block_arms }
            }
        },
    }
}

fn allocate_block(variant: &VariantInfo, tag: u8) -> TokenStream {
    let size = variant.bindings().len();
    let mut fields = TokenStream::new();
    for (i, bi) in variant.bindings().iter().enumerate() {
        fields.extend(quote! {
            Alloc::set_field(&mut block, #i, arena.add(#bi));
        });
    }
    quote! {
        let mut block = arena.block_with_size_and_tag(#size, #tag);
        #fields
        block.build()
    }
}

fn boxed_tuple_variant_to_block(bi: &BindingInfo, tag: u8, len: usize) -> TokenStream {
    let mut fields = TokenStream::new();
    for i in 0..len {
        let idx = syn::Index::from(i);
        fields.extend(quote! {
            Alloc::set_field(&mut block, #i, arena.add(&#bi.#idx));
        });
    }
    quote! {
        let mut block = arena.block_with_size_and_tag(#len, #tag);
        #fields
        block.build()
    }
}

fn boxed_tuple_variant_constructor(variant: &VariantInfo, len: usize) -> TokenStream {
    let mut ident = TokenStream::new();
    if let Some(prefix) = variant.prefix {
        ident.extend(quote!(#prefix ::));
    }
    let id = variant.ast().ident;
    ident.extend(quote!(#id));

    let mut fields = TokenStream::new();
    for i in 0..len {
        let idx = syn::Index::from(i);
        fields.extend(quote! { ::ocamlrep::from::field(block, #idx)?, });
    }
    quote! { #ident(::std::boxed::Box::new((#fields))) }
}

fn get_boxed_tuple_len(variant: &VariantInfo) -> Option<usize> {
    use syn::{Fields, GenericArgument, PathArguments, Type, TypePath};

    match &variant.ast().fields {
        Fields::Unnamed(_) => (),
        _ => return None,
    }
    let bi = match variant.bindings() {
        [bi] => bi,
        _ => return None,
    };
    let path = match &bi.ast().ty {
        Type::Path(TypePath { path, .. }) => path,
        _ => return None,
    };
    let path_seg = match path.segments.first() {
        Some(s) if s.ident == "Box" => s,
        _ => return None,
    };
    let args = match &path_seg.arguments {
        PathArguments::AngleBracketed(args) => args,
        _ => return None,
    };
    let tuple = match args.args.first() {
        Some(GenericArgument::Type(Type::Tuple(tuple))) => tuple,
        _ => return None,
    };

    Some(tuple.elems.len())
}
