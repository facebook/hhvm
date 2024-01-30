// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_gen::ImmType;
use hhbc_gen::Inputs;
use hhbc_gen::InstrFlags;
use hhbc_gen::OpcodeData;
use hhbc_gen::Outputs;
use proc_macro2::Ident;
use proc_macro2::Punct;
use proc_macro2::Spacing;
use proc_macro2::Span;
use proc_macro2::TokenStream;
use quote::quote;
use quote::ToTokens;
use syn::punctuated::Punctuated;
use syn::token;
use syn::ItemEnum;
use syn::Lifetime;
use syn::Result;
use syn::Variant;

pub fn emit_opcodes(input: TokenStream, opcodes: &[OpcodeData]) -> Result<TokenStream> {
    let mut enum_def = syn::parse2::<ItemEnum>(input)?;
    let generics = &enum_def.generics;

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
        enum_def.variants.push(variant);
    }

    // Silliness to match rustfmt...
    if !enum_def.variants.trailing_punct() {
        enum_def.variants.push_punct(Default::default());
    }

    let (impl_generics, impl_types, impl_where) = enum_def.generics.split_for_impl();

    let enum_impl = {
        let enum_name = &enum_def.ident;
        let mut variant_names_matches = Vec::new();
        let mut variant_index_matches = Vec::new();
        let mut num_inputs_matches = Vec::new();
        let mut num_outputs_matches = Vec::new();
        for (idx, opcode) in opcodes.iter().enumerate() {
            let name = Ident::new(opcode.name, Span::call_site());
            let name_str = &opcode.name;
            let is_struct = opcode.flags.contains(InstrFlags::AS_STRUCT);
            let ignore_args = if opcode.immediates.is_empty() {
                Default::default()
            } else if is_struct {
                quote!({ .. })
            } else {
                quote!((..))
            };
            variant_names_matches.push(quote!(#enum_name :: #name #ignore_args => #name_str,));
            variant_index_matches.push(quote!(#enum_name :: #name #ignore_args => #idx,));

            let num_inputs = match opcode.inputs {
                Inputs::NOV => quote!(#enum_name :: #name #ignore_args => 0),
                Inputs::Fixed(ref inputs) => {
                    let n = inputs.len();
                    quote!(#enum_name :: #name #ignore_args => #n)
                }
                Inputs::FCall { inp, .. } => {
                    quote!(#enum_name :: #name (fca, ..) => NUM_ACT_REC_CELLS + fca.num_inputs() + fca.num_inouts() + #inp as usize)
                }
                Inputs::CMany | Inputs::CUMany => {
                    quote!(#enum_name :: #name (n, ..) => *n as usize)
                }
                Inputs::SMany => {
                    quote!(#enum_name :: #name (n, ..) => n.len())
                }
                Inputs::MFinal => {
                    quote!(#enum_name :: #name (n, ..) => *n as usize)
                }
                Inputs::CMFinal(m) => {
                    quote!(#enum_name :: #name (n, ..) => *n as usize + #m as usize)
                }
            };
            num_inputs_matches.push(num_inputs);

            let num_outputs = match opcode.outputs {
                Outputs::NOV => quote!(#enum_name :: #name #ignore_args => 0),
                Outputs::Fixed(ref outputs) => {
                    let n = outputs.len();
                    quote!(#enum_name :: #name #ignore_args => #n)
                }
                Outputs::FCall => quote!(#enum_name :: #name #ignore_args => 0),
            };
            num_outputs_matches.push(num_outputs);
        }
        quote!(
            impl #impl_generics #enum_name #impl_types #impl_where {
                pub fn variant_name(&self) -> &'static str {
                    match self {
                        #(#variant_names_matches)*
                    }
                }

                pub fn variant_index(&self) -> usize {
                    match self {
                        #(#variant_index_matches)*
                    }
                }

                pub fn num_inputs(&self) -> usize {
                    match self {
                        #(#num_inputs_matches),*,
                    }
                }

                pub fn num_outputs(&self) -> usize {
                    match self {
                        #(#num_outputs_matches),*,
                    }
                }
            }
        )
    };

    Ok(quote!(
        #enum_def
        #enum_impl
    ))
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
                | ImmType::DUMMY
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

        #[allow(clippy::todo)]
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
                    let old = result.replace(result_ref);
                    if old.is_some() {
                        panic!("Unable to build targets for opcode with multiple labels");
                    }
                } else if is_struct {
                    match_parts.push(quote!(#imm_name: _));
                } else {
                    match_parts.push(quote!(_));
                }
            }

            let result_ref = result.unwrap();

            if is_struct {
                with_targets_ref.push(quote!(#variant_name { #match_parts } => #result_ref, ));
            } else {
                with_targets_ref.push(quote!(#variant_name ( #match_parts ) => #result_ref, ));
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
        }),
    )
}

fn convert_imm_type(imm: &ImmType, lifetime: &Lifetime) -> TokenStream {
    match imm {
        ImmType::AA => quote!(AdataId<#lifetime>),
        ImmType::ARR(sub) => {
            let sub_ty = convert_imm_type(sub, lifetime);
            quote!(Vector<#sub_ty>)
        }
        ImmType::BA => quote!(Label),
        ImmType::BA2 => quote!([Label; 2]),
        ImmType::BLA => quote!(Vector<Label>),
        ImmType::DA => quote!(FloatBits),
        ImmType::DUMMY => quote!(Dummy),
        ImmType::FCA => quote!(FCallArgs<#lifetime>),
        ImmType::I64A => quote!(i64),
        ImmType::IA => quote!(IterId),
        ImmType::ILA => quote!(Local),
        ImmType::ITA => quote!(IterArgs),
        ImmType::IVA => quote!(u32),
        ImmType::KA => quote!(MemberKey<#lifetime>),
        ImmType::LA => quote!(Local),
        ImmType::LAR => quote!(LocalRange),
        ImmType::NA => panic!("NA is not expected"),
        ImmType::NLA => quote!(Local),
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
        ImmType::SLA => quote!(Vector<SwitchLabel>),
        ImmType::VSA => quote!(Vector<Str<#lifetime>>),
    }
}

/// Build construction helpers for InstrSeq.  Each line in the input is either
///   (a) a list of opcodes and the keyword 'default':
///           `A | B => default`
///       which means to generate helpers for those opcodes with default snake-case names
///   (b) a single opcode and a single name:
///           `A => my_a`
///       which means to generate a helper for that opcode with the given name.
///   (c) a list of opcodes and an empty block:
///           `A | B => {}`
///       which means to skip generating helpers for those opcodes.
///
/// The parameters to the function are based on the expected immediates for the
/// opcode.
///
///     define_instr_seq_helpers! {
///       MyA | MyB => default
///       MyC => myc
///       MyD => {}
///     }
///
/// Expands into:
///
///     pub fn my_a<'a>() -> InstrSeq<'a> {
///         instr(Instruct::Opcode(Opcode::MyA))
///     }
///
///     pub fn my_b<'a>(arg1: i64) -> InstrSeq<'a> {
///         instr(Instruct::Opcode(Opcode::MyB(arg1)))
///     }
///
///     pub fn myc<'a>(arg1: i64, arg2: i64) -> InstrSeq<'a> {
///         instr(Instruct::Opcode(Opcode::MyC(arg1, arg2)))
///     }
pub fn define_instr_seq_helpers(input: TokenStream, opcodes: &[OpcodeData]) -> Result<TokenStream> {
    // Foo => bar
    // Foo | Bar | Baz => default
    // Foo | Bar | Baz => {}

    use std::collections::HashMap;

    use convert_case::Case;
    use convert_case::Casing;
    use proc_macro2::TokenTree;
    use syn::parse::ParseStream;
    use syn::parse::Parser;
    use syn::Error;
    use syn::Token;

    #[derive(Debug)]
    struct Helper<'a> {
        opcode_name: Ident,
        fn_name: Ident,
        opcode_data: &'a OpcodeData,
    }

    let mut opcodes: HashMap<&str, &OpcodeData> =
        opcodes.iter().map(|data| (data.name, data)).collect();

    // Foo => bar
    // Foo | Bar | Baz => default
    // Foo | Bar | Baz => {}

    let macro_input = |input: ParseStream<'_>| -> Result<Vec<Helper<'_>>> {
        let mut helpers: Vec<Helper<'_>> = Vec::new();

        let mut same_as_default: Vec<Ident> = Vec::new();

        while !input.is_empty() {
            let mut opcode_names: Vec<(Ident, &OpcodeData)> = {
                let names =
                    syn::punctuated::Punctuated::<Ident, Token![|]>::parse_separated_nonempty(
                        input,
                    )?;

                let mut opcode_names: Vec<(Ident, &OpcodeData)> = Vec::new();
                for name in names {
                    if let Some(opcode_data) = opcodes.remove(name.to_string().as_str()) {
                        opcode_names.push((name, opcode_data));
                    } else {
                        return Err(Error::new(
                            name.span(),
                            format!("Unknown opcode '{}'", name),
                        ));
                    }
                }

                opcode_names
            };

            let _arrow: Token![=>] = input.parse()?;

            let tt: TokenTree = input.parse()?;
            match tt {
                TokenTree::Ident(fn_name) => {
                    // Foo => bar

                    let (opcode_name, opcode_data) = opcode_names.pop().unwrap();
                    if let Some((unexpected, _)) = opcode_names.pop() {
                        return Err(Error::new(
                            unexpected.span(),
                            format!(
                                "Only a single Opcode is allowed when a function name ('{}') is provided",
                                fn_name
                            ),
                        ));
                    }

                    // If the 'alias' name is what we would have picked as a default
                    // then complain.
                    let default_name = opcode_data.name.to_case(Case::Snake);
                    if fn_name == default_name {
                        same_as_default.push(opcode_name.clone());
                    }

                    helpers.push(Helper {
                        opcode_name,
                        fn_name,
                        opcode_data,
                    });

                    if input.is_empty() {
                        break;
                    }
                    input.parse::<Token![,]>()?;
                }
                TokenTree::Group(grp) => {
                    // Foo | Bar | Baz => {}

                    let stream = grp.stream();
                    let mut iter = stream.into_iter();
                    if let Some(tt) = iter.next() {
                        return Err(Error::new(tt.span(), "Block must be empty"));
                    }

                    // do nothing

                    // allow an unnecessary comma
                    if input.peek(Token![,]) {
                        input.parse::<Token![,]>()?;
                    }
                }
                TokenTree::Punct(_) | TokenTree::Literal(_) => {
                    return Err(Error::new(tt.span(), "identifier or '{}' expected"));
                }
            }
        }

        if let Some(opcode_name) = same_as_default.pop() {
            let span = opcode_name.span();
            let msg = if same_as_default.is_empty() {
                format!(
                    "Opcode '{}' was given an alias which is the same name it would have gotten with default - please omit it instead.",
                    opcode_name,
                )
            } else {
                format!(
                    "Opcodes {} were given aliases which are the same name they would have gotten with default - please omit it instead.",
                    std::iter::once(opcode_name)
                        .chain(same_as_default.into_iter())
                        .map(|s| format!("'{}'", s))
                        .collect::<Vec<_>>()
                        .join(", ")
                )
            };

            return Err(Error::new(span, msg));
        }

        for opcode_data in opcodes.values() {
            let opcode_name = Ident::new(opcode_data.name, Span::call_site());
            let fn_name = Ident::new(&opcode_data.name.to_case(Case::Snake), opcode_name.span());
            helpers.push(Helper {
                opcode_name,
                fn_name,
                opcode_data,
            });
        }

        helpers.sort_by(|a, b| a.fn_name.cmp(&b.fn_name));
        Ok(helpers)
    };

    let input: Vec<Helper<'_>> = macro_input.parse2(input)?;

    let vis = quote!(pub);
    let enum_name = quote!(Opcode);
    let lifetime: Lifetime = syn::parse2::<Lifetime>(quote!('a))?;

    let mut res: Vec<TokenStream> = Vec::new();
    for Helper {
        opcode_name,
        fn_name,
        opcode_data,
    } in input
    {
        let params: Vec<TokenStream> = opcode_data
            .immediates
            .iter()
            .map(|(imm_name, imm_ty)| {
                let imm_name = Ident::new(imm_name, Span::call_site());
                let ty = convert_imm_type(imm_ty, &lifetime);
                quote!(#imm_name: #ty)
            })
            .collect();

        let args = {
            let args: Vec<TokenStream> = opcode_data
                .immediates
                .iter()
                .map(|(imm_name, _)| {
                    let imm_name = Ident::new(imm_name, Span::call_site());
                    quote!(#imm_name)
                })
                .collect();
            if args.is_empty() {
                quote!()
            } else {
                quote!(( #(#args),* ))
            }
        };

        let func = quote!(
            #vis fn #fn_name<#lifetime>(#(#params),*) -> InstrSeq<#lifetime> {
                instr(Instruct::#enum_name(#enum_name::#opcode_name #args))
            }
        );

        res.push(func);
    }

    Ok(quote!(#(#res)*))
}

#[cfg(test)]
mod tests {
    use hhbc_gen as _;
    use macro_test_util::assert_pat_eq;
    use quote::quote;

    use super::*;

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
                    TestARR(Vector<Str<'a>>),
                    TestBA(Label),
                    TestBA2([Label; 2]),
                    TestBLA(Vector<Label>),
                    TestDA(FloatBits),
                    TestFCA(FCallArgs<'a>),
                    TestI64A(i64),
                    TestIA(IterId),
                    TestILA(Local),
                    TestITA(IterArgs),
                    TestIVA(u32),
                    TestKA(MemberKey<'a>),
                    TestLA(Local),
                    TestLAR(LocalRange),
                    TestNLA(Local),
                    TestOA(OaSubType),
                    TestOAL(OaSubType<'a>),
                    TestRATA(RepoAuthType<'a>),
                    TestSA(Str<'a>),
                    TestSLA(Vector<SwitchLabel>),
                    TestVSA(Vector<Str<'a>>),
                }
                impl<'a> MyOps<'a> {
                    pub fn variant_name(&self) -> &'static str {
                        match self {
                            MyOps::TestZeroImm => "TestZeroImm",
                            MyOps::TestOneImm(..) => "TestOneImm",
                            MyOps::TestTwoImm(..) => "TestTwoImm",
                            MyOps::TestThreeImm(..) => "TestThreeImm",
                            MyOps::TestAsStruct { .. } => "TestAsStruct",
                            MyOps::TestAA(..) => "TestAA",
                            MyOps::TestARR(..) => "TestARR",
                            MyOps::TestBA(..) => "TestBA",
                            MyOps::TestBA2(..) => "TestBA2",
                            MyOps::TestBLA(..) => "TestBLA",
                            MyOps::TestDA(..) => "TestDA",
                            MyOps::TestFCA(..) => "TestFCA",
                            MyOps::TestI64A(..) => "TestI64A",
                            MyOps::TestIA(..) => "TestIA",
                            MyOps::TestILA(..) => "TestILA",
                            MyOps::TestITA(..) => "TestITA",
                            MyOps::TestIVA(..) => "TestIVA",
                            MyOps::TestKA(..) => "TestKA",
                            MyOps::TestLA(..) => "TestLA",
                            MyOps::TestLAR(..) => "TestLAR",
                            MyOps::TestNLA(..) => "TestNLA",
                            MyOps::TestOA(..) => "TestOA",
                            MyOps::TestOAL(..) => "TestOAL",
                            MyOps::TestRATA(..) => "TestRATA",
                            MyOps::TestSA(..) => "TestSA",
                            MyOps::TestSLA(..) => "TestSLA",
                            MyOps::TestVSA(..) => "TestVSA",
                        }
                    }
                    pub fn variant_index(&self) -> usize {
                        match self {
                            MyOps::TestZeroImm => 0usize,
                            MyOps::TestOneImm(..) => 1usize,
                            MyOps::TestTwoImm(..) => 2usize,
                            MyOps::TestThreeImm(..) => 3usize,
                            MyOps::TestAsStruct { .. } => 4usize,
                            MyOps::TestAA(..) => 5usize,
                            MyOps::TestARR(..) => 6usize,
                            MyOps::TestBA(..) => 7usize,
                            MyOps::TestBA2(..) => 8usize,
                            MyOps::TestBLA(..) => 9usize,
                            MyOps::TestDA(..) => 10usize,
                            MyOps::TestFCA(..) => 11usize,
                            MyOps::TestI64A(..) => 12usize,
                            MyOps::TestIA(..) => 13usize,
                            MyOps::TestILA(..) => 14usize,
                            MyOps::TestITA(..) => 15usize,
                            MyOps::TestIVA(..) => 16usize,
                            MyOps::TestKA(..) => 17usize,
                            MyOps::TestLA(..) => 18usize,
                            MyOps::TestLAR(..) => 19usize,
                            MyOps::TestNLA(..) => 20usize,
                            MyOps::TestOA(..) => 21usize,
                            MyOps::TestOAL(..) => 22usize,
                            MyOps::TestRATA(..) => 23usize,
                            MyOps::TestSA(..) => 24usize,
                            MyOps::TestSLA(..) => 25usize,
                            MyOps::TestVSA(..) => 26usize,
                        }
                    }
                    pub fn num_inputs(&self) -> usize {
                        match self {
                            MyOps::TestZeroImm => 0,
                            MyOps::TestOneImm(..) => 0,
                            MyOps::TestTwoImm(..) => 0,
                            MyOps::TestThreeImm(..) => 0,
                            MyOps::TestAsStruct { .. } => 0,
                            MyOps::TestAA(..) => 0,
                            MyOps::TestARR(..) => 0,
                            MyOps::TestBA(..) => 0,
                            MyOps::TestBA2(..) => 0,
                            MyOps::TestBLA(..) => 0,
                            MyOps::TestDA(..) => 0,
                            MyOps::TestFCA(..) => 0,
                            MyOps::TestI64A(..) => 0,
                            MyOps::TestIA(..) => 0,
                            MyOps::TestILA(..) => 0,
                            MyOps::TestITA(..) => 0,
                            MyOps::TestIVA(..) => 0,
                            MyOps::TestKA(..) => 0,
                            MyOps::TestLA(..) => 0,
                            MyOps::TestLAR(..) => 0,
                            MyOps::TestNLA(..) => 0,
                            MyOps::TestOA(..) => 0,
                            MyOps::TestOAL(..) => 0,
                            MyOps::TestRATA(..) => 0,
                            MyOps::TestSA(..) => 0,
                            MyOps::TestSLA(..) => 0,
                            MyOps::TestVSA(..) => 0,
                        }
                    }
                    pub fn num_outputs(&self) -> usize {
                        match self {
                            MyOps::TestZeroImm => 0,
                            MyOps::TestOneImm(..) => 0,
                            MyOps::TestTwoImm(..) => 0,
                            MyOps::TestThreeImm(..) => 0,
                            MyOps::TestAsStruct { .. } => 0,
                            MyOps::TestAA(..) => 0,
                            MyOps::TestARR(..) => 0,
                            MyOps::TestBA(..) => 0,
                            MyOps::TestBA2(..) => 0,
                            MyOps::TestBLA(..) => 0,
                            MyOps::TestDA(..) => 0,
                            MyOps::TestFCA(..) => 0,
                            MyOps::TestI64A(..) => 0,
                            MyOps::TestIA(..) => 0,
                            MyOps::TestILA(..) => 0,
                            MyOps::TestITA(..) => 0,
                            MyOps::TestIVA(..) => 0,
                            MyOps::TestKA(..) => 0,
                            MyOps::TestLA(..) => 0,
                            MyOps::TestLAR(..) => 0,
                            MyOps::TestNLA(..) => 0,
                            MyOps::TestOA(..) => 0,
                            MyOps::TestOAL(..) => 0,
                            MyOps::TestRATA(..) => 0,
                            MyOps::TestSA(..) => 0,
                            MyOps::TestSLA(..) => 0,
                            MyOps::TestVSA(..) => 0,
                        }
                    }
                }
            ),
        );
    }

    #[test]
    fn test_instr_seq() {
        assert_pat_eq(
            define_instr_seq_helpers(
                quote!(
                    TestOneImm => test_oneimm,
                    TestTwoImm => test_twoimm,
                    TestBLA | TestFCA | TestARR | TestDA | TestBA2 | TestBA |
                    TestIA | TestITA | TestNLA | TestOAL | TestLA | TestRATA |
                    TestSLA | TestILA | TestIVA | TestKA | TestI64A | TestSA |
                    TestOA | TestVSA | TestThreeImm => {}
                ),
                &opcode_test_data::test_opcodes(),
            ),
            quote!(
                pub fn test_aa<'a>(arr1: AdataId<'a>) -> InstrSeq<'a> {
                    instr(Instruct::Opcode(Opcode::TestAA(arr1)))
                }
                pub fn test_as_struct<'a>(str1: Str<'a>, str2: Str<'a>) -> InstrSeq<'a> {
                    instr(Instruct::Opcode(Opcode::TestAsStruct(str1, str2)))
                }
                pub fn test_lar<'a>(locrange: LocalRange) -> InstrSeq<'a> {
                    instr(Instruct::Opcode(Opcode::TestLAR(locrange)))
                }
                pub fn test_oneimm<'a>(str1: Str<'a>) -> InstrSeq<'a> {
                    instr(Instruct::Opcode(Opcode::TestOneImm(str1)))
                }
                pub fn test_twoimm<'a>(str1: Str<'a>, str2: Str<'a>) -> InstrSeq<'a> {
                    instr(Instruct::Opcode(Opcode::TestTwoImm(str1, str2)))
                }
                pub fn test_zero_imm<'a>() -> InstrSeq<'a> {
                    instr(Instruct::Opcode(Opcode::TestZeroImm))
                }
            ),
        );
    }
}
