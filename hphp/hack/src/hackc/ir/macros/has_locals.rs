// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Cow;

use proc_macro2::Ident;
use proc_macro2::Span;
use proc_macro2::TokenStream;
use quote::quote;
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

/// Builds a HasLocals impl.
pub(crate) fn build_has_locals(input: TokenStream) -> Result<TokenStream> {
    let input = syn::parse2::<DeriveInput>(input)?;
    match &input.data {
        Data::Enum(data) => build_has_locals_enum(&input, data),
        Data::Struct(data) => build_has_locals_struct(&input, data),
        Data::Union(_) => Err(Error::new(input.span(), "Union not handled")),
    }
}

fn field_might_contain_buried_local_id(ty: &SimpleType<'_>) -> bool {
    if let Some(ident) = ty.get_ident() {
        !(ident == "BlockId"
            || ident == "BareThisOp"
            || ident == "ClassGetCMode"
            || ident == "ClassId"
            || ident == "CmpOp"
            || ident == "CollectionType"
            || ident == "ConstId"
            || ident == "ContCheckOp"
            || ident == "FatalOp"
            || ident == "FunctionId"
            || ident == "IncDecOp"
            || ident == "InitPropOp"
            || ident == "IsTypeOp"
            || ident == "IterId"
            || ident == "LocId"
            || ident == "MethodId"
            || ident == "ParamId"
            || ident == "Predicate"
            || ident == "PropId"
            || ident == "ReadonlyOp"
            || ident == "SetOpOp"
            || ident == "SurpriseCheck"
            || ident == "TypeStructResolveOp"
            || ident == "TypeStructEnforceKind"
            || ident == "AsTypeStructExceptionKind"
            || ident == "UnitBytesId"
            || ident == "ValueId"
            || ident == "VarId"
            || ident == "u32"
            || ident == "usize")
    } else {
        true
    }
}

fn find_local_id_field<'a>(input: &'a DeriveInput, data: &'a DataStruct) -> Result<Field<'a>> {
    let mut fields: Vec<_> = data
        .fields
        .iter()
        .enumerate()
        .filter_map(|(i, field)| {
            let sty = SimpleType::from_type(&field.ty);
            if sty.is_based_on("LocalId") {
                Some((i, field, sty))
            } else {
                None
            }
        })
        .collect();

    if fields.len() > 1 {
        return Err(Error::new(
            input.span(),
            format!(
                "Too many fields with LocalId type: [{}]",
                fields
                    .iter()
                    .map(|(i, _, _)| i.to_string())
                    .collect::<Vec<_>>()
                    .join(", ")
            ),
        ));
    }

    let (idx, field, sty) = if let Some(info) = fields.pop() {
        info
    } else {
        let mut fields: Vec<_> = data
            .fields
            .iter()
            .enumerate()
            .filter_map(|(i, field)| {
                let sty = SimpleType::from_type(&field.ty);
                if matches!(sty, SimpleType::Unit(_)) {
                    if field_might_contain_buried_local_id(&sty) {
                        Some((i, field, sty))
                    } else {
                        None
                    }
                } else {
                    None
                }
            })
            .collect();
        if fields.len() > 1 {
            return Err(Error::new(
                input.span(),
                format!(
                    "Too many fields with possible LocalId type: [{}]",
                    fields
                        .iter()
                        .map(|(i, _, _)| i.to_string())
                        .collect::<Vec<_>>()
                        .join(", ")
                ),
            ));
        }
        if let Some(info) = fields.pop() {
            info
        } else {
            return Err(Error::new(input.span(), "No field could contain LocalId"));
        }
    };

    let field = if let Some(ident) = field.ident.as_ref() {
        Field {
            kind: FieldKind::Named(Cow::Borrowed(ident)),
            ty: sty,
        }
    } else {
        Field {
            kind: FieldKind::Numbered(idx),
            ty: sty,
        }
    };

    Ok(field)
}

