// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_gen::OpcodeData;
use itertools::Itertools;
use proc_macro2::Ident;
use proc_macro2::Span;
use proc_macro2::TokenStream;
use quote::quote;
use syn::LitByteStr;
use syn::Result;

#[proc_macro_attribute]
pub fn assemble_opcode(
    _attrs: proc_macro::TokenStream,
    input: proc_macro::TokenStream,
) -> proc_macro::TokenStream {
    match assemble_opcode_impl(input.into(), hhbc_gen::opcode_data()) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

fn assemble_opcode_impl(_input: TokenStream, opcodes: &[OpcodeData]) -> Result<TokenStream> {
    let name = quote!(assemble_opcode);
    let mut body = Vec::new();
    for opcode in opcodes {
        let variant_name = Ident::new(opcode.name, Span::call_site());
        let name = opcode.name.to_string();
        let name_bstr = LitByteStr::new(name.as_bytes(), Span::call_site());

        // SSwitch and MemoGetEage both have unusual semantics and have to be
        // parsed specially in the outer code block.
        if name == "SSwitch" || name == "MemoGetEager" {
            body.push(quote!(#name_bstr => { unreachable!(); }));
            continue;
        }

        let imms = if opcode.immediates.is_empty() {
            quote!()
        } else {
            let imms = opcode
                .immediates
                .iter()
                .map(|_| quote!(token_iter.assemble_imm(alloc, decl_map)?))
                .collect_vec();

            quote!((#(#imms),*))
        };

        body.push(quote!(
            #name_bstr => {
                token_iter.expect_str(Token::is_identifier, #name)?;
                Ok(hhbc::Instruct::Opcode(hhbc::Opcode::#variant_name #imms))
            }
        ));
    }

    Ok(quote!(
        fn #name(
            alloc: &Bump,
            tok: &'_ [u8],
            token_iter: &mut Lexer<'_>,
            decl_map: &StringIdMap<u32>,
        ) -> Result<hhbc::Instruct>{
            match tok {
                #(#body)*
                t => bail!("unknown opcode: {:?}", t),
            }

        }
    ))
}

/// Used like:
///
///   assemble_enum!(lexer, [E::A, E::B, E::C])
///
/// turns into a handler for A, B, and C that looks something like:
///
/// impl AssembleImm<'_, $ret_ty> for Lexer<'_> {
///   fn assemble_imm(&mut self, _alloc: &'_ Bump, _decl_map: &DeclMap) -> Result<$ret_ty> {
///     use $ret_ty;
///     match self.expect(Token::into_identifier)? {
///       b"A" => E::A,
///       b"B" => E::B,
///       b"C" => E::C,
///       _ => bail!(...)
///     }
///   }
/// }
///
/// This needs to be a proc-macro so it can manipulate the names (turning 'E::A'
/// into 'b"A"').
#[proc_macro]
pub fn assemble_imm_for_enum(tokens: proc_macro::TokenStream) -> proc_macro::TokenStream {
    use quote::ToTokens;
    use syn::Path;
    use syn::Token;
    use syn::Type;

    #[derive(Debug)]
    struct Input {
        ret_ty: Type,
        variants: Vec<Path>,
    }

    impl syn::parse::Parse for Input {
        fn parse(input: syn::parse::ParseStream<'_>) -> Result<Self> {
            let ret_ty = input.parse()?;
            input.parse::<Token![,]>()?;
            let variants_stream;
            syn::bracketed!(variants_stream in input);
            input.parse::<syn::parse::Nothing>()?;
            let variants = variants_stream
                .parse_terminated::<Path, Token![,]>(Path::parse)?
                .into_iter()
                .collect_vec();

            Ok(Input { ret_ty, variants })
        }
    }

    let Input { ret_ty, variants } = syn::parse_macro_input!(tokens as Input);

    let mut expected = variants.first().unwrap().clone();
    if expected.segments.len() > 1 {
        expected.segments.pop();
        // Make sure to remove any trailing punctuation.
        let t = expected.segments.pop().unwrap().into_value();
        expected.segments.push(t);
    }

    let body = variants
        .into_iter()
        .map(|variant| {
            let ident = &variant.segments.last().unwrap().ident;
            let ident_str = LitByteStr::new(ident.to_string().as_bytes(), Span::call_site());
            quote!(#ident_str => #variant)
        })
        .collect_vec();

    let msg = format!(
        "Expected a '{}', got {{:?}} on line {{}}",
        expected.into_token_stream()
    );

    // Unfortunately since most of these 'enums' aren't real Rust enums we can't
    // do anything to ensure that this is exhaustive.

    let output = quote! {
        impl AssembleImm<'_, #ret_ty> for Lexer<'_> {
            fn assemble_imm(&mut self, _: &'_ Bump, _: &DeclMap) -> Result<#ret_ty> {
                use #ret_ty;
                let tok = self.expect_token()?;
                let id = tok.into_identifier()?;
                Ok(match id {
                    #(#body),*,
                    f => return Err(tok.error(#msg)),
                })
            }
        }
    };

    output.into()
}
