// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hash::HashSet;
use hhbc_gen::ImmType;
use hhbc_gen::OpcodeData;
use proc_macro2::Ident;
use proc_macro2::Span;
use proc_macro2::TokenStream;
use quote::quote;
use quote::ToTokens;
use syn::spanned::Spanned;
use syn::DeriveInput;
use syn::Error;
use syn::Lit;
use syn::LitByteStr;
use syn::LitStr;
use syn::Meta;
use syn::MetaList;
use syn::MetaNameValue;
use syn::NestedMeta;
use syn::Result;

// ----------------------------------------------------------------------------

pub fn build_print_opcode(input: TokenStream, opcodes: &[OpcodeData]) -> Result<TokenStream> {
    let input = syn::parse2::<DeriveInput>(input)?;

    let struct_name = &input.ident;

    let attributes = Attributes::from_derive_input(&input)?;

    let mut body = Vec::new();
    for opcode in opcodes {
        // Op1 { some, parameters } => write!("op1 {} {}", some, parameters),
        let variant_name = Ident::new(opcode.name, Span::call_site());

        let symbol = {
            let mut name = opcode.name.to_string();
            if !opcode.immediates.is_empty() {
                name.push(' ');
            }
            LitByteStr::new(name.as_bytes(), Span::call_site())
        };

        let is_override = attributes.overrides.contains(opcode.name);

        let parameters: Vec<Ident> = opcode
            .immediates
            .iter()
            .map(|(name, _)| Ident::new(name, Span::call_site()))
            .collect();

        let input_parameters = if parameters.is_empty() {
            TokenStream::new()
        } else {
            quote!( (#(#parameters),*) )
        };

        if is_override {
            let override_call = {
                use convert_case::Case;
                use convert_case::Casing;
                let name = opcode.name.to_case(Case::Snake);
                Ident::new(&format!("print_{}", name), Span::call_site())
            };

            let call_args: Vec<TokenStream> = opcode
                .immediates
                .iter()
                .map(|(name, imm)| convert_call_arg(name, imm))
                .collect();

            body.push(quote!(
                Opcode::#variant_name #input_parameters => {
                    self.#override_call (w, #(#call_args),* )?;
                }
            ));
        } else {
            let immediates = {
                let mut imms = Vec::new();
                for (name, imm) in opcode.immediates.iter() {
                    imms.push(convert_immediate(name, imm));
                    imms.push(quote!(w.write_all(b" ")?;));
                }
                imms.pop();
                imms
            };

            body.push(quote!(
                Opcode::#variant_name #input_parameters => {
                    w.write_all(#symbol)?;
                    #(#immediates)*
                }
            ));
        }
    }

    let vis = input.vis;
    let (impl_generics, ty_generics, where_clause) = input.generics.split_for_impl();

    let output = quote!(
        impl #impl_generics #struct_name #ty_generics #where_clause {
            #vis fn print_opcode(
                &self,
                w: &mut <Self as PrintOpcodeTypes>::Write
            ) -> std::result::Result<(), <Self as PrintOpcodeTypes>::Error> {
                match self.get_opcode() {
                    #(#body)*
                }
                Ok(())
            }
        }
    );

    Ok(output)
}

trait MetaHelper {
    fn expect_list(self) -> Result<MetaList>;
    fn expect_name_value(self) -> Result<MetaNameValue>;
}

impl MetaHelper for Meta {
    fn expect_list(self) -> Result<MetaList> {
        match self {
            Meta::Path(_) | Meta::NameValue(_) => Err(Error::new(self.span(), "Expected List")),
            Meta::List(list) => Ok(list),
        }
    }

    fn expect_name_value(self) -> Result<MetaNameValue> {
        match self {
            Meta::Path(_) | Meta::List(_) => {
                Err(Error::new(self.span(), "Expected 'Name' = 'Value'"))
            }
            Meta::NameValue(nv) => Ok(nv),
        }
    }
}

trait NestedMetaHelper {
    fn expect_meta(self) -> Result<Meta>;
}

impl NestedMetaHelper for NestedMeta {
    fn expect_meta(self) -> Result<Meta> {
        match self {
            NestedMeta::Lit(lit) => Err(Error::new(lit.span(), "Unexpected literal")),
            NestedMeta::Meta(meta) => Ok(meta),
        }
    }
}

trait LitHelper {
    fn expect_str(self) -> Result<LitStr>;
}

impl LitHelper for Lit {
    fn expect_str(self) -> Result<LitStr> {
        match self {
            Lit::Str(ls) => Ok(ls),
            _ => Err(Error::new(self.span(), "Literal string expected")),
        }
    }
}

