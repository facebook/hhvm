// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(clippy::todo)]

use std::collections::HashSet;

use hhbc::ImmType;
use hhbc::InstrFlags;
use hhbc::OpcodeData;
use proc_macro2::Ident;
use proc_macro2::Span;
use proc_macro2::TokenStream;
use quote::quote;
use quote::ToTokens;
use syn::spanned::Spanned;
use syn::Attribute;
use syn::Data;
use syn::DataEnum;
use syn::DataStruct;
use syn::DeriveInput;
use syn::Error;
use syn::Lit;
use syn::LitByteStr;
use syn::LitStr;
use syn::Meta;
use syn::MetaList;
use syn::MetaNameValue;
use syn::NestedMeta;
use syn::Path;
use syn::Result;
use syn::Variant;

/// Builds a HasOperands impl.
///
/// The build rules are as follows:
/// //- For a struct it just looks for a field with a type of LocId.
/// //- For an enum it does a match on each variant.
/// //  - For either tuple variants or struct variants it looks for a field with a
/// //    type of LocId.
/// //  - For a tuple variant with a single non-LocId type and calls `.loc_id()`
/// //    on that field.
/// //  - Otherwise you can specify `#[has_loc(n)]` where `n` is the index of the
/// //    field to call `.loc_id()` on.  `#[has_loc(n)]` can also be used on the
/// //    whole enum to provide a default index.
pub(crate) fn build_has_operands(input: TokenStream) -> Result<TokenStream> {
    let input = syn::parse2::<DeriveInput>(input)?;
    match &input.data {
        Data::Enum(data) => build_has_operands_enum(&input, data),
        Data::Struct(data) => build_has_operands_struct(&input, data),
        Data::Union(_) => Err(Error::new(input.span(), "Union not handled")),
    }
}

fn type_is_value_id(ty: &syn::Type) -> bool {
    match ty {
        syn::Type::Path(ty) => ty.path.is_ident("ValueId"),
        syn::Type::Reference(reference) => type_is_value_id(&reference.elem),
        _ => false,
    }
}

fn build_has_operands_struct(input: &DeriveInput, data: &DataStruct) -> Result<TokenStream> {
    todo!("build_has_operands_struct");
    /*
        // struct Foo {
        //   ...
        //   loc: LocId,
        // }
        let struct_name = &input.ident;

        let field = data
            .fields
            .iter()
            .enumerate()
            .find(|(_, field)| type_is_loc_id(&field.ty));

        let (idx, field) = if let Some((idx, field)) = field {
            (idx, field)
        } else {
            return Err(Error::new(input.span(), "No field with type LocId found"));
        };

        let loc_field = if let Some(ident) = field.ident.as_ref() {
            ident.to_token_stream()
        } else {
            syn::Index::from(idx).to_token_stream()
        };

        let (impl_generics, ty_generics, where_clause) = input.generics.split_for_impl();

        let output = quote!(impl #impl_generics HasLoc for #struct_name #ty_generics #where_clause {
            fn loc_id(&self) -> LocId {
                self.#loc_field
            }
        });

        Ok(output)
    */
}

fn build_has_operands_enum(input: &DeriveInput, data: &DataEnum) -> Result<TokenStream> {
    // enum Foo {
    //   Bar(.., LocId),
    //   Baz { .., loc: LocId },
    // }

    let default_select_field = handle_has_operands_attr(&input.attrs)?;

    let enum_name = &input.ident;
    let mut variants: Vec<TokenStream> = Vec::new();

    for variant in data.variants.iter() {
        let variant_name = &variant.ident;

        let mut select_field = handle_has_operands_attr(&variant.attrs)?;

        if select_field.is_none() {
            for (idx, field) in variant.fields.iter().enumerate() {
                if type_is_value_id(&field.ty) {
                    let kind = if let Some(ident) = field.ident.as_ref() {
                        // Bar { .., vid: ValueId }
                        FieldKind::Named(ident)
                    } else {
                        // Bar(.., ValueId)
                        FieldKind::Numbered(idx)
                    };
                    select_field = Some(Field { kind, direct: true });
                    break;
                }
            }
        }

        if select_field.is_none() && variant.fields.len() == 1 {
            // `Foo(field)` or `Foo{field: Ty}`
            let field = variant.fields.iter().next().unwrap();
            if let Some(ident) = field.ident.as_ref() {
                todo!("named field");
            } else {
                select_field = Some(Field {
                    kind: FieldKind::Numbered(0),
                    direct: false,
                });
            }
        }

        let select_field = select_field.or(default_select_field.clone());

        if let Some(select_field) = select_field {
            push_handler(&mut variants, enum_name, variant, select_field);
        } else {
            return Err(Error::new(
                variant.span(),
                format!("ValueId field not found in variant {}", variant.ident,),
            ));
        }
    }

    let (impl_generics, ty_generics, where_clause) = input.generics.split_for_impl();

    let output = quote!(impl #impl_generics HasOperands for #enum_name #ty_generics #where_clause {
        fn operands(&self) -> LocId {
            match self {
                #(#variants),*
            }
        }

        fn operands_mut(&mut self) -> &mut [ValueId] {
            todo!();
        }
    });

    Ok(output)
}

#[derive(Clone)]
struct Field<'a> {
    kind: FieldKind<'a>,
    direct: bool,
}

#[derive(Clone)]
enum FieldKind<'a> {
    Named(&'a Ident),
    None,
    Numbered(usize),
}

fn push_handler(
    variants: &mut Vec<TokenStream>,
    enum_name: &Ident,
    variant: &Variant,
    field: Field<'_>,
) {
    let variant_name = &variant.ident;

    let reference = if field.direct {
        quote!(*f)
    } else {
        quote!(f.operands())
    };

    match field.kind {
        FieldKind::Named(id) => {
            variants.push(quote!(#enum_name::#variant_name { #id: f, .. } => #reference));
        }
        FieldKind::None => {
            variants.push(quote!(#enum_name::#variant_name(..) => LocId::NONE));
        }
        FieldKind::Numbered(idx) => {
            let mut fields = Vec::new();
            for (field_idx, _) in variant.fields.iter().enumerate() {
                if field_idx == idx {
                    fields.push(quote!(f));
                } else {
                    fields.push(quote!(_));
                }
            }
            variants.push(quote!(#enum_name::#variant_name(#(#fields),*) => #reference));
        }
    }
}

fn handle_has_operands_attr(attrs: &[Attribute]) -> Result<Option<Field<'_>>> {
    for attr in attrs {
        if attr.path.is_ident("has_operands") {
            let meta = attr.parse_meta()?;
            match meta {
                Meta::Path(path) => {
                    return Err(Error::new(path.span(), "Arguments expected"));
                }
                Meta::List(list) => {
                    // has_operands(A, B, C)
                    if list.nested.len() != 1 {
                        return Err(Error::new(list.span(), "Only one argument expected"));
                    }

                    match &list.nested[0] {
                        NestedMeta::Lit(Lit::Int(i)) => {
                            return Ok(Some(Field {
                                kind: FieldKind::Numbered(i.base10_parse()?),
                                direct: false,
                            }));
                        }
                        NestedMeta::Meta(Meta::Path(meta)) if meta.is_ident("none") => {
                            return Ok(Some(Field {
                                kind: FieldKind::None,
                                direct: true,
                            }));
                        }
                        i => {
                            todo!("Unhandled: {:?}", i);
                        }
                    }
                }
                Meta::NameValue(list) => {
                    todo!("C");
                }
            }
        }
    }

    Ok(None)
}
