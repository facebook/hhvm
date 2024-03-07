// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashMap;

use hhbc_gen::ImmType;
use hhbc_gen::Inputs;
use hhbc_gen::OpcodeData;
use hhbc_gen::Outputs;
use proc_macro2::Ident;
use proc_macro2::TokenStream;
use quote::quote;
use quote::quote_spanned;
use quote::ToTokens;
use syn::parse::ParseStream;
use syn::parse::Parser;
use syn::spanned::Spanned;
use syn::Arm;
use syn::Attribute;
use syn::Error;
use syn::Expr;
use syn::ExprAssign;
use syn::ExprMacro;
use syn::ExprMatch;
use syn::ExprPath;
use syn::Item;
use syn::Macro;
use syn::Pat;
use syn::PatPath;
use syn::Path;
use syn::Result;
use syn::Stmt;
use syn::Token;

/// This macro is used by the convert_opcode function to help it convert
/// "standard" opcodes from HHVM's bytecode to IR Instrs.
///
/// The macro looks in the main `match` of the function for patterns in the form:
///
///     EnumSrc => simple!(EnumDst),
///     EnumSrc => todo!(),
///
/// When it sees a match arm in that form it rewrites it using the hhbc_gen
/// data.  As a concrete example:
///
///     Opcode::ResolveFunc => simple!(Hhbc::ResolveFunc),
///
/// is rewritten as:
///
///     Opcode::ResolveFunc(ref str1) => {
///         let str1 = *str1;
///         Action::Push(Instr::Hhbc(Hhbc::ResolveFunc(str1, ctx.loc)))
///     }
///
/// The macro is more complicated than just the basic rewriter because Rust
/// doesn't allow you to use a macro as the pattern for a match statement - so
/// we have to rewrite the whole function so we can rewrite the match itself.
#[proc_macro_attribute]
pub fn bc_to_ir(
    _attr: proc_macro::TokenStream,
    input: proc_macro::TokenStream,
) -> proc_macro::TokenStream {
    match build_bc_to_ir(input.into(), hhbc_gen::opcode_data()) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

fn build_bc_to_ir(input: TokenStream, opcodes: &[OpcodeData]) -> Result<TokenStream> {
    let mut input = syn::parse2::<Item>(input)?;

    let opcodes: HashMap<String, &OpcodeData> = opcodes
        .iter()
        .map(|data| (data.name.to_owned(), data))
        .collect();

    let fn_item = match &mut input {
        Item::Fn(item) => item,
        _ => panic!("Unexpected item {}", debug_dump(input)),
    };

    for stmt in fn_item.block.as_mut().stmts.iter_mut() {
        parse_stmt(stmt, &opcodes)?;
    }

    Ok(quote!(#input))
}

fn debug_dump<T: std::fmt::Debug>(t: T) -> String {
    let mut s = format!("{:?}", t);
    s.truncate(10000);
    s
}

fn parse_stmt(stmt: &mut Stmt, opcodes: &HashMap<String, &OpcodeData>) -> Result<()> {
    match stmt {
        Stmt::Local(local) => {
            if has_bc_to_ir_attr(&mut local.attrs) {
                local.attrs = Vec::new();
                if let Some(init) = local.init.as_mut() {
                    let expr = init.1.as_mut();
                    match expr {
                        Expr::Match(m) => process_match(m, opcodes)?,
                        _ => {
                            panic!("Unable to use #[bc_to_ir] on non-match.");
                        }
                    }
                } else {
                    panic!("Unable to use #[bc_to_ir] on non-init assignment.");
                }
            }
        }
        Stmt::Item(..) | Stmt::Expr(..) | Stmt::Semi(..) => {}
    }
    Ok(())
}

fn has_bc_to_ir_attr(attrs: &mut [Attribute]) -> bool {
    attrs
        .iter()
        .next()
        .map_or(false, |attr| attr.path.is_ident("bc_to_ir"))
}

fn expr_to_path(expr: Expr) -> Result<Path> {
    match expr {
        Expr::Path(ExprPath { path, .. }) => Ok(path),
        _ => Err(Error::new(expr.span(), "Simple path expected")),
    }
}

fn process_match(expr: &mut ExprMatch, opcodes: &HashMap<String, &OpcodeData>) -> Result<()> {
    for arm in expr.arms.iter_mut() {
        match &mut *arm.body {
            Expr::Macro(ExprMacro {
                mac: Macro { path, tokens, .. },
                ..
            }) if path.is_ident("simple") => {
                let tokens = std::mem::take(tokens);
                *arm = (|parser: ParseStream<'_>| parse_convert_simple(parser, &arm.pat, opcodes))
                    .parse2(tokens)?;
            }
            Expr::Macro(ExprMacro {
                mac: Macro { path, tokens, .. },
                ..
            }) if path.is_ident("todo") => {
                let tokens = std::mem::take(tokens);
                *arm = (|parser: ParseStream<'_>| parse_convert_todo(parser, &arm.pat, opcodes))
                    .parse2(tokens)?;
            }
            _ => {}
        }
    }
    Ok(())
}

fn parse_opcode_pat<'a, 'b>(
    pat: &'a Pat,
    opcodes: &'b HashMap<String, &OpcodeData>,
) -> Result<(&'a Path, &'b OpcodeData)> {
    let span = pat.span();

    let opcode_path = match pat {
        Pat::Path(PatPath { path, .. }) => path,
        _ => return Err(Error::new(span, "Simple path expected")),
    };
    let (opcode_class, opcode_variant) = split_path2(opcode_path)?;
    if opcode_class != "Opcode" {
        return Err(Error::new(
            span,
            format!("Expected 'Opcode', not '{}'", opcode_class),
        ));
    }
    let data = if let Some(data) = opcodes.get(&opcode_variant) {
        data
    } else {
        return Err(Error::new(
            span,
            format!("Opcode '{}' not found", opcode_variant),
        ));
    };

    Ok((opcode_path, data))
}

#[allow(clippy::todo)]
fn parse_convert_simple(
    input: ParseStream<'_>,
    pat: &Pat,
    opcodes: &HashMap<String, &OpcodeData>,
) -> Result<Arm> {
    // simple!(Foo::Bar) ==> pat => Instr::Foo(Foo::Bar(loc))
    // simple!(Foo::Bar, expr) ==> pat => Instr::Foo(Foo::Bar(expr, loc))

    let (opcode_path, data) = parse_opcode_pat(pat, opcodes)?;

    let span = pat.span();

    let instr_path: Path = input.parse()?;
    let (instr_class, _instr_variant) = split_path2(&instr_path)?;
    let is_constant = instr_class == "Constant";
    let is_terminator = instr_class == "Terminator";
    let instr_class = Ident::new(&instr_class, instr_path.span());

    let mut extra = Vec::new();
    while !input.is_empty() {
        input.parse::<Token![,]>()?;
        let expr = input.parse::<Expr>()?;
        match expr {
            Expr::Assign(ExprAssign { left, right: _, .. }) => {
                let left = expr_to_path(*left)?;
                return Err(Error::new(left.span(), "Unexpected tag"));
            }
            Expr::Path(_) | Expr::Lit(_) => {
                extra.push(expr);
            }
            _ => {
                return Err(Error::new(span, "Unhandled input"));
            }
        }
    }

    let mut imms_out: Vec<&str> = data.immediates.iter().map(|(a, _)| *a).collect();
    let mut pops = Vec::new();
    let mut params: Vec<TokenStream> = Vec::new();
    let mut conv = Vec::new();

    match &data.inputs {
        Inputs::NOV => {}
        Inputs::Fixed(inputs) => {
            let mut inputs: Vec<Ident> = (0..inputs.len())
                .map(|i| Ident::new(&format!("s{}", i + 1), span))
                .collect();

            pops.extend(inputs.iter().rev().map(|id| quote!(let #id = ctx.pop();)));

            if inputs.len() == 1 {
                params.push(inputs.pop().unwrap().to_token_stream());
            } else {
                params.push(quote!([#(#inputs),*]));
            };
        }

        Inputs::CMany => {
            pops.push(quote!(let args = collect_args(ctx, arg1).into();));
            *imms_out.iter_mut().find(|imm| **imm == "arg1").unwrap() = "args";
        }

        Inputs::CMFinal(..)
        | Inputs::CUMany
        | Inputs::FCall { .. }
        | Inputs::MFinal
        | Inputs::SMany => {
            return Err(Error::new(
                span,
                format!("Unhandled conversion: {:?}", data.inputs),
            ));
        }
    };

    let mut match_pats = Vec::new();
    for (name, ty) in &data.immediates {
        let imm = Ident::new(name, span);

        let mut convert = |converter: TokenStream| {
            match_pats.push(quote!(ref #imm));
            conv.push(quote_spanned!(span=> let #imm = #converter; ));
        };

        let strings = quote_spanned!(span=> ctx.strings);

        match ty {
            ImmType::OA(n) if *n == "ClassName" => {
                convert(quote_spanned!(span=> ir::ClassId::from_hhbc(*#imm, #strings) ));
            }
            ImmType::OA(n) if *n == "ConstName" => {
                convert(quote_spanned!(span=> *#imm ));
            }
            ImmType::OA(n) if *n == "FunctionName" => {
                convert(quote_spanned!(span=> *#imm ));
            }
            ImmType::OA(n) if *n == "MethodName" => {
                convert(quote_spanned!(span=> ir::MethodId::from_hhbc(*#imm, #strings) ));
            }
            ImmType::OA(n) if *n == "PropName" => {
                convert(quote_spanned!(span=> ir::PropId::from_hhbc(*#imm, #strings) ));
            }
            ImmType::AA
            | ImmType::ARR(_)
            | ImmType::BA
            | ImmType::BA2
            | ImmType::BLA
            | ImmType::DA
            | ImmType::DUMMY
            | ImmType::FCA
            | ImmType::I64A
            | ImmType::IA
            | ImmType::ITA
            | ImmType::IVA
            | ImmType::KA
            | ImmType::NA
            | ImmType::OA(_)
            | ImmType::RATA
            | ImmType::SA
            | ImmType::SLA
            | ImmType::VSA => {
                match_pats.push(imm.to_token_stream());
            }
            ImmType::ILA | ImmType::LA | ImmType::NLA => {
                convert(quote_spanned!(span=> convert_local(ctx, #imm) ));
            }
            ImmType::LAR => {
                convert(quote_spanned!(span=> convert_local_range(ctx, #imm) ));
            }
        }
    }

    params.extend(
        imms_out
            .into_iter()
            .map(|name| Ident::new(name, span).to_token_stream()),
    );
    params.extend(extra.iter().map(ToTokens::to_token_stream));
    if !is_constant {
        params.push(quote!(ctx.loc));
    }

    let pat = build_match_pat(opcode_path, match_pats)?;

    let cons = {
        match &data.outputs {
            Outputs::NOV => {
                if is_terminator {
                    quote!(Action::Terminal(#instr_path(#(#params),*)))
                } else {
                    quote!(Action::Emit(Instr::#instr_class(#instr_path(#(#params),*))))
                }
            }
            Outputs::Fixed(outputs) if outputs.len() == 1 => {
                if is_constant {
                    if params.is_empty() {
                        quote!(Action::Constant(#instr_path))
                    } else {
                        quote!(Action::Constant(#instr_path(#(#params),*)))
                    }
                } else if is_terminator {
                    quote!(Action::Terminal(#instr_path(#(#params),*)))
                } else {
                    quote!(Action::Push(Instr::#instr_class(#instr_path(#(#params),*))))
                }
            }
            outputs => {
                return Err(Error::new(
                    span,
                    format!("Unhandled outputs: {:?}", outputs),
                ));
            }
        }
    };

    let arm = syn::parse2::<Arm>(quote!(#pat => {
        #(#pops)*
        #(#conv)*
        #cons
    }))?;
    Ok(arm)
}

#[allow(clippy::todo)]
fn parse_convert_todo(
    _input: ParseStream<'_>,
    pat: &Pat,
    opcodes: &HashMap<String, &OpcodeData>,
) -> Result<Arm> {
    let (_, data) = parse_opcode_pat(pat, opcodes)?;

    let span = pat.span();
    let pat = pat.to_token_stream();
    let msg = format!("Unimplemented opcode '{}'", pat);
    let tokens = if data.immediates.is_empty() {
        quote_spanned!(span=> #pat => todo!(#msg),)
    } else {
        quote_spanned!(span=> #pat(..) => todo!(#msg),)
    };

    let arm = syn::parse2::<Arm>(tokens)?;
    Ok(arm)
}

fn build_match_pat(opcode: &Path, match_pats: Vec<TokenStream>) -> Result<Pat> {
    let toks = if match_pats.is_empty() {
        opcode.to_token_stream()
    } else {
        quote!(#opcode ( #(#match_pats),* ))
    };
    syn::parse2::<Pat>(toks)
}

fn split_path(path: &Path) -> Vec<String> {
    let mut res = Vec::new();
    for seg in path.segments.iter() {
        res.push(format!("{}", seg.ident));
    }
    res
}

fn split_path2(path: &Path) -> Result<(String, String)> {
    let v = split_path(path);
    if v.len() != 2 {
        Err(Error::new(path.span(), "Expected A::B"))
    } else {
        let mut it = v.into_iter();
        let a = it.next().unwrap();
        let b = it.next().unwrap();
        Ok((a, b))
    }
}
