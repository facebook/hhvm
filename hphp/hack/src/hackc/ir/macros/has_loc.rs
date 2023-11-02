// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(clippy::todo)]

use std::borrow::Cow;

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
use syn::Meta;
use syn::NestedMeta;
use syn::Result;
use syn::Variant;

use crate::simple_type::SimpleType;
use crate::util::InterestingFields;

/// Builds a HasLoc impl.
///
/// The build rules are as follows:
/// - For a struct it just looks for a field with a type of LocId.
/// - For an enum it does a match on each variant.
///   - For either tuple variants or struct variants it looks for a field with a
///     type of LocId.
///   - For a tuple variant with a single non-LocId type and calls `.loc_id()`
///     on that field.
///   - Otherwise you can specify `#[has_loc(n)]` where `n` is the index of the
///     field to call `.loc_id()` on.  `#[has_loc(n)]` can also be used on the
///     whole enum to provide a default index.
///
pub(crate) fn build_has_loc(input: TokenStream) -> Result<TokenStream> {
    let input = syn::parse2::<DeriveInput>(input)?;
    match &input.data {
        Data::Enum(data) => build_has_loc_enum(&input, data),
        Data::Struct(data) => build_has_loc_struct(&input, data),
        Data::Union(_) => Err(Error::new(input.span(), "Union not handled")),
    }
}

fn field_might_contain_buried_loc_id(ty: &SimpleType<'_>) -> bool {
    if let Some(ident) = ty.get_ident() {
        !(ident == "BlockId"
            || ident == "ClassId"
            || ident == "ConstId"
            || ident == "ValueId"
            || ident == "LocalId"
            || ident == "MethodId"
            || ident == "ParamId"
            || ident == "VarId"
            || ident == "usize"
            || ident == "u32")
    } else {
        true
    }
}

fn build_has_loc_struct(input: &DeriveInput, data: &DataStruct) -> Result<TokenStream> {
    // struct Foo {
    //   ...
    //   loc: LocId,
    // }
    let struct_name = &input.ident;

    let default_select_field = handle_has_loc_attr(&input.attrs)?;
    let loc_field = if let Some(f) = default_select_field {
        match f.kind {
            FieldKind::Named(name) => {
                let name = name.to_string();
                let field = data
                    .fields
                    .iter()
                    .find(|field| field.ident.as_ref().map_or(false, |id| id == &name))
                    .ok_or_else(|| Error::new(input.span(), format!("Field '{name}' not found")))?
                    .ident
                    .as_ref()
                    .unwrap();

                quote!(#field.loc_id())
            }
            FieldKind::None => todo!(),
            FieldKind::Numbered(_) => todo!(),
        }
    } else {
        let field = data
            .fields
            .iter()
            .enumerate()
            .map(|(i, field)| (i, field, SimpleType::from_type(&field.ty)))
            .find(|(_, _, ty)| ty.is_based_on("LocId"));

        let (idx, field, _) =
            field.ok_or_else(|| Error::new(input.span(), "No field with type LocId found"))?;

        if let Some(ident) = field.ident.as_ref() {
            ident.to_token_stream()
        } else {
            syn::Index::from(idx).to_token_stream()
        }
    };

    let (impl_generics, ty_generics, where_clause) = input.generics.split_for_impl();

    let output = quote!(impl #impl_generics HasLoc for #struct_name #ty_generics #where_clause {
        fn loc_id(&self) -> LocId {
            self.#loc_field
        }
    });

    Ok(output)
}

fn get_select_field<'a>(
    variant: &'a Variant,
    default_select_field: &Option<Field<'a>>,
) -> Result<Option<Field<'a>>> {
    if let Some(f) = handle_has_loc_attr(&variant.attrs)? {
        return Ok(Some(f));
    }

    if let Some(f) = default_select_field.as_ref() {
        return Ok(Some(f.clone()));
    }

    let mut interesting_fields = InterestingFields::None;
    for (idx, field) in variant.fields.iter().enumerate() {
        let ty = SimpleType::from_type(&field.ty);
        if ty.is_based_on("LocId") {
            let kind = if let Some(ident) = field.ident.as_ref() {
                // Bar { .., loc: LocId }
                FieldKind::Named(Cow::Borrowed(ident))
            } else {
                // Bar(.., LocId)
                FieldKind::Numbered(idx)
            };
            return Ok(Some(Field { kind, ty }));
        } else if field_might_contain_buried_loc_id(&ty) {
            // Report the type as 'unknown' because it's not a type that's
            // related to LocId.
            interesting_fields.add(idx, field.ident.as_ref(), SimpleType::Unknown);
        }
    }

    match interesting_fields {
        InterestingFields::None => {
            let kind = FieldKind::None;
            let ty = SimpleType::Unknown;
            Ok(Some(Field { kind, ty }))
        }
        InterestingFields::One(idx, ident, ty) => {
            // There's only a single field that could possibly contain a buried
            // LocId.
            let kind = ident.map_or_else(
                || FieldKind::Numbered(idx),
                |id| FieldKind::Named(Cow::Borrowed(id)),
            );
            Ok(Some(Field { kind, ty }))
        }
        InterestingFields::Many => Ok(None),
    }
}

