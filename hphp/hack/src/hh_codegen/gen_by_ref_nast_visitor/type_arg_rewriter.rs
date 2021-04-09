// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Rewrite a declaration of generic parameters (e.g., from a type declaration)
/// into the set of type arguments which should be provided for that type.
/// Replaces AAST type parameters (`Ex`, `En`, etc.) with their instantiations
/// for the Named AST, allowing the generated visitor to avoid specifying type
/// parameters. This makes it less general (it cannot visit an unnamed AST or a
/// typed AST), but easier to use.
pub fn rewrite_type_args(generics: &syn::Generics) -> proc_macro2::TokenStream {
    use syn::visit_mut::VisitMut;

    let (_, ty_generics, _) = generics.split_for_impl();
    if generics.params.is_empty() {
        quote::quote!()
    } else {
        // Sneaky: `split_for_impl` returns thin wrappers around the `Generics`
        // value (which represents an angle-bracketed list of type parameters).
        // When converted to a token stream, the `TyGenerics` wrapper emits a
        // token stream which looks like a type *argument* list (because it is
        // intended to be used as one). We can convert the `TyGenerics` to an
        // `AngleBrackedGenericArguments` by converting it to a token stream,
        // then parsing it.
        let mut type_args = syn::parse_quote!(#ty_generics);
        TypeArgRewriter.visit_angle_bracketed_generic_arguments_mut(&mut type_args);
        quote::quote!(#type_args)
    }
}

struct TypeArgRewriter;
impl syn::visit_mut::VisitMut for TypeArgRewriter {
    fn visit_generic_argument_mut(&mut self, node: &mut syn::GenericArgument) {
        if let syn::GenericArgument::Type(ty) = node {
            if let syn::Type::Path(path) = ty {
                if let Some(segment) = path.path.segments.first() {
                    if segment.ident == "Ex" {
                        *ty = syn::parse_quote!(&'a crate::pos::Pos<'a>)
                    } else if segment.ident == "Fb" {
                        *ty = syn::parse_quote!(crate::nast::FuncBodyAnn<'a>)
                    } else if segment.ident == "En" || segment.ident == "Hi" {
                        *ty = syn::parse_quote!(())
                    }
                }
            }
        }
    }
}