struct Attributes {
    overrides: HashSet<String>,
}

impl Attributes {
    fn from_derive_input(input: &DeriveInput) -> Result<Self> {
        let mut result = Attributes {
            overrides: Default::default(),
        };

        for attr in &input.attrs {
            if attr.path.is_ident("print_opcode") {
                let args = attr.parse_meta()?.expect_list()?;

                for nested in args.nested.into_iter() {
                    let meta = nested.expect_meta()?;
                    if meta.path().is_ident("override") {
                        let nv = meta.expect_name_value()?;
                        let ov = nv.lit.expect_str()?;
                        result.overrides.insert(ov.value());
                    } else {
                        return Err(Error::new(
                            meta.span(),
                            format!("Unknown attribute '{:?}'", meta.path()),
                        ));
                    }
                }
            }
        }

        Ok(result)
    }
}

#[allow(clippy::todo)]
fn convert_immediate(name: &str, imm: &ImmType) -> TokenStream {
    let name = Ident::new(name, Span::call_site());
    match imm {
        ImmType::AA => quote!(print_adata_id(w, #name)?;),
        ImmType::ARR(_sub_ty) => {
            let msg = format!("unsupported '{}'", name);
            quote!(todo!(#msg);)
        }
        ImmType::BA => quote!(self.print_label(w, #name)?;),
        ImmType::BA2 => quote!(self.print_label2(w, #name)?;),
        ImmType::BLA => quote!(self.print_branch_labels(w, #name.as_ref())?;),
        ImmType::DA => quote!(print_float(w, *#name)?;),
        ImmType::DUMMY => TokenStream::new(),
        ImmType::FCA => quote!(self.print_fcall_args(w, #name)?;),
        ImmType::I64A => quote!(write!(w, "{}", #name)?;),
        ImmType::IA => quote!(print_iterator_id(w, #name)?;),
        ImmType::ILA => quote!(self.print_local(w, #name)?;),
        ImmType::ITA => quote!(self.print_iter_args(w, #name)?;),
        ImmType::IVA => quote!(write!(w, "{}", #name)?;),
        ImmType::KA => quote!(self.print_member_key(w, #name)?;),
        ImmType::LA => quote!(self.print_local(w, #name)?;),
        ImmType::LAR => quote!(print_local_range(w, #name)?;),
        ImmType::NA => panic!("NA is not expected"),
        ImmType::NLA => quote!(self.print_local(w, #name)?;),
        ImmType::OA(ty) | ImmType::OAL(ty) => {
            use convert_case::Case;
            use convert_case::Casing;
            let handler = Ident::new(
                &format!("print_{}", ty.to_case(Case::Snake)),
                Span::call_site(),
            );
            quote!(#handler(w, #name)?;)
        }
        ImmType::RATA => quote!(print_str(w, #name)?;),
        ImmType::SA => quote!(print_quoted_ffi_str(w, #name)?;),
        ImmType::SLA => quote!(print_switch_labels(w, #name)?;),
        ImmType::VSA => quote!(print_shape_fields(w, #name)?;),
    }
}

fn convert_call_arg(name: &str, imm: &ImmType) -> TokenStream {
    let name = Ident::new(name, Span::call_site());
    match imm {
        ImmType::AA
        | ImmType::BA
        | ImmType::BA2
        | ImmType::DA
        | ImmType::DUMMY
        | ImmType::FCA
        | ImmType::I64A
        | ImmType::IA
        | ImmType::ILA
        | ImmType::ITA
        | ImmType::IVA
        | ImmType::KA
        | ImmType::LA
        | ImmType::LAR
        | ImmType::NLA
        | ImmType::RATA
        | ImmType::SA
        | ImmType::SLA
        | ImmType::VSA => name.to_token_stream(),

        ImmType::ARR(_) | ImmType::BLA | ImmType::OA(_) | ImmType::OAL(_) => {
            quote!(#name.as_ref())
        }

        ImmType::NA => panic!("NA is not expected"),
    }
}

pub trait PrintOpcodeTypes {
    type Write: ?Sized;
    type Error: std::error::Error;
}

#[cfg(test)]
mod tests {
    use hhbc_gen as _;
    use macro_test_util::assert_pat_eq;
    use quote::quote;

    use super::*;

    #[test]
    fn test_basic() {
        #[rustfmt::skip]
        assert_pat_eq(
            build_print_opcode(
                quote!(
                    #[derive(PrintOpcode)]
                    struct PrintMe<T>(Opcode);
                ),
                &opcode_test_data::test_opcodes(),
            ),
            quote!(
                impl<T> PrintMe<T> {
                    fn print_opcode(
                        &self,
                        w: &mut <Self as PrintOpcodeTypes>::Write
                    ) -> std::result::Result<(), <Self as PrintOpcodeTypes>::Error>
                    {
                        match self.get_opcode() {
                            Opcode::TestZeroImm => {
                                w.write_all(b"TestZeroImm")?;
                            }
                            Opcode::TestOneImm(str1) => {
                                w.write_all(b"TestOneImm ")?;
                                print_quoted_ffi_str(w, str1)?;
                            }
                            Opcode::TestTwoImm(str1, str2) => {
                                w.write_all(b"TestTwoImm ")?;
                                print_quoted_ffi_str(w, str1)?;
                                w.write_all(b" ")?;
                                print_quoted_ffi_str(w, str2)?;
                            }
                            Opcode::TestThreeImm(str1, str2, str3) => {
                                w.write_all(b"TestThreeImm ")?;
                                print_quoted_ffi_str(w, str1)?;
                                w.write_all(b" ")?;
                                print_quoted_ffi_str(w, str2)?;
                                w.write_all(b" ")?;
                                print_quoted_ffi_str(w, str3)?;
                            }
                            Opcode::TestAA(arr1) => {
                                w.write_all(b"TestAA ")?;
                                print_adata_id(w, arr1)?;
                            }
                            Opcode::TestARR(arr1) => {
                                w.write_all(b"TestARR ")?;
                                todo!("unsupported 'arr1'");
                            }
                            Opcode::TestBA(target1) => {
                                w.write_all(b"TestBA ")?;
                                self.print_label(w, target1)?;
                            }
                            Opcode::TestBA2(target1) => {
                                w.write_all(b"TestBA2 ")?;
                                self.print_label2(w, target1)?;
                            }
                            Opcode::TestBLA(targets) => {
                                w.write_all(b"TestBLA ")?;
                                self.print_branch_labels(w, targets.as_ref())?;
                            }
                            Opcode::TestDA(dbl1) => {
                                w.write_all(b"TestDA ")?;
                                print_float(w, *dbl1)?;
                            }
                            Opcode::TestFCA(fca) => {
                                w.write_all(b"TestFCA ")?;
                                self.print_fcall_args(w, fca)?;
                            }
                            Opcode::TestI64A(arg1) => {
                                w.write_all(b"TestI64A ")?;
                                write!(w, "{}", arg1)?;
                            }
                            Opcode::TestIA(iter1) => {
                                w.write_all(b"TestIA ")?;
                                print_iterator_id(w, iter1)?;
                            }
                            Opcode::TestILA(loc1) => {
                                w.write_all(b"TestILA ")?;
                                self.print_local(w, loc1)?;
                            }
                            Opcode::TestITA(ita) => {
                                w.write_all(b"TestITA ")?;
                                self.print_iter_args(w, ita)?;
                            }
                            Opcode::TestIVA(arg1) => {
                                w.write_all(b"TestIVA ")?;
                                write!(w, "{}", arg1)?;
                            }
                            Opcode::TestKA(mkey) => {
                                w.write_all(b"TestKA ")?;
                                self.print_member_key(w, mkey)?;
                            }
                            Opcode::TestLA(loc1) => {
                                w.write_all(b"TestLA ")?;
                                self.print_local(w, loc1)?;
                            }
                            Opcode::TestLAR(locrange) => {
                                w.write_all(b"TestLAR ")?;
                                print_local_range(w, locrange)?;
                            }
                            Opcode::TestNLA(nloc1) => {
                                w.write_all(b"TestNLA ")?;
                                self.print_local(w, nloc1)?;
                            }
                            Opcode::TestOA(subop1) => {
                                w.write_all(b"TestOA ")?;
                                print_oa_sub_type(w, subop1)?;
                            }
                            Opcode::TestOAL(subop1) => {
                                w.write_all(b"TestOAL ")?;
                                print_oa_sub_type(w, subop1)?;
                            }
                            Opcode::TestRATA(rat) => {
                                w.write_all(b"TestRATA ")?;
                                print_str(w, rat)?;
                            }
                            Opcode::TestSA(str1) => {
                                w.write_all(b"TestSA ")?;
                                print_quoted_ffi_str(w, str1)?;
                            }
                            Opcode::TestSLA(targets) => {
                                w.write_all(b"TestSLA ")?;
                                print_switch_labels(w, targets)?;
                            }
                            Opcode::TestVSA(keys) => {
                                w.write_all(b"TestVSA ")?;
                                print_shape_fields(w, keys)?;
                            }
                        }
                        Ok(())
                    }
                }
            ),
        );
    }
}
