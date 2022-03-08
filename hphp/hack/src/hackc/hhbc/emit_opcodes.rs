// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::{ImmType, InstrFlags, OpcodeData};
use proc_macro2::{Ident, Punct, Spacing, Span, TokenStream};
use quote::{quote, ToTokens};
use syn::{punctuated::Punctuated, token, ItemEnum, Lifetime, Result, Variant};

pub fn emit_opcodes(input: TokenStream, opcodes: &[OpcodeData]) -> Result<TokenStream> {
    let mut item_enum = syn::parse2::<ItemEnum>(input)?;
    let generics = &item_enum.generics;

    let lifetime = &generics.lifetimes().next().unwrap().lifetime;

    for opcode in opcodes {
        let name = Ident::new(opcode.name, Span::call_site());

        let mut body = Vec::new();
        body.extend(name.to_token_stream());

        if !opcode.immediates.is_empty() {
            let is_struct = opcode.flags.contains(InstrFlags::AS_STRUCT);

            let mut imms = Vec::new();
            for (idx, (name, ty)) in opcode.immediates.iter().enumerate() {
                if idx != 0 {
                    imms.push(Punct::new(',', Spacing::Alone).into());
                }
                let ty = convert_imm_type(ty, lifetime);
                if is_struct {
                    let name = Ident::new(name, Span::call_site());
                    imms.extend(quote!( #name: #ty ));
                } else {
                    imms.extend(ty);
                }
            }

            if is_struct {
                body.extend(quote!( { #(#imms)* } ));
            } else {
                body.extend(quote!( ( #(#imms)* ) ));
            }
        }

        let variant = syn::parse2::<Variant>(quote!(#(#body)*))?;
        item_enum.variants.push(variant);
    }

    // Silliness to match rustfmt...
    if !item_enum.variants.trailing_punct() {
        item_enum.variants.push_punct(Default::default());
    }

    Ok(quote!(#item_enum))
}

pub fn emit_impl_targets(input: TokenStream, opcodes: &[OpcodeData]) -> Result<TokenStream> {
    // impl Targets for Instruct<'_> {
    //   pub fn targets(&self) -> &[Label] {
    //     match self {
    //       ..
    //     }
    //   }
    // }
    //

    let item_enum = syn::parse2::<ItemEnum>(input)?;
    let name = &item_enum.ident;
    let (impl_generics, impl_types, impl_where) = item_enum.generics.split_for_impl();

    let mut with_targets_ref: Vec<TokenStream> = Vec::new();
    let mut with_targets_mut: Vec<TokenStream> = Vec::new();
    let mut without_targets: Punctuated<TokenStream, token::Or> = Punctuated::new();

    for opcode in opcodes {
        let variant_name = Ident::new(opcode.name, Span::call_site());
        let variant_name = quote!(#name::#variant_name);
        let is_struct = opcode.flags.contains(InstrFlags::AS_STRUCT);

        fn is_label_type(imm_ty: &ImmType) -> bool {
            match imm_ty {
                ImmType::BA | ImmType::BA2 | ImmType::FCA | ImmType::BLA => true,
                ImmType::ARR(subty) => is_label_type(subty),
                ImmType::AA
                | ImmType::DA
                | ImmType::I64A
                | ImmType::IA
                | ImmType::ILA
                | ImmType::ITA
                | ImmType::IVA
                | ImmType::KA
                | ImmType::LA
                | ImmType::LAR
                | ImmType::NA
                | ImmType::NLA
                | ImmType::OA(_)
                | ImmType::RATA
                | ImmType::SA
                | ImmType::SLA
                | ImmType::VSA
                | ImmType::OAL(_) => false,
            }
        }

        fn ident_with_ref_or_mut(id: &str, is_ref: bool) -> Ident {
            let ext = if is_ref { "_ref" } else { "_mut" };
            Ident::new(&format!("{}{}", id, ext), Span::call_site())
        }

        fn ident_with_mut(id: &str, is_ref: bool) -> Ident {
            if is_ref {
                Ident::new(id, Span::call_site())
            } else {
                Ident::new(&format!("{}_mut", id), Span::call_site())
            }
        }

        fn compute_label(
            opcode_name: &str,
            imm_name: &Ident,
            imm_ty: &ImmType,
            is_ref: bool,
        ) -> TokenStream {
            match imm_ty {
                ImmType::BA => {
                    let call = ident_with_ref_or_mut("from", is_ref);
                    quote!(std::slice::#call(#imm_name))
                }
                ImmType::BA2 => quote!(#imm_name),
                ImmType::BLA => {
                    let call = ident_with_ref_or_mut("as", is_ref);
                    quote!(#imm_name.#call())
                }
                ImmType::FCA => {
                    let call = ident_with_mut("targets", is_ref);
                    quote!(#imm_name.#call())
                }
                _ => todo!("unhandled {:?} for {:?}", imm_ty, opcode_name),
            }
        }

        if let Some(idx) = opcode
            .immediates
            .iter()
            .position(|(_, imm_ty)| is_label_type(imm_ty))
        {
            // Label opcodes.
            let mut match_parts: Punctuated<TokenStream, token::Comma> = Punctuated::new();
            let mut result = None;
            for (i, (imm_name, imm_ty)) in opcode.immediates.iter().enumerate() {
                let imm_name = Ident::new(imm_name, Span::call_site());
                if i == idx {
                    match_parts.push(imm_name.to_token_stream());
                    let result_ref = compute_label(opcode.name, &imm_name, imm_ty, true);
                    let result_mut = compute_label(opcode.name, &imm_name, imm_ty, false);
                    let old = result.replace((result_ref, result_mut));
                    if old.is_some() {
                        panic!("Unable to build targets for opcode with multiple labels");
                    }
                } else if is_struct {
                    match_parts.push(quote!(#imm_name: _));
                } else {
                    match_parts.push(quote!(_));
                }
            }

            let (result_ref, result_mut) = result.unwrap();

            if is_struct {
                with_targets_ref.push(quote!(#variant_name { #match_parts } => #result_ref, ));
                with_targets_mut.push(quote!(#variant_name { #match_parts } => #result_mut, ));
            } else {
                with_targets_ref.push(quote!(#variant_name ( #match_parts ) => #result_ref, ));
                with_targets_mut.push(quote!(#variant_name ( #match_parts ) => #result_mut, ));
            }
        } else {
            // Non-label opcodes.
            if opcode.immediates.is_empty() {
                without_targets.push(quote!(#variant_name));
            } else if is_struct {
                without_targets.push(quote!(#variant_name { .. }));
            } else {
                without_targets.push(quote!(#variant_name ( .. )));
            }
        }
    }

    Ok(
        quote!(impl #impl_generics Targets for #name #impl_types #impl_where {
            fn targets(&self) -> &[Label] {
                match self {
                    #(#with_targets_ref)*
                    #without_targets => &[],
                }
            }

            fn targets_mut(&mut self) -> &mut [Label] {
                match self {
                    #(#with_targets_mut)*
                    #without_targets => &mut [],
                }
            }
        }),
    )
}

fn convert_imm_type(imm: &ImmType, lifetime: &Lifetime) -> TokenStream {
    match imm {
        ImmType::AA => quote!(AdataId<#lifetime>),
        ImmType::ARR(sub) => {
            let sub_ty = convert_imm_type(sub, lifetime);
            quote!(BumpSliceMut<#lifetime, #sub_ty>)
        }
        ImmType::BA => quote!(Label),
        ImmType::BA2 => quote!([Label; 2]),
        ImmType::BLA => quote!(BumpSliceMut<#lifetime, Label>),
        ImmType::DA => quote!(f64),
        ImmType::FCA => quote!(FcallArgs<#lifetime>),
        ImmType::I64A => quote!(i64),
        ImmType::IA => quote!(IterId),
        ImmType::ILA => quote!(Local<#lifetime>),
        ImmType::ITA => quote!(IterArgs<#lifetime>),
        ImmType::IVA => quote!(u32),
        ImmType::KA => quote!(MemberKey<#lifetime>),
        ImmType::LA => quote!(Local<#lifetime>),
        ImmType::LAR => quote!(LocalRange),
        ImmType::NA => panic!("NA is not expected"),
        ImmType::NLA => quote!(Local<#lifetime>),
        ImmType::OA(ty) => {
            let ty = Ident::new(ty, Span::call_site());
            quote!(#ty)
        }
        ImmType::OAL(ty) => {
            let ty = Ident::new(ty, Span::call_site());
            quote!(#ty<#lifetime>)
        }
        ImmType::RATA => quote!(RepoAuthType<#lifetime>),
        ImmType::SA => quote!(Str<#lifetime>),
        ImmType::SLA => quote!(BumpSliceMut<#lifetime, SwitchLabel>),
        ImmType::VSA => quote!(Slice<#lifetime, Str<#lifetime>>),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use hhbc as _;
    use macro_test_util::assert_pat_eq;
    use quote::quote;

    #[test]
    fn test_basic() {
        assert_pat_eq(
            emit_opcodes(
                quote!(
                    enum MyOps<'a> {}
                ),
                &opcode_test_data::test_opcodes(),
            ),
            quote!(
                enum MyOps<'a> {
                    TestZeroImm,
                    TestOneImm(Str<'a>),
                    TestTwoImm(Str<'a>, Str<'a>),
                    TestThreeImm(Str<'a>, Str<'a>, Str<'a>),
                    // --------------------
                    TestAsStruct { str1: Str<'a>, str2: Str<'a> },
                    // --------------------
                    TestAA(AdataId<'a>),
                    TestARR(BumpSliceMut<'a, Str<'a>>),
                    TestBA(Label),
                    TestBA2([Label; 2]),
                    TestBLA(BumpSliceMut<'a, Label>),
                    TestDA(f64),
                    TestFCA(FcallArgs<'a>),
                    TestI64A(i64),
                    TestIA(IterId),
                    TestILA(Local<'a>),
                    TestITA(IterArgs<'a>),
                    TestIVA(u32),
                    TestKA(MemberKey<'a>),
                    TestLA(Local<'a>),
                    TestLAR(LocalRange),
                    TestNLA(Local<'a>),
                    TestOA(OaSubType),
                    TestOAL(OaSubType<'a>),
                    TestRATA(RepoAuthType<'a>),
                    TestSA(Str<'a>),
                    TestSLA(BumpSliceMut<'a, SwitchLabel>),
                    TestVSA(Slice<'a, Str<'a>>),
                }
            ),
        );
    }
}
