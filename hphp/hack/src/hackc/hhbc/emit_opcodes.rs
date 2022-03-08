// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::{ImmType, InstrFlags, OpcodeData};
use proc_macro2::{Ident, Punct, Spacing, Span, TokenStream};
use quote::quote;
use syn::{ItemEnum, Lifetime, Result, Variant};

pub fn emit_opcodes(input: TokenStream, opcodes: &[OpcodeData]) -> Result<TokenStream> {
    let mut input = syn::parse2::<ItemEnum>(input)?;
    let generics = &input.generics;

    let lifetime = &generics.lifetimes().next().unwrap().lifetime;

    for opcode in opcodes {
        let name = Ident::new(opcode.name, Span::call_site());

        let mut body = Vec::new();
        body.extend(quote!(#name));

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
        input.variants.push(variant);
    }

    // Silliness to match rustfmt...
    if !input.variants.trailing_punct() {
        input.variants.push_punct(Default::default());
    }

    Ok(quote!(#input))
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
