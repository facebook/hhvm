// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc::{ImmType, InstrFlags, OpcodeData};
use proc_macro2::{Ident, Span, TokenStream};
use quote::quote;
use syn::{DeriveInput, Result};

// ----------------------------------------------------------------------------

/// Emit a formatter to print opcodes.
///
/// Given input that looks like this:
/// ```
/// #[derive(macros::OpcodeDisplay)]
/// struct PrintMe(Opcodes);
/// ```
///
/// This will add an impl for Display that looks something like this:
///
/// ```
/// impl<T> std::fmt::Display for PrintMe<T> {
///     fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
///         match self.0 {
///             PrintMe::Jmp(target1) => {
///                 f.write_str("Jmp ")?;
///                 print_label(f, target1)?;
///             }
///         }
///     }
/// }
/// ```
///
/// See print_opcode_derive::tests::test_basic() below for a more detailed
/// example output.
///
#[cfg(not(test))]
#[proc_macro_derive(OpcodeDisplay)]
pub fn opcode_display_macro(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    match opcode_display(input.into(), hhbc::opcode_data()) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

fn opcode_display(input: TokenStream, opcodes: &[OpcodeData]) -> Result<TokenStream> {
    let input = syn::parse2::<DeriveInput>(input)?;

    let enum_name = input.ident;

    let mut body = Vec::new();
    for opcode in opcodes {
        // Op1 { some, parameters } => write!("op1 {} {}", some, parameters),
        let variant_name = Ident::new(opcode.name, Span::call_site());

        let symbol = {
            let mut name = opcode.name.to_string();
            if !opcode.immediates.is_empty() {
                name.push(' ');
            }
            name
        };

        let is_struct = opcode.flags.contains(InstrFlags::AS_STRUCT);

        let mut parameters: Vec<Ident> = Vec::new();
        let mut immediates = Vec::new();

        for (i, (name, imm)) in opcode.immediates.iter().enumerate() {
            if i != 0 {
                immediates.push(quote!(f.write_str(" ")?;));
            }
            let param = Ident::new(name, Span::call_site());
            let immediate = convert_immediate(&param, imm);
            parameters.push(param);
            immediates.push(immediate);
        }

        let parameters = if parameters.is_empty() {
            TokenStream::new()
        } else if is_struct {
            quote!( {#(#parameters),*} )
        } else {
            quote!( (#(#parameters),*) )
        };


        body.push(quote!(
            #enum_name::#variant_name #parameters => {
                f.write_str(#symbol)?;
                #(#immediates)*
            }
        ));
    }

    let (impl_generics, ty_generics, where_clause) = input.generics.split_for_impl();

    let output = quote!(
        impl #impl_generics std::fmt::Display for #enum_name #ty_generics #where_clause {
            fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                match self.0 {
                    #(#body)*
                }
                Ok(())
            }
        }
    );

    Ok(output)
}

fn convert_immediate(name: &Ident, imm: &ImmType) -> TokenStream {
    match imm {
        ImmType::AA => quote!(print_adata_id(f, #name)?;),
        ImmType::ARR(_sub_ty) => {
            let msg = format!("unsupported '{}'", name);
            quote!(todo!(#msg);)
        }
        ImmType::BA => quote!(print_label(f, #name)?;),
        ImmType::BA2 => quote!(print_label2(f, #name)?;),
        ImmType::BLA => quote!(print_branch_labels(f, #name)?;),
        ImmType::DA => quote!(print_bytes(f, #name.as_bytes())?;),
        ImmType::FCA => quote!(print_fcall_args(f, #name)?;),
        ImmType::I64A => quote!(#name.fmt(f)?;),
        ImmType::IA => quote!(print_iterator_id(f, #name)?;),
        ImmType::ILA => quote!(print_local(f, #name)?;),
        ImmType::ITA => quote!(print_iter_args(f, #name)?;),
        ImmType::IVA => quote!(#name.fmt(f)?;),
        ImmType::KA => quote!(print_member_key(f, #name)?;),
        ImmType::LA => quote!(print_local(f, #name)?;),
        ImmType::LAR => quote!(print_local_range(f, #name)?;),
        ImmType::NA => panic!("NA is not expected"),
        ImmType::NLA => quote!(print_local(f, #name)?;),
        ImmType::OA(ty) | ImmType::OAL(ty) => {
            use convert_case::{Case, Casing};
            let handler = Ident::new(
                &format!("print_{}", ty.to_case(Case::Snake)),
                Span::call_site(),
            );
            quote!(#handler(f, #name)?;)
        }
        ImmType::RATA => quote!(#name.fmt(f)?;),
        ImmType::SA => quote!(print_bytes(f, #name.as_bytes())?;),
        ImmType::SLA => quote!(print_switch_labels(f, #name)?;),
        ImmType::VSA => quote!(print_shape_fields(f, #name)?;),
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
            opcode_display(
                quote!(
                    #[derive(macros::OpcodeDisplay)]
                    struct PrintMe<T>(Opcodes);
                ),
                &opcode_test_data::test_opcodes(),
            ),
            quote!(
                impl<T> std::fmt::Display for PrintMe<T> {
                    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
                        match self.0 {
                            PrintMe::TestZeroImm => {
                                f.write_str("TestZeroImm")?;
                            }
                            PrintMe::TestOneImm(str1) => {
                                f.write_str("TestOneImm ")?;
                                print_bytes(f, str1.as_bytes())?;
                            }
                            PrintMe::TestTwoImm(str1, str2) => {
                                f.write_str("TestTwoImm ")?;
                                print_bytes(f, str1.as_bytes())?;
                                f.write_str(" ")?;
                                print_bytes(f, str2.as_bytes())?;
                            }
                            PrintMe::TestThreeImm(str1, str2, str3) => {
                                f.write_str("TestThreeImm ")?;
                                print_bytes(f, str1.as_bytes())?;
                                f.write_str(" ")?;
                                print_bytes(f, str2.as_bytes())?;
                                f.write_str(" ")?;
                                print_bytes(f, str3.as_bytes())?;
                            }
                            // -------------------------------------------
                            PrintMe::TestAsStruct { str1, str2 } => {
                                f.write_str("TestAsStruct ")?;
                                print_bytes(f, str1.as_bytes())?;
                                f.write_str(" ")?;
                                print_bytes(f, str2.as_bytes())?;
                            }
                            // -------------------------------------------
                            PrintMe::TestAA(arr1) => {
                                f.write_str("TestAA ")?;
                                print_adata_id(f, arr1)?;
                            }
                            PrintMe::TestARR(arr1) => {
                                f.write_str("TestARR ")?;
                                todo!("unsupported 'arr1'");
                            }
                            PrintMe::TestBA(target1) => {
                                f.write_str("TestBA ")?;
                                print_label(f, target1)?;
                            }
                            PrintMe::TestBA2(target1) => {
                                f.write_str("TestBA2 ")?;
                                print_label2(f, target1)?;
                            }
                            PrintMe::TestBLA(targets) => {
                                f.write_str("TestBLA ")?;
                                print_branch_labels(f, targets)?;
                            }
                            PrintMe::TestDA(dbl1) => {
                                f.write_str("TestDA ")?;
                                print_bytes(f, dbl1.as_bytes())?;
                            }
                            PrintMe::TestFCA(fca) => {
                                f.write_str("TestFCA ")?;
                                print_fcall_args(f, fca)?;
                            }
                            PrintMe::TestI64A(arg1) => {
                                f.write_str("TestI64A ")?;
                                arg1.fmt(f)?;
                            }
                            PrintMe::TestIA(iter1) => {
                                f.write_str("TestIA ")?;
                                print_iterator_id(f, iter1)?;
                            }
                            PrintMe::TestILA(loc1) => {
                                f.write_str("TestILA ")?;
                                print_local(f, loc1)?;
                            }
                            PrintMe::TestITA(ita) => {
                                f.write_str("TestITA ")?;
                                print_iter_args(f, ita)?;
                            }
                            PrintMe::TestIVA(arg1) => {
                                f.write_str("TestIVA ")?;
                                arg1.fmt(f)?;
                            }
                            PrintMe::TestKA(mkey) => {
                                f.write_str("TestKA ")?;
                                print_member_key(f, mkey)?;
                            }
                            PrintMe::TestLA(loc1) => {
                                f.write_str("TestLA ")?;
                                print_local(f, loc1)?;
                            }
                            PrintMe::TestLAR(locrange) => {
                                f.write_str("TestLAR ")?;
                                print_local_range(f, locrange)?;
                            }
                            PrintMe::TestNLA(nloc1) => {
                                f.write_str("TestNLA ")?;
                                print_local(f, nloc1)?;
                            }
                            PrintMe::TestOA(subop1) => {
                                f.write_str("TestOA ")?;
                                print_oa_sub_type(f, subop1)?;
                            }
                            PrintMe::TestOAL(subop1) => {
                                f.write_str("TestOAL ")?;
                                print_oa_sub_type(f, subop1)?;
                            }
                            PrintMe::TestRATA(rat) => {
                                f.write_str("TestRATA ")?;
                                rat.fmt(f)?;
                            }
                            PrintMe::TestSA(str1) => {
                                f.write_str("TestSA ")?;
                                print_bytes(f, str1.as_bytes())?;
                            }
                            PrintMe::TestSLA(targets) => {
                                f.write_str("TestSLA ")?;
                                print_switch_labels(f, targets)?;
                            }
                            PrintMe::TestVSA(keys) => {
                                f.write_str("TestVSA ")?;
                                print_shape_fields(f, keys)?;
                            }
                        }
                        Ok(())
                    }
                }
            ),
        );
    }
}