fn build_has_loc_enum(input: &DeriveInput, data: &DataEnum) -> Result<TokenStream> {
    // enum Foo {
    //   Bar(.., LocId),
    //   Baz { .., loc: LocId },
    // }

    let default_select_field = handle_has_loc_attr(&input.attrs)?;

    let enum_name = &input.ident;
    let mut variants: Vec<TokenStream> = Vec::new();
    for variant in data.variants.iter() {
        let select_field = get_select_field(variant, &default_select_field)?;
        if let Some(select_field) = select_field {
            push_handler(&mut variants, enum_name, variant, select_field);
        } else {
            return Err(Error::new(
                variant.span(),
                format!("LocId field not found in variant {}", variant.ident,),
            ));
        }
    }

    let (impl_generics, ty_generics, where_clause) = input.generics.split_for_impl();

    let output = quote!(impl #impl_generics HasLoc for #enum_name #ty_generics #where_clause {
            fn loc_id(&self) -> LocId {
                match self {
                    #(#variants),*
                }
            }
    });

    Ok(output)
}

#[derive(Clone)]
struct Field<'a> {
    kind: FieldKind<'a>,
    ty: SimpleType<'a>,
}

#[derive(Clone)]
enum FieldKind<'a> {
    Named(Cow<'a, Ident>),
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

    let reference = match (&field.kind, &field.ty) {
        (FieldKind::None, _) => quote!(LocId::NONE),
        (_, SimpleType::Unknown) => quote!(f.loc_id()),
        (_, SimpleType::Unit(_)) => quote!(*f),
        (_, SimpleType::Array(_))
        | (_, SimpleType::BoxedSlice(_))
        | (_, SimpleType::RefSlice(_))
        | (_, SimpleType::Slice(_)) => {
            todo!("Unhandled type: {:?}", field.ty)
        }
    };

    let params = match field.kind {
        FieldKind::Named(id) => {
            quote!( { #id: f, .. })
        }
        FieldKind::None => match &variant.fields {
            syn::Fields::Named(_) => quote!({ .. }),
            syn::Fields::Unnamed(_) => quote!((..)),
            syn::Fields::Unit => TokenStream::default(),
        },
        FieldKind::Numbered(idx) => {
            let mut fields = Vec::new();
            for (field_idx, _) in variant.fields.iter().enumerate() {
                if field_idx == idx {
                    fields.push(quote!(f));
                } else {
                    fields.push(quote!(_));
                }
            }
            quote!((#(#fields),*))
        }
    };

    variants.push(quote!(#enum_name::#variant_name #params => #reference));
}

fn handle_has_loc_attr(attrs: &[Attribute]) -> Result<Option<Field<'_>>> {
    for attr in attrs {
        if attr.path.is_ident("has_loc") {
            let meta = attr.parse_meta()?;
            match meta {
                Meta::Path(path) => {
                    return Err(Error::new(path.span(), "Arguments expected"));
                }
                Meta::List(list) => {
                    // has_loc(A, B, C)
                    if list.nested.len() != 1 {
                        return Err(Error::new(list.span(), "Only one argument expected"));
                    }

                    match &list.nested[0] {
                        NestedMeta::Lit(Lit::Int(i)) => {
                            return Ok(Some(Field {
                                kind: FieldKind::Numbered(i.base10_parse()?),
                                ty: SimpleType::Unknown,
                            }));
                        }
                        NestedMeta::Lit(Lit::Str(n)) => {
                            return Ok(Some(Field {
                                kind: FieldKind::Named(Cow::Owned(Ident::new(
                                    &n.value(),
                                    Span::call_site(),
                                ))),
                                ty: SimpleType::Unknown,
                            }));
                        }
                        NestedMeta::Meta(Meta::Path(meta)) if meta.is_ident("none") => {
                            return Ok(Some(Field {
                                kind: FieldKind::None,
                                ty: SimpleType::Unknown,
                            }));
                        }
                        i => {
                            todo!("Unhandled: {:?}", i);
                        }
                    }
                }
                Meta::NameValue(_list) => {
                    todo!();
                }
            }
        }
    }

    Ok(None)
}