fn build_has_locals_struct(input: &DeriveInput, data: &DataStruct) -> Result<TokenStream> {
    // struct Foo {
    //   ...
    //   lid: LocalId,
    // }

    let struct_name = &input.ident;

    let lid_field = {
        if let Some(f) = handle_has_locals_attr(&input.attrs)? {
            f
        } else {
            find_local_id_field(input, data)?
        }
    };

    let (impl_generics, ty_generics, where_clause) = input.generics.split_for_impl();

    let name = match lid_field {
        Field {
            kind: FieldKind::None,
            ..
        } => {
            quote!()
        }
        Field {
            kind: FieldKind::Named(ref name),
            ..
        } => quote!(self.#name),
        Field {
            kind: FieldKind::Numbered(idx),
            ..
        } => {
            let idx = syn::Index::from(idx);
            quote!(self.#idx)
        }
    };
    let select = reference_for_ty(&lid_field.kind, &lid_field.ty, name, false);

    let output = quote!(impl #impl_generics HasLocals for #struct_name #ty_generics #where_clause {
        fn locals(&self) -> &[LocalId] {
            #select
        }
    });

    // eprintln!("OUTPUT: {}", output);

    Ok(output)
}

fn get_select_field<'a>(
    variant: &'a Variant,
    default_select_field: &Option<Field<'a>>,
) -> Result<Option<Field<'a>>> {
    if let Some(f) = handle_has_locals_attr(&variant.attrs)? {
        return Ok(Some(f));
    }

    if let Some(f) = default_select_field.as_ref() {
        return Ok(Some(f.clone()));
    }

    let mut interesting_fields = InterestingFields::None;
    for (idx, field) in variant.fields.iter().enumerate() {
        let ty = SimpleType::from_type(&field.ty);
        if ty.is_based_on("LocalId") {
            let kind = if let Some(ident) = field.ident.as_ref() {
                // Bar { .., lid: LocalId }
                FieldKind::Named(Cow::Borrowed(ident))
            } else {
                // Bar(.., LocalId)
                FieldKind::Numbered(idx)
            };
            return Ok(Some(Field { kind, ty }));
        } else if field_might_contain_buried_local_id(&ty) {
            // Report the type as 'unknown' because it's not a type that's
            // related to LocalId.
            interesting_fields.add(idx, field.ident.as_ref(), SimpleType::Unknown);
        }
    }

    // There are no explicit LocalId fields.

    let (kind, ty) = match interesting_fields {
        InterestingFields::None => {
            // There are no fields which might contain a buried LocalId.
            (FieldKind::None, SimpleType::Unknown)
        }
        InterestingFields::One(idx, ident, ty) => {
            // If there's only a single field that might contain a buried
            // LocalId. If it doesn't then the caller needs to be explicit.
            match ident {
                Some(ident) => (FieldKind::Named(Cow::Borrowed(ident)), ty),
                None => (FieldKind::Numbered(idx), ty),
            }
        }
        InterestingFields::Many => {
            // There are a bunch of fields which could contain a buried LocalId
            // - so we have no idea which to use.  Caller needs to specify.
            return Ok(None);
        }
    };

    Ok(Some(Field { kind, ty }))
}

fn build_has_locals_enum(input: &DeriveInput, data: &DataEnum) -> Result<TokenStream> {
    // enum Foo {
    //   Bar(.., LocId),
    //   Baz { .., loc: LocId },
    // }

    let default_select_field = handle_has_locals_attr(&input.attrs)?;

    let enum_name = &input.ident;
    let mut variants: Vec<TokenStream> = Vec::new();

    for variant in data.variants.iter() {
        let select_field = get_select_field(variant, &default_select_field)?;
        if let Some(select_field) = select_field {
            push_handler(&mut variants, enum_name, variant, select_field);
        } else {
            return Err(Error::new(
                variant.span(),
                format!("LocalId field not found in variant {}", variant.ident),
            ));
        }
    }

    let (impl_generics, ty_generics, where_clause) = input.generics.split_for_impl();

    let output = quote!(impl #impl_generics HasLocals for #enum_name #ty_generics #where_clause {
        fn locals(&self) -> &[LocalId] {
            match self {
                #(#variants),*
            }
        }
    });

    // eprintln!("OUTPUT: {}", output);

    Ok(output)
}

#[derive(Clone, Debug)]
struct Field<'a> {
    kind: FieldKind<'a>,
    ty: SimpleType<'a>,
}

#[derive(Clone, Debug)]
enum FieldKind<'a> {
    Named(Cow<'a, Ident>),
    None,
    Numbered(usize),
}

#[allow(clippy::todo)]
fn reference_for_ty(
    kind: &FieldKind<'_>,
    ty: &SimpleType<'_>,
    name: TokenStream,
    is_ref: bool,
) -> TokenStream {
    match (kind, ty) {
        (FieldKind::None, _) => {
            quote!(&[])
        }
        (_, SimpleType::Unit(_)) => {
            if is_ref {
                quote!(std::slice::from_ref(#name))
            } else {
                quote!(std::slice::from_ref(&#name))
            }
        }
        (_, SimpleType::Array(_)) | (_, SimpleType::BoxedSlice(_)) => {
            if is_ref {
                quote!(#name)
            } else {
                quote!(&#name)
            }
        }
        (_, SimpleType::Unknown) => {
            quote!(#name.locals())
        }
        (_, SimpleType::RefSlice(_)) | (_, SimpleType::Slice(_)) => {
            todo!("Unhandled enum type: {:?}", ty)
        }
    }
}

fn push_handler(
    variants: &mut Vec<TokenStream>,
    enum_name: &Ident,
    variant: &Variant,
    field: Field<'_>,
) {
    let variant_name = &variant.ident;

    let reference = reference_for_ty(&field.kind, &field.ty, quote!(f), true);

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

#[allow(clippy::todo)]
fn handle_has_locals_attr(attrs: &[Attribute]) -> Result<Option<Field<'_>>> {
    for attr in attrs {
        if attr.path.is_ident("has_locals") {
            let meta = attr.parse_meta()?;
            match meta {
                Meta::Path(path) => {
                    return Err(Error::new(path.span(), "Arguments expected"));
                }
                Meta::List(list) => {
                    // locals(A, B, C)
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
