// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use proc_macro2::Ident;
use syn::Path;

#[derive(Clone, Debug, Eq, PartialEq)]
pub(crate) enum SimpleType<'a> {
    // [T; #]
    Array(&'a Ident),
    // Box<[T]>
    BoxedSlice(&'a Ident),
    // &[T]
    RefSlice(&'a Ident),
    // [T]
    Slice(&'a Ident),
    // T
    Unit(&'a Ident),
    Unknown,
}

impl<'a> SimpleType<'a> {
    pub(crate) fn get_ident(&self) -> Option<&'a Ident> {
        match self {
            SimpleType::Unknown => None,
            SimpleType::Unit(id)
            | SimpleType::Array(id)
            | SimpleType::BoxedSlice(id)
            | SimpleType::RefSlice(id)
            | SimpleType::Slice(id) => Some(id),
        }
    }

    pub(crate) fn is_based_on(&self, s: &str) -> bool {
        if let Some(id) = self.get_ident() {
            id == s
        } else {
            false
        }
    }

    pub(crate) fn from_type(ty: &'a syn::Type) -> SimpleType<'a> {
        match ty {
            syn::Type::Path(ty) => {
                let path = &ty.path;
                if let Some(id) = path.get_ident() {
                    SimpleType::Unit(id)
                } else if let Some(box_ty) = get_box_ty(path) {
                    let box_ty = SimpleType::from_type(box_ty);
                    match box_ty {
                        SimpleType::Slice(id) => {
                            // Box<[T]>
                            SimpleType::BoxedSlice(id)
                        }
                        SimpleType::Array(_)
                        | SimpleType::BoxedSlice(_)
                        | SimpleType::RefSlice(_)
                        | SimpleType::Unit(_)
                        | SimpleType::Unknown => {
                            // Box<[T; #]>
                            // Box<Box<[T]>>
                            // Box<&[T]>
                            // Box<T>
                            SimpleType::Unknown
                        }
                    }
                } else {
                    SimpleType::Unknown
                }
            }
            syn::Type::Reference(ref_ty) => {
                let ref_ty = SimpleType::from_type(&ref_ty.elem);
                match ref_ty {
                    SimpleType::Slice(id) => {
                        // &[T]
                        SimpleType::RefSlice(id)
                    }
                    SimpleType::Array(_)
                    | SimpleType::BoxedSlice(_)
                    | SimpleType::RefSlice(_)
                    | SimpleType::Unit(_)
                    | SimpleType::Unknown => {
                        // &[T; #]
                        // &Box<[T]>
                        // &&[T]
                        // &T
                        SimpleType::Unknown
                    }
                }
            }
            syn::Type::Slice(slice_ty) => {
                let slice_ty = SimpleType::from_type(&slice_ty.elem);
                match slice_ty {
                    SimpleType::Unit(id) => SimpleType::Slice(id),
                    SimpleType::Array(_)
                    | SimpleType::BoxedSlice(_)
                    | SimpleType::RefSlice(_)
                    | SimpleType::Slice(_)
                    | SimpleType::Unknown => {
                        // [[T; #]]
                        // [Box<[T]>]
                        // [&[T]]
                        // [[T]]
                        SimpleType::Unknown
                    }
                }
            }
            syn::Type::Array(array_ty) => {
                let array_ty = SimpleType::from_type(&array_ty.elem);
                match array_ty {
                    SimpleType::Unit(id) => SimpleType::Array(id),
                    SimpleType::Array(_)
                    | SimpleType::BoxedSlice(_)
                    | SimpleType::RefSlice(_)
                    | SimpleType::Slice(_)
                    | SimpleType::Unknown => {
                        // [[T; #]; #]
                        // [Box<[T]>; #]
                        // [&[T]; #]
                        // [[T]; #]
                        SimpleType::Unknown
                    }
                }
            }
            _ => SimpleType::Unknown,
        }
    }
}

fn get_box_ty<'a>(path: &'a Path) -> Option<&'a syn::Type> {
    if path.segments.len() != 1 {
        return None;
    }

    let segment = &path.segments[0];
    if segment.ident != "Box" {
        return None;
    }

    let generic_args = match &segment.arguments {
        syn::PathArguments::AngleBracketed(generic_args) => generic_args,
        _ => {
            return None;
        }
    };

    let gty = generic_args.args.first()?;

    match gty {
        syn::GenericArgument::Type(ty) => Some(ty),
        _ => None,
    }
}
