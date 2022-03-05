// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::{ImmType, OpcodeData};
use proc_macro2::{Ident, Punct, Spacing, Span, TokenStream};
use quote::quote;
use syn::{ItemEnum, Lifetime, Result};

pub fn emit_opcodes(input: TokenStream, opcodes: &[OpcodeData]) -> Result<TokenStream> {
    let input = syn::parse2::<ItemEnum>(input)?;
    let name = input.ident;
    let generics = input.generics;

    let lifetime = &generics.lifetimes().next().unwrap().lifetime;

    let mut body = Vec::new();

    for opcode in opcodes {
        let name = Ident::new(opcode.name, Span::call_site());
        body.extend(quote!(#name));

        if !opcode.immediates.is_empty() {
            let mut imms = Vec::new();
            for (idx, (_name, ty)) in opcode.immediates.iter().enumerate() {
                if idx != 0 {
                    imms.push(Punct::new(',', Spacing::Alone).into());
                }
                imms.extend(convert_imm_type(ty, lifetime));
            }
            body.extend(quote!( ( #(#imms)* ) ));
        }

        body.extend(quote!(,));
    }

    Ok(quote!(
        enum #name #generics {
            #(#body)*
        }
    ))
}

fn convert_imm_type(imm: &ImmType, lifetime: &Lifetime) -> TokenStream {
    match imm {
        ImmType::AA => quote!(AdataId<#lifetime>),
        ImmType::BA => quote!(Label),
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
                    TestAA(AdataId<'a>),
                    TestBA(Label),
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
                    TestRATA(RepoAuthType<'a>),
                    TestSA(Str<'a>),
                    TestSLA(BumpSliceMut<'a, SwitchLabel>),
                    TestVSA(Slice<'a, Str<'a>>),
                }
            ),
        );
    }
}
