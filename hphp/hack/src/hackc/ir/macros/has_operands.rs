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

/// Builds a HasOperands impl.
pub(crate) fn build_has_operands(input: TokenStream) -> Result<TokenStream> {
    let input = syn::parse2::<DeriveInput>(input)?;
    match &input.data {
        Data::Enum(data) => build_has_operands_enum(&input, data),
        Data::Struct(data) => build_has_operands_struct(&input, data),
        Data::Union(_) => Err(Error::new(input.span(), "Union not handled")),
    }
}

fn field_might_contain_buried_value_id(ty: &SimpleType<'_>) -> bool {
    if let Some(ident) = ty.get_ident() {
        !(ident == "BlockId"
            || ident == "ClassName"
            || ident == "ConstName"
            || ident == "LocId"
            || ident == "MethodName"
            || ident == "FunctionName"
            || ident == "LocalId"
            || ident == "ParamId"
            || ident == "PropId"
            || ident == "VarId"
            || ident == "usize"
            || ident == "u32")
    } else {
        true
    }
}

fn build_has_operands_struct(input: &DeriveInput, data: &DataStruct) -> Result<TokenStream> {
    // struct Foo {
    //   ...
    //   vid: ValueId,
    // }

    let struct_name = &input.ident;

    let field = data
        .fields
        .iter()
        .enumerate()
        .map(|(i, field)| (i, field, SimpleType::from_type(&field.ty)))
        .find(|(_, _, ty)| ty.is_based_on("ValueId"));

    let (idx, field, sty) =
        field.ok_or_else(|| Error::new(input.span(), "No field with type ValueId found"))?;

    let vid_field = if let Some(ident) = field.ident.as_ref() {
        ident.to_token_stream()
    } else {
        syn::Index::from(idx).to_token_stream()
    };

    let (impl_generics, ty_generics, where_clause) = input.generics.split_for_impl();

    // The unit type here is guaranteed to be 'ValueId'
    let (getter, getter_mut) = match sty {
        SimpleType::BoxedSlice(_) => {
            // Box<[T]>
            let getter = quote!(&self.#vid_field);
            let getter_mut = quote!(&mut self.#vid_field);
            (getter, getter_mut)
        }
        SimpleType::Unit(_) => {
            // T
            let getter = quote!(std::slice::from_ref(&self.#vid_field));
            let getter_mut = quote!(std::slice::from_mut(&mut self.#vid_field));
            (getter, getter_mut)
        }
        SimpleType::Array(_)
        | SimpleType::Unknown
        | SimpleType::RefSlice(_)
        | SimpleType::Slice(_) => {
            todo!("Unhandled type: {:?}", sty)
        }
    };

    let output = quote!(impl #impl_generics HasOperands for #struct_name #ty_generics #where_clause {
        fn operands(&self) -> &[ValueId] {
            #getter
        }

        fn operands_mut(&mut self) -> &mut [ValueId] {
            #getter_mut
        }
    });

    // eprintln!("OUTPUT: {}", output);

    Ok(output)
}

fn get_select_field<'a>(
    variant: &'a Variant,
    default_select_field: &Option<Field<'a>>,
) -> Result<Option<Field<'a>>> {
    if let Some(f) = handle_has_operands_attr(&variant.attrs)? {
        return Ok(Some(f));
    }

    if let Some(f) = default_select_field.as_ref() {
        return Ok(Some(f.clone()));
    }

    let mut interesting_fields = InterestingFields::None;
    for (idx, field) in variant.fields.iter().enumerate() {
        let ty = SimpleType::from_type(&field.ty);
        if ty.is_based_on("ValueId") {
            let kind = if let Some(ident) = field.ident.as_ref() {
                // Bar { .., vid: ValueId }
                FieldKind::Named(Cow::Borrowed(ident))
            } else {
                // Bar(.., ValueId)
                FieldKind::Numbered(idx)
            };
            return Ok(Some(Field { kind, ty }));
        } else if field_might_contain_buried_value_id(&ty) {
            // Report the type as 'unknown' because it's not a type that's
            // related to ValueId.
            interesting_fields.add(idx, field.ident.as_ref(), SimpleType::Unknown);
        }
    }

    // There are no explicit ValueId fields.

    let (kind, ty) = match interesting_fields {
        InterestingFields::None => {
            // There are no fields which might contain a buried ValueId.
            (FieldKind::None, SimpleType::Unknown)
        }
        InterestingFields::One(idx, ident, ty) => {
            // If there's only a single field that might contain a buried
            // ValueId. If it doesn't then the caller needs to be explicit.
            match ident {
                Some(ident) => (FieldKind::Named(Cow::Borrowed(ident)), ty),
                None => (FieldKind::Numbered(idx), ty),
            }
        }
        InterestingFields::Many => {
            // There are a bunch of fields which could contain a buried ValueId
            // - so we have no idea which to use.  Caller needs to specify.
            return Ok(None);
        }
    };

    Ok(Some(Field { kind, ty }))
}

fn build_has_operands_enum(input: &DeriveInput, data: &DataEnum) -> Result<TokenStream> {
    // enum Foo {
    //   Bar(.., LocId),
    //   Baz { .., loc: LocId },
    // }

    let default_select_field = handle_has_operands_attr(&input.attrs)?;

    let enum_name = &input.ident;
    let mut variants: Vec<(TokenStream, TokenStream)> = Vec::new();

    for variant in data.variants.iter() {
        let select_field = get_select_field(variant, &default_select_field)?;
        if let Some(select_field) = select_field {
            push_handler(&mut variants, enum_name, variant, select_field);
        } else {
            return Err(Error::new(
                variant.span(),
                format!("ValueId field not found in variant {}", variant.ident),
            ));
        }
    }

    let (impl_generics, ty_generics, where_clause) = input.generics.split_for_impl();

    let (variants, variants_mut): (Vec<TokenStream>, Vec<TokenStream>) =
        variants.into_iter().unzip();

    let output = quote!(impl #impl_generics HasOperands for #enum_name #ty_generics #where_clause {
        fn operands(&self) -> &[ValueId] {
            match self {
                #(#variants),*
            }
        }

        fn operands_mut(&mut self) -> &mut [ValueId] {
            match self {
                #(#variants_mut),*
            }
        }
    });

    // eprintln!("OUTPUT: {}", output);

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
    variants: &mut Vec<(TokenStream, TokenStream)>,
    enum_name: &Ident,
    variant: &Variant,
    field: Field<'_>,
) {
    let variant_name = &variant.ident;

    let (reference, reference_mut) = match (&field.kind, &field.ty) {
        (FieldKind::None, _) => {
            let reference = quote!(&[]);
            let reference_mut = quote!(&mut []);
            (reference, reference_mut)
        }
        (_, SimpleType::Unit(_)) => {
            let reference = quote!(std::slice::from_ref(f));
            let reference_mut = quote!(std::slice::from_mut(f));
            (reference, reference_mut)
        }
        (_, SimpleType::Array(_)) | (_, SimpleType::BoxedSlice(_)) => {
            let reference = quote!(f);
            let reference_mut = quote!(f);
            (reference, reference_mut)
        }
        (_, SimpleType::Unknown) => {
            let reference = quote!(f.operands());
            let reference_mut = quote!(f.operands_mut());
            (reference, reference_mut)
        }
        (_, SimpleType::RefSlice(_)) | (_, SimpleType::Slice(_)) => {
            todo!("Unhandled enum type: {:?}", field.ty)
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

    variants.push((
        quote!(#enum_name::#variant_name #params => #reference),
        quote!(#enum_name::#variant_name #params => #reference_mut),
    ));
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
                    // operands(A, B, C)
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
                Meta::NameValue(_) => {
                    todo!();
                }
            }
        }
    }

    Ok(None)
}
