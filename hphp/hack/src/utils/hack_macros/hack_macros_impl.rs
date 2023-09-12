// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#![feature(box_patterns)]

mod ast_writer;

use std::collections::HashMap;
use std::collections::HashSet;
use std::sync::Arc;

use aast_parser::rust_aast_parser_types::Env;
use aast_parser::Error as AastError;
use naming_special_names_rust::modules as special_modules;
use once_cell::sync::OnceCell;
use oxidized::ast;
use oxidized::ast::Def;
use oxidized::ast::Pos;
use oxidized::ast::Program;
use oxidized::errors;
use oxidized::parser_options::ParserOptions;
use parser_core_types::indexed_source_text::IndexedSourceText;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax_error::SyntaxError;
use proc_macro2::Literal;
use proc_macro2::Span;
use proc_macro2::TokenStream;
use quote::quote;
use quote::ToTokens;
use regex::Match;
use regex::Regex;
use relative_path::Prefix;
use relative_path::RelativePath;
use rust_parser_errors::UnstableFeatures;
use syn::parse::ParseStream;
use syn::parse::Parser;
use syn::punctuated::Punctuated;
use syn::spanned::Spanned;
use syn::Error;
use syn::Expr;
use syn::ExprLit;
use syn::Ident;
use syn::Lit;
use syn::LitStr;
use syn::Path;
use syn::Result;
use syn::Token;

use crate::ast_writer::Replacement;

/// See hack_expr! for docs.
#[proc_macro]
pub fn hack_expr_proc(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    let input: TokenStream = input.into();
    match hack_expr_impl.parse2(input) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

#[proc_macro]
pub fn hack_stmt_proc(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    let input: TokenStream = input.into();
    match hack_stmt_impl.parse2(input) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

#[proc_macro]
pub fn hack_stmts_proc(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    let input: TokenStream = input.into();
    match hack_stmts_impl.parse2(input) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

fn hack_stmts_impl(input: ParseStream<'_>) -> Result<TokenStream> {
    let exports: Path = input.parse()?;
    let input = Input::parse(input)?;
    let stmts = parse_stmts(&input.hack_src, 0, input.span)?;
    crate::ast_writer::write_ast(
        exports,
        input.span,
        input.replacements,
        input.pos_src,
        input.pos_id,
        stmts,
    )
    .map_err(Into::into)
}

fn hack_stmt_impl(input: ParseStream<'_>) -> Result<TokenStream> {
    let exports: Path = input.parse()?;
    let input = Input::parse(input)?;
    let stmt = parse_stmt(&input.hack_src, 0, input.span)?;
    crate::ast_writer::write_ast(
        exports,
        input.span,
        input.replacements,
        input.pos_src,
        input.pos_id,
        stmt,
    )
    .map_err(Into::into)
}

fn parse_stmts(src: &str, internal_offset: usize, span: Span) -> Result<Vec<ast::Stmt>> {
    // Parse the statements within a method (so we can use `$this` legally).
    let prefix = "class C { public function f(): void { ";
    let postfix = " } }";
    let src = format!("{}{}{}", prefix, src, postfix);
    let internal_offset = internal_offset + prefix.len();

    let mut program = parse_aast_from_string(&src, internal_offset, span)?;

    // Expect a single Def::Class.
    if program.0.len() != 1 {
        match program.0.first() {
            Some(Def::SetModule(module)) if module.1 == special_modules::DEFAULT => {
                // if the program did not specify a module, the default module would
                // have been automatically inserted here and can be safely ignored
                ()
            }
            _ => panic!("Expected a single Def"),
        }
    };
    let def = program.0.pop().unwrap();
    let cls = match def {
        Def::Class(cls) => cls,
        _ => panic!("Expected a Def::Class"),
    };

    let mut methods = cls.methods;
    if methods.len() != 1 {
        panic!("Expecting a single method");
    }
    let method = methods.pop().unwrap();

    Ok(method.body.fb_ast.0)
}

fn parse_stmt(src: &str, internal_offset: usize, span: Span) -> Result<ast::Stmt> {
    let mut stmts = parse_stmts(src, internal_offset, span)?;
    if stmts.len() != 1 {
        panic!("Expecting a single Stmt");
    }
    let stmt = stmts.pop().unwrap();
    Ok(stmt)
}

fn hack_expr_impl(input: ParseStream<'_>) -> Result<TokenStream> {
    // Unlike macro_rules macros, procedural macros can't refer to `$crate`. We
    // wrap the proc_macros with a macro_rules that passes `$crate::exports` as
    // the first value so the procedural macro can explicitly refer to types
    // exported by the hack_macros crate.
    let exports: Path = input.parse()?;

    let input = Input::parse(input)?;
    let expr = parse_expr(&input.hack_src, input.span)?;
    crate::ast_writer::write_ast(
        exports,
        input.span,
        input.replacements,
        input.pos_src,
        input.pos_id,
        expr,
    )
    .map_err(Into::into)
}

fn parse_expr(src: &str, span: Span) -> Result<ast::Expr> {
    // Parse the expression as a statement.
    let stmt = parse_stmt(&format!("{};", src), 0, span)?;
    match stmt.1 {
        ast::Stmt_::Expr(expr) => Ok(*expr),
        _ => Err(Error::new(span, "Expression expected")),
    }
}

struct Input {
    pos_src: TokenStream,
    pos_id: TokenStream,
    span: Span,
    hack_src: String,
    replacements: HashMap<String, Replacement>,
}

impl Input {
    fn parse(input: ParseStream<'_>) -> Result<Input> {
        let input: Punctuated<Expr, Token![,]> = Punctuated::parse_terminated(input)?;

        let mut pos_src = None;
        let mut hack_src = None;

        for expr in input.into_iter() {
            match expr {
                Expr::Assign(assign) => {
                    let left = match *assign.left {
                        Expr::Path(path) => path.path,
                        left => return Err(Error::new(left.span(), "Identifier expected")),
                    };
                    let target = if left.is_ident("pos") {
                        &mut pos_src
                    } else {
                        return Err(Error::new(left.span(), "Unknown keyword"));
                    };
                    if target.is_some() {
                        return Err(Error::new(
                            left.span(),
                            format!("{} cannot be set twice", left.to_token_stream()),
                        ));
                    }
                    *target = Some(assign.right);
                }
                Expr::Lit(ExprLit {
                    lit: Lit::Str(s), ..
                }) => {
                    if hack_src.is_some() {
                        return Err(Error::new(s.span(), "Unepected string"));
                    }
                    hack_src = Some(s);
                }
                _ => return Err(Error::new(expr.span(), "String expected")),
            }
        }

        let pos_src = pos_src.map_or_else(|| quote!(Pos::NONE), |p| p.to_token_stream());
        let hack_src =
            hack_src.ok_or_else(|| Error::new(Span::call_site(), "Missing hack source string"))?;

        let span = hack_src.span();
        let pos_id = crate::ast_writer::hygienic_pos(span.clone());

        let (hack_src, replacements) = prepare_hack(hack_src, &pos_id)?;

        Ok(Input {
            pos_src,
            pos_id,
            span,
            hack_src,
            replacements,
        })
    }
}

fn prepare_hack(
    input: LitStr,
    default_pos: &TokenStream,
) -> Result<(String, HashMap<String, Replacement>)> {
    static RE_VAR: OnceCell<Regex> = OnceCell::new();
    let re_var = RE_VAR.get_or_init(|| {
        let short = r"#(\w+)";
        let long = r"#\{([^}*]+)\}";
        let args = r"#\{(\w+)\*\}";
        let pat = [short, long, args]
            .into_iter()
            .map(|s| format!("(:?{})", s))
            .collect::<Vec<_>>()
            .join("|");
        Regex::new(&pat).unwrap()
    });

    let span = input.span();
    let mut replacements: HashMap<String, Replacement> = HashMap::default();

    let input_str = input.value();
    let mut output = String::with_capacity(input_str.len());
    let mut last_match = 0;
    for (idx, caps) in re_var.captures_iter(&input_str).enumerate() {
        assert_eq!(caps.len(), 7);
        // caps[0] = whole string
        // caps[1] = short form - full capture
        // caps[2] = short form - substring
        // caps[3] = long form - full capture
        // caps[4] = long form - substring
        // caps[5] = args form - full capture
        // caps[6] = args form - substring

        let m = caps.get(0).unwrap();
        output.push_str(&input_str[last_match..m.start()]);
        last_match = m.end();

        fn compute_subspan<'a>(input_str: &'a str, span: Span, mat: Match<'a>) -> (&'a str, Span) {
            // TODO: Our version of syn is old (1.0.75) and doesn't have the
            // `token()` function which would let us figure out how much to properly
            // add - so we'll have to be "close enough".
            let offset = 1;
            let range = (mat.start() + offset)..(mat.end() + offset);
            let subspan = span_for_range(input_str, 0, span, range);
            (mat.as_str(), subspan)
        }

        let var = { format!("$__hack_repl_{}", idx) };
        output.push_str(&var);

        // The last match is always the most interesting to us (prior matches
        // tend to be things like 'full string' matches instead of the captured
        // content).
        let (idx, match_) = caps
            .iter()
            .enumerate()
            .filter_map(|(i, m)| m.map(|s| (i, s)))
            .last()
            .unwrap();
        let (name_str, subspan) = compute_subspan(&input_str, span, match_);

        let repl = match idx {
            0 => {
                panic!("No matches found");
            }
            2 => {
                // short form - #name
                let pat = Ident::new(name_str, subspan).to_token_stream();
                Replacement::Simple { pat, span: subspan }
            }
            4 => {
                // long form - #{cmd(...)}
                parse_repl_var(name_str, subspan, default_pos)?
            }
            6 => {
                // args form - #{name*}
                let pat = Ident::new(name_str, subspan).to_token_stream();
                Replacement::Repeat { pat, span: subspan }
            }
            _ => {
                panic!("Unexpected match index {} in {:?}", idx, caps);
            }
        };

        replacements.insert(var, repl);
    }

    output.push_str(&input_str[last_match..]);
    Ok((output, replacements))
}

fn unwrap_call(input: &str, span: Span) -> Result<(&str, Vec<&str>)> {
    let open = input
        .find('(')
        .ok_or_else(|| Error::new(span, "'(' not found"))?;
    if !input.ends_with(')') {
        return Err(Error::new(span, "')' not at end"));
    }

    fn is_sep(c: char, in_paren: &mut isize) -> bool {
        if *in_paren > 0 {
            match c {
                '(' => *in_paren += 1,
                ')' => *in_paren -= 1,
                _ => {}
            }
            false
        } else {
            match c {
                '(' => {
                    *in_paren = 1;
                    false
                }
                ')' => {
                    // Force a detection at the end.
                    *in_paren = 100;
                    false
                }
                ',' => true,
                _ => false,
            }
        }
    }

    let cmd = input[..open].trim();
    let args_text = &input[open + 1..input.len() - 1];
    let mut args = Vec::new();

    if !args_text.is_empty() {
        let mut in_paren = 0;
        let mut last_start = 0;
        for (i, c) in args_text.chars().enumerate() {
            if is_sep(c, &mut in_paren) {
                args.push(args_text[last_start..i].trim());
                last_start = i + 1;
            }
        }

        if in_paren != 0 {
            return Err(Error::new(span, "Unbalanced parentheses"));
        }

        args.push(args_text[last_start..].trim());
    }

    Ok((cmd, args))
}

fn is_word_char(c: char) -> bool {
    c.is_alphanumeric() || c == '_'
}

fn is_word(input: &str) -> bool {
    input.chars().all(is_word_char)
}

fn parse_repl_var(input: &str, span: Span, default_pos: &TokenStream) -> Result<Replacement> {
    let (cmd, args) = unwrap_call(input, span)?;

    fn parse_pos(
        has_convert: bool,
        cmd: &str,
        args: &[&str],
        span: Span,
        default_pos: &TokenStream,
    ) -> Result<TokenStream> {
        match args.len() {
            0 => Err(Error::new(span, format!("Too few arguments to '{}'", cmd))),
            1 => Ok(default_pos.clone()),
            2 if has_convert => Ok(Ident::new(args[1], span).to_token_stream()),
            _ => Err(Error::new(span, format!("Too many arguments to '{}'", cmd))),
        }
    }

    fn parse_expr<F: FnOnce(Ident) -> TokenStream>(
        inner: &str,
        span: Span,
        f: F,
    ) -> Result<TokenStream> {
        if is_word(inner) {
            Ok(Ident::new(inner, span).to_token_stream())
        } else {
            let (cmd, args) = unwrap_call(inner, span)?;
            if cmd != "clone" {
                return Err(Error::new(span, "Inner command can only be 'clone'"));
            }
            match args.len() {
                0 => return Err(Error::new(span, "Too few arguments to 'clone'")),
                1 => {}
                _ => return Err(Error::new(span, "Too many arguments to 'clone'")),
            }

            let var = Ident::new(args[0], span);
            Ok(f(var))
        }
    }

    match cmd {
        "as_expr" => {
            let pat = parse_expr(args[0], span, |var| quote!(#var.clone()))?;
            Ok(Replacement::AsExpr { pat, span })
        }
        "clone" => {
            let pat = parse_expr(input, span, |var| quote!(#var.clone()))?;
            Ok(Replacement::Simple { pat, span })
        }
        "id" => {
            let pat = parse_expr(args[0], span, |var| quote!(#var.to_string()))?;
            let pos = parse_pos(true, cmd, &args, span, default_pos)?;
            Ok(Replacement::Id { pat, pos, span })
        }
        "lvar" => {
            let pat = parse_expr(args[0], span, |var| quote!(#var.clone()))?;
            let pos = parse_pos(true, cmd, &args, span, default_pos)?;
            Ok(Replacement::Lvar { pat, pos, span })
        }
        "str" => {
            let pat = parse_expr(args[0], span, |var| quote!(#var.to_owned()))?;
            let pos = parse_pos(true, cmd, &args, span, default_pos)?;
            Ok(Replacement::Str { pat, pos, span })
        }
        _ => Err(Error::new(span, format!("Unknown command '{}'", cmd))),
    }
}

fn parse_aast_from_string(input: &str, internal_offset: usize, span: Span) -> Result<Program> {
    let parser_options = ParserOptions {
        tco_union_intersection_type_hints: true,
        po_allow_unstable_features: true,
        ..ParserOptions::default()
    };

    let env = Env {
        codegen: true,
        elaborate_namespaces: false,
        include_line_comments: false,
        parser_options,
        php5_compat_mode: false,
        quick_mode: false,
        show_all_errors: true,
        is_systemlib: true,
        for_debugger_eval: false,
        scour_comments: false,
    };

    let rel_path = RelativePath::make(Prefix::Dummy, "".into());
    let source_text = SourceText::make(Arc::new(rel_path), input.as_bytes());
    let indexed_source_text = IndexedSourceText::new(source_text);

    let mut default_unstable_features = HashSet::default();
    default_unstable_features.insert(UnstableFeatures::TypedLocalVariables);
    default_unstable_features.insert(UnstableFeatures::PipeAwait);

    let aast =
        aast_parser::AastParser::from_text(&env, &indexed_source_text, default_unstable_features)
            .map_err(|err| convert_aast_error(err, input, internal_offset, span))?;

    aast.errors
        .iter()
        .try_for_each(|e| convert_error(e, input, internal_offset, span))?;
    aast.syntax_errors
        .iter()
        .try_for_each(|e| convert_syntax_error(e, input, internal_offset, span))?;
    aast.lowerer_parsing_errors
        .iter()
        .try_for_each(|e| convert_lowerer_parsing_error(e, input, internal_offset, span))?;

    Ok(aast.aast)
}

fn span_for_pos(src: &str, internal_offset: usize, span: Span, pos: &Pos) -> Span {
    let range = pos.info_raw();
    span_for_range(src, internal_offset, span, range.0..range.1)
}

fn span_for_range(
    src: &str,
    internal_offset: usize,
    span: Span,
    mut range: std::ops::Range<usize>,
) -> Span {
    if internal_offset <= range.start {
        range.start -= internal_offset;
    } else {
        range.start = 0;
    }
    if internal_offset <= range.end {
        range.end -= internal_offset;
    } else {
        range.end = 0;
    }
    let mut tmp = Literal::string(&src[internal_offset..]);
    tmp.set_span(span);
    tmp.subspan(range).unwrap_or(span)
}

fn convert_aast_error(err: AastError, src: &str, internal_offset: usize, span: Span) -> Error {
    match err {
        AastError::NotAHackFile() => {
            Error::new(Span::call_site(), "Internal Error: Not a Hack File")
        }
        AastError::ParserFatal(syn, _) => convert_syntax_error(&syn, src, internal_offset, span)
            .err()
            .unwrap(),
        AastError::Other(msg) => Error::new(Span::call_site(), msg),
    }
}

fn convert_error(err: &errors::Error, src: &str, internal_offset: usize, span: Span) -> Result<()> {
    let err_span = span_for_pos(src, internal_offset, span, &err.claim.0);
    Err(Error::new(err_span, err.claim.1.to_string()))
}

fn convert_syntax_error(
    err: &SyntaxError,
    src: &str,
    internal_offset: usize,
    span: Span,
) -> Result<()> {
    let err_span = span_for_range(src, internal_offset, span, err.start_offset..err.end_offset);
    Err(Error::new(
        err_span,
        format!("[{}] {}", err.start_offset, err.message),
    ))
}

fn convert_lowerer_parsing_error(
    err: &(Pos, String),
    src: &str,
    internal_offset: usize,
    span: Span,
) -> Result<()> {
    let err_span = span_for_pos(src, internal_offset, span, &err.0);
    Err(Error::new(err_span, err.1.to_string()))
}

#[cfg(test)]
mod tests {
    use macro_test_util::assert_pat_eq;

    use super::*;

    #[test]
    fn test_basic() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(EX pos = p, "#foo + $bar")),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = p;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Binop(Box::new(Binop {
                        bop: Bop::Plus,
                        lhs: {
                            let tmp: Expr = foo;
                            tmp
                        },
                        rhs: Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Lvar(Box::new(Lid(
                                __hygienic_pos.clone(),
                                (0isize, "$bar".to_owned()),
                            ))),
                        ),
                    })),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex1() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(EX
                pos = pos(),
                r#"#obj_lvar->#meth_lvar(...#{lvar(args_var)})"#
            )),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = pos();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new(CallExpr {
                        func: Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::ObjGet(Box::new((
                                {
                                    let tmp: Expr = obj_lvar;
                                    tmp
                                },
                                {
                                    let tmp: Expr = meth_lvar;
                                    tmp
                                },
                                OgNullFlavor::OGNullthrows,
                                PropOrMethod::IsMethod,
                            ))),
                        ),
                        targs: vec![],
                        args: vec![],
                        unpacked_arg: Some({
                            let tmp: LocalId = args_var;
                            Expr(
                                (),
                                __hygienic_pos.clone(),
                                Expr_::Lvar(Box::new(Lid(__hygienic_pos.clone(), tmp))),
                            )
                        }),
                    })),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex2() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(EX
                pos = pos(),
                r#"\__SystemLib\dynamic_meth_caller(
                       #{clone(cexpr)},
                       #{clone(fexpr)},
                       #efun,
                       #force_val_expr
                  )
                "#
            )),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = pos();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new(CallExpr {
                        func: Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(
                                __hygienic_pos.clone(),
                                "\\__SystemLib\\dynamic_meth_caller".to_owned(),
                            ))),
                        ),
                        targs: vec![],
                        args: vec![
                            (ParamKind::Pnormal, {
                                let tmp: Expr = cexpr.clone();
                                tmp
                            }),
                            (ParamKind::Pnormal, {
                                let tmp: Expr = fexpr.clone();
                                tmp
                            }),
                            (ParamKind::Pnormal, {
                                let tmp: Expr = efun;
                                tmp
                            }),
                            (ParamKind::Pnormal, {
                                let tmp: Expr = force_val_expr;
                                tmp
                            }),
                        ],
                        unpacked_arg: None,
                    })),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex3() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(EX
                pos = pos(),
                r#"\__SystemLib\meth_caller(#{str(clone(mangle_name))})"#
            )),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = pos();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new(CallExpr {
                        func: Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(
                                __hygienic_pos.clone(),
                                "\\__SystemLib\\meth_caller".to_owned(),
                            ))),
                        ),
                        targs: vec![],
                        args: vec![(ParamKind::Pnormal, {
                            let tmp: String = mangle_name.to_owned();
                            Expr((), __hygienic_pos.clone(), Expr_::String(tmp.into()))
                        })],
                        unpacked_arg: None,
                    })),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex4() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(EX
                pos = pos(),
                r#"\HH\invariant(
                       \is_a(#{clone(obj_lvar)}, #{str(clone(cls), pc)}),
                       #{str(msg)}
                   )
                "#
            )),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = pos();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new(CallExpr {
                        func: Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(
                                __hygienic_pos.clone(),
                                "\\HH\\invariant".to_owned(),
                            ))),
                        ),
                        targs: vec![],
                        args: vec![
                            (
                                ParamKind::Pnormal,
                                Expr(
                                    (),
                                    __hygienic_pos.clone(),
                                    Expr_::Call(Box::new(CallExpr {
                                        func: Expr(
                                            (),
                                            __hygienic_pos.clone(),
                                            Expr_::Id(Box::new(Id(
                                                __hygienic_pos.clone(),
                                                "\\is_a".to_owned(),
                                            ))),
                                        ),
                                        targs: vec![],
                                        args: vec![
                                            (ParamKind::Pnormal, {
                                                let tmp: Expr = obj_lvar.clone();
                                                tmp
                                            }),
                                            (ParamKind::Pnormal, {
                                                let tmp: String = cls.to_owned();
                                                Expr((), pc.clone(), Expr_::String(tmp.into()))
                                            }),
                                        ],
                                        unpacked_arg: None,
                                    })),
                                ),
                            ),
                            (ParamKind::Pnormal, {
                                let tmp: String = msg;
                                Expr((), __hygienic_pos.clone(), Expr_::String(tmp.into()))
                            }),
                        ],
                        unpacked_arg: None,
                    })),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex5() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(EX
                pos = pos(),
                r#"#obj_lvar->#{id(clone(fname), pf)}(...#{lvar(args_var)})"#
            )),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = pos();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new(CallExpr {
                        func: Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::ObjGet(Box::new((
                                {
                                    let tmp: Expr = obj_lvar;
                                    tmp
                                },
                                {
                                    let tmp: String = fname.to_string();
                                    Expr((), pf.clone(), Expr_::Id(Box::new(Id(pf.clone(), tmp))))
                                },
                                OgNullFlavor::OGNullthrows,
                                PropOrMethod::IsMethod,
                            ))),
                        ),
                        targs: vec![],
                        args: vec![],
                        unpacked_arg: Some({
                            let tmp: LocalId = args_var;
                            Expr(
                                (),
                                __hygienic_pos.clone(),
                                Expr_::Lvar(Box::new(Lid(__hygienic_pos.clone(), tmp))),
                            )
                        }),
                    })),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex6() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(EX "echo #{str(tail)}")),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = Pos::NONE;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new(CallExpr {
                        func: Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(__hygienic_pos.clone(), "echo".to_owned()))),
                        ),
                        targs: vec![],
                        args: vec![(ParamKind::Pnormal, {
                            let tmp: String = tail;
                            Expr((), __hygienic_pos.clone(), Expr_::String(tmp.into()))
                        })],
                        unpacked_arg: None,
                    })),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex7() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(EX "parent::__xhpAttributeDeclaration()")),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = Pos::NONE;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new(CallExpr {
                        func: Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::ClassConst(Box::new((
                                ClassId(
                                    (),
                                    __hygienic_pos.clone(),
                                    ClassId_::CIexpr(Expr(
                                        (),
                                        __hygienic_pos.clone(),
                                        Expr_::Id(Box::new(Id(
                                            __hygienic_pos.clone(),
                                            "parent".to_owned(),
                                        ))),
                                    )),
                                ),
                                (
                                    __hygienic_pos.clone(),
                                    "__xhpAttributeDeclaration".to_owned(),
                                ),
                            ))),
                        ),
                        targs: vec![],
                        args: vec![],
                        unpacked_arg: None,
                    })),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex8() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(EX "#{id(s)}::__xhpAttributeDeclaration()")),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = Pos::NONE;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new(CallExpr {
                        func: Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::ClassConst(Box::new((
                                ClassId(
                                    (),
                                    __hygienic_pos.clone(),
                                    ClassId_::CIexpr({
                                        let tmp: String = s;
                                        Expr(
                                            (),
                                            __hygienic_pos.clone(),
                                            Expr_::Id(Box::new(Id(__hygienic_pos.clone(), tmp))),
                                        )
                                    }),
                                ),
                                (
                                    __hygienic_pos.clone(),
                                    "__xhpAttributeDeclaration".to_owned(),
                                ),
                            ))),
                        ),
                        targs: vec![],
                        args: vec![],
                        unpacked_arg: None,
                    })),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex9() {
        assert_pat_eq(
            hack_stmt_impl.parse2(quote!(EX
                "if (#{lvar(clone(name))} is __uninitSentinel) { unset(#{lvar(name)}); }"
            )),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = Pos::NONE;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Stmt(
                    __hygienic_pos.clone(),
                    Stmt_::If(Box::new((
                        Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Is(Box::new((
                                {
                                    let tmp: LocalId = name.clone();
                                    Expr(
                                        (),
                                        __hygienic_pos.clone(),
                                        Expr_::Lvar(Box::new(Lid(__hygienic_pos.clone(), tmp))),
                                    )
                                },
                                Hint(
                                    __hygienic_pos.clone(),
                                    Box::new(Hint_::Happly(
                                        Id(__hygienic_pos.clone(), "__uninitSentinel".to_owned()),
                                        vec![],
                                    )),
                                ),
                            ))),
                        ),
                        Block(vec![Stmt(
                            __hygienic_pos.clone(),
                            Stmt_::Expr(Box::new(Expr(
                                (),
                                __hygienic_pos.clone(),
                                Expr_::Call(Box::new(CallExpr {
                                    func: Expr(
                                        (),
                                        __hygienic_pos.clone(),
                                        Expr_::Id(Box::new(Id(
                                            __hygienic_pos.clone(),
                                            "unset".to_owned(),
                                        ))),
                                    ),
                                    targs: vec![],
                                    args: vec![(ParamKind::Pnormal, {
                                        let tmp: LocalId = name;
                                        Expr(
                                            (),
                                            __hygienic_pos.clone(),
                                            Expr_::Lvar(Box::new(Lid(__hygienic_pos.clone(), tmp))),
                                        )
                                    })],
                                    unpacked_arg: None,
                                })),
                            ))),
                        )]),
                        Block(vec![Stmt(__hygienic_pos.clone(), Stmt_::Noop)]),
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex10() {
        assert_pat_eq(
            hack_stmt_impl.parse2(quote!(EX
                pos = p(),
                r#"if (\__SystemLib\__debugger_is_uninit(#{lvar(clone(name))})) {
                       #{lvar(name)} = new __uninitSentinel();
                     }
                "#
            )),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = p();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Stmt(
                    __hygienic_pos.clone(),
                    Stmt_::If(Box::new((
                        Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Call(Box::new(CallExpr {
                                func: Expr(
                                    (),
                                    __hygienic_pos.clone(),
                                    Expr_::Id(Box::new(Id(
                                        __hygienic_pos.clone(),
                                        "\\__SystemLib\\__debugger_is_uninit".to_owned(),
                                    ))),
                                ),
                                targs: vec![],
                                args: vec![(ParamKind::Pnormal, {
                                    let tmp: LocalId = name.clone();
                                    Expr(
                                        (),
                                        __hygienic_pos.clone(),
                                        Expr_::Lvar(Box::new(Lid(__hygienic_pos.clone(), tmp))),
                                    )
                                })],
                                unpacked_arg: None,
                            })),
                        ),
                        Block(vec![Stmt(
                            __hygienic_pos.clone(),
                            Stmt_::Expr(Box::new(Expr(
                                (),
                                __hygienic_pos.clone(),
                                Expr_::Binop(Box::new(Binop {
                                    bop: Bop::Eq(None),
                                    lhs: {
                                        let tmp: LocalId = name;
                                        Expr(
                                            (),
                                            __hygienic_pos.clone(),
                                            Expr_::Lvar(Box::new(Lid(__hygienic_pos.clone(), tmp))),
                                        )
                                    },
                                    rhs: Expr(
                                        (),
                                        __hygienic_pos.clone(),
                                        Expr_::New(Box::new((
                                            ClassId(
                                                (),
                                                __hygienic_pos.clone(),
                                                ClassId_::CIexpr(Expr(
                                                    (),
                                                    __hygienic_pos.clone(),
                                                    Expr_::Id(Box::new(Id(
                                                        __hygienic_pos.clone(),
                                                        "__uninitSentinel".to_owned(),
                                                    ))),
                                                )),
                                            ),
                                            vec![],
                                            vec![],
                                            None,
                                            (),
                                        ))),
                                    ),
                                })),
                            ))),
                        )]),
                        Block(vec![Stmt(__hygienic_pos.clone(), Stmt_::Noop)]),
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex11() {
        assert_pat_eq(
            hack_stmt_impl.parse2(quote!(EX
                pos = p(),
                r#"
                    try {
                        #{stmts*};
                    } catch (Throwable #{lvar(exnvar)}) {
                        /* no-op */
                    } finally {
                        #{sets*};
                    }
                "#
            )),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = p();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Stmt(
                    __hygienic_pos.clone(),
                    Stmt_::Try(Box::new((
                        Block(
                            std::iter::empty()
                                .chain(stmts.into_iter())
                                .collect::<Vec<_>>(),
                        ),
                        vec![Catch(
                            Id(__hygienic_pos.clone(), "Throwable".to_owned()),
                            {
                                let tmp: LocalId = exnvar;
                                Lid(__hygienic_pos.clone(), tmp)
                            },
                            Block(vec![]),
                        )],
                        FinallyBlock(
                            std::iter::empty()
                                .chain(sets.into_iter())
                                .collect::<Vec<_>>(),
                        ),
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex12() {
        assert_pat_eq(
            hack_stmts_impl.parse2(quote!(EX
                r#"
                    $r = self::$__xhpAttributeDeclarationCache;
                    if ($r === null) {
                        self::$__xhpAttributeDeclarationCache =
                            __SystemLib\merge_xhp_attr_declarations(#{args*});
                        $r = self::$__xhpAttributeDeclarationCache;
                    }
                    return $r;
                "#
            )),
            {
                let stmt1 = quote!(Stmt(
                    __hygienic_pos.clone(),
                    Stmt_::Expr(Box::new(Expr(
                        (),
                        __hygienic_pos.clone(),
                        Expr_::Binop(Box::new(Binop {
                            bop: Bop::Eq(None),
                            lhs: Expr(
                                (),
                                __hygienic_pos.clone(),
                                Expr_::Lvar(Box::new(Lid(
                                    __hygienic_pos.clone(),
                                    (0isize, "$r".to_owned())
                                )))
                            ),
                            rhs: Expr(
                                (),
                                __hygienic_pos.clone(),
                                Expr_::ClassGet(Box::new((
                                    ClassId(
                                        (),
                                        __hygienic_pos.clone(),
                                        ClassId_::CIexpr(Expr(
                                            (),
                                            __hygienic_pos.clone(),
                                            Expr_::Id(Box::new(Id(
                                                __hygienic_pos.clone(),
                                                "self".to_owned()
                                            )))
                                        ))
                                    ),
                                    ClassGetExpr::CGstring((
                                        __hygienic_pos.clone(),
                                        "$__xhpAttributeDeclarationCache".to_owned()
                                    )),
                                    PropOrMethod::IsProp
                                )))
                            )
                        }))
                    )))
                ));
                let stmt2 = quote!(Stmt(
                    __hygienic_pos.clone(),
                    Stmt_::If(Box::new((
                        Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Binop(Box::new(Binop {
                                bop: Bop::Eqeqeq,
                                lhs: Expr(
                                    (),
                                    __hygienic_pos.clone(),
                                    Expr_::Lvar(Box::new(Lid(
                                        __hygienic_pos.clone(),
                                        (0isize, "$r".to_owned())
                                    )))
                                ),
                                rhs: Expr((), __hygienic_pos.clone(), Expr_::Null)
                            }))
                        ),
                        Block(vec![
                            Stmt(
                                __hygienic_pos.clone(),
                                Stmt_::Expr(Box::new(Expr(
                                    (),
                                    __hygienic_pos.clone(),
                                    Expr_::Binop(Box::new(Binop {
                                        bop: Bop::Eq(None),
                                        lhs: Expr(
                                            (),
                                            __hygienic_pos.clone(),
                                            Expr_::ClassGet(Box::new((
                                                ClassId(
                                                    (),
                                                    __hygienic_pos.clone(),
                                                    ClassId_::CIexpr(Expr(
                                                        (),
                                                        __hygienic_pos.clone(),
                                                        Expr_::Id(Box::new(Id(
                                                            __hygienic_pos.clone(),
                                                            "self".to_owned()
                                                        )))
                                                    ))
                                                ),
                                                ClassGetExpr::CGstring((
                                                    __hygienic_pos.clone(),
                                                    "$__xhpAttributeDeclarationCache".to_owned()
                                                )),
                                                PropOrMethod::IsProp
                                            )))
                                        ),
                                        rhs: Expr(
                                            (),
                                            __hygienic_pos.clone(),
                                            Expr_::Call(Box::new(CallExpr {
                                                func: Expr(
                                                    (),
                                                    __hygienic_pos.clone(),
                                                    Expr_::Id(Box::new(Id(
                                                        __hygienic_pos.clone(),
                                                        "__SystemLib\\merge_xhp_attr_declarations"
                                                            .to_owned()
                                                    )))
                                                ),
                                                targs: vec![],
                                                args: std::iter::empty()
                                                    .chain(args.into_iter())
                                                    .collect::<Vec<_>>(),
                                                unpacked_arg: None
                                            }))
                                        )
                                    }))
                                )))
                            ),
                            Stmt(
                                __hygienic_pos.clone(),
                                Stmt_::Expr(Box::new(Expr(
                                    (),
                                    __hygienic_pos.clone(),
                                    Expr_::Binop(Box::new(Binop {
                                        bop: Bop::Eq(None),
                                        lhs: Expr(
                                            (),
                                            __hygienic_pos.clone(),
                                            Expr_::Lvar(Box::new(Lid(
                                                __hygienic_pos.clone(),
                                                (0isize, "$r".to_owned())
                                            )))
                                        ),
                                        rhs: Expr(
                                            (),
                                            __hygienic_pos.clone(),
                                            Expr_::ClassGet(Box::new((
                                                ClassId(
                                                    (),
                                                    __hygienic_pos.clone(),
                                                    ClassId_::CIexpr(Expr(
                                                        (),
                                                        __hygienic_pos.clone(),
                                                        Expr_::Id(Box::new(Id(
                                                            __hygienic_pos.clone(),
                                                            "self".to_owned()
                                                        )))
                                                    ))
                                                ),
                                                ClassGetExpr::CGstring((
                                                    __hygienic_pos.clone(),
                                                    "$__xhpAttributeDeclarationCache".to_owned()
                                                )),
                                                PropOrMethod::IsProp
                                            )))
                                        )
                                    }))
                                )))
                            )
                        ]),
                        Block(vec![Stmt(__hygienic_pos.clone(), Stmt_::Noop)])
                    )))
                ));
                let stmt3 = quote!(Stmt(
                    __hygienic_pos.clone(),
                    Stmt_::Return(Box::new(Some(Expr(
                        (),
                        __hygienic_pos.clone(),
                        Expr_::Lvar(Box::new(Lid(
                            __hygienic_pos.clone(),
                            (0isize, "$r".to_owned())
                        )))
                    ))))
                ));
                quote!({
                    use EX::ast::*;
                    let __hygienic_pos: Pos = Pos::NONE;
                    #[allow(clippy::redundant_clone)]
                    let __hygienic_tmp = vec![#stmt1, #stmt2, #stmt3];
                    __hygienic_tmp
                })
            },
        );
    }

    #[test]
    fn test_ex13() {
        assert_pat_eq(hack_expr_impl.parse2(quote!(EX r#"darray[a => 42]"#)), {
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = Pos::NONE;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Darray(Box::new((
                        None,
                        vec![(
                            Expr(
                                (),
                                __hygienic_pos.clone(),
                                Expr_::Id(Box::new(Id(__hygienic_pos.clone(), "a".to_owned()))),
                            ),
                            Expr((), __hygienic_pos.clone(), Expr_::Int("42".to_owned())),
                        )],
                    ))),
                );
                __hygienic_tmp
            })
        });
    }

    #[test]
    fn test_stmt() {
        assert_pat_eq(
            hack_stmt_impl.parse2(quote!(EX pos = p, "{ a; #b; c; }")),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = p;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Stmt(
                    __hygienic_pos.clone(),
                    Stmt_::Block(Box::new((
                        None,
                        Block(vec![
                            Stmt(
                                __hygienic_pos.clone(),
                                Stmt_::Expr(Box::new(Expr(
                                    (),
                                    __hygienic_pos.clone(),
                                    Expr_::Id(Box::new(Id(__hygienic_pos.clone(), "a".to_owned()))),
                                ))),
                            ),
                            {
                                let tmp: Stmt = b;
                                tmp
                            },
                            Stmt(
                                __hygienic_pos.clone(),
                                Stmt_::Expr(Box::new(Expr(
                                    (),
                                    __hygienic_pos.clone(),
                                    Expr_::Id(Box::new(Id(__hygienic_pos.clone(), "c".to_owned()))),
                                ))),
                            ),
                        ]),
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_stmts() {
        assert_pat_eq(
            hack_stmt_impl.parse2(quote!(EX pos = p, "{ a; b; #{c*}; d; e; }")),
            {
                let before = quote!([
                    Stmt(
                        __hygienic_pos.clone(),
                        Stmt_::Expr(Box::new(Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(__hygienic_pos.clone(), "a".to_owned()))),
                        ))),
                    ),
                    Stmt(
                        __hygienic_pos.clone(),
                        Stmt_::Expr(Box::new(Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(__hygienic_pos.clone(), "b".to_owned()))),
                        ))),
                    ),
                ]);
                let after = quote!([
                    Stmt(
                        __hygienic_pos.clone(),
                        Stmt_::Expr(Box::new(Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(__hygienic_pos.clone(), "d".to_owned(),))),
                        ))),
                    ),
                    Stmt(
                        __hygienic_pos.clone(),
                        Stmt_::Expr(Box::new(Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(__hygienic_pos.clone(), "e".to_owned(),))),
                        ))),
                    ),
                ]);
                quote!({
                    use EX::ast::*;
                    let __hygienic_pos: Pos = p;
                    #[allow(clippy::redundant_clone)]
                    let __hygienic_tmp = Stmt(
                        __hygienic_pos.clone(),
                        Stmt_::Block(Box::new ((None, Block(
                            std::iter::empty()
                                .chain(#before.into_iter())
                                .chain(c.into_iter())
                                .chain(#after.into_iter())
                                .collect::<Vec<_>>(),
                        )))),
                    );
                    __hygienic_tmp
                })
            },
        );
    }

    #[test]
    fn test_ex14() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(EX pos = pos.clone(),
            r#"() ==> {
                 $result = #kind;
                 foreach (#{clone(collection)} as #{as_expr(clone(binding))}) {
                   #inner_body;
                 }
                 return $result;
               }()
             "#)),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = pos.clone();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new(CallExpr {
                        func: Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Lfun(Box::new((
                                Fun_ {
                                    span: __hygienic_pos.clone(),
                                    readonly_this: None,
                                    annotation: (),
                                    readonly_ret: None,
                                    ret: TypeHint((), None),
                                    params: vec![],
                                    ctxs: None,
                                    unsafe_ctxs: None,
                                    body: FuncBody {
                                        fb_ast: Block(vec![
                                            Stmt(
                                                __hygienic_pos.clone(),
                                                Stmt_::Expr(Box::new(Expr(
                                                    (),
                                                    __hygienic_pos.clone(),
                                                    Expr_::Binop(Box::new(Binop {
                                                        bop: Bop::Eq(None),
                                                        lhs: Expr(
                                                            (),
                                                            __hygienic_pos.clone(),
                                                            Expr_::Lvar(Box::new(Lid(
                                                                __hygienic_pos.clone(),
                                                                (0isize, "$result".to_owned()),
                                                            ))),
                                                        ),
                                                        rhs: {
                                                            let tmp: Expr = kind;
                                                            tmp
                                                        },
                                                    })),
                                                ))),
                                            ),
                                            Stmt(
                                                __hygienic_pos.clone(),
                                                Stmt_::Foreach(Box::new((
                                                    {
                                                        let tmp: Expr = collection.clone();
                                                        tmp
                                                    },
                                                    {
                                                        let tmp: AsExpr = binding.clone();
                                                        tmp
                                                    },
                                                    Block(vec![{
                                                        let tmp: Stmt = inner_body;
                                                        tmp
                                                    }]),
                                                ))),
                                            ),
                                            Stmt(
                                                __hygienic_pos.clone(),
                                                Stmt_::Return(Box::new(Some(Expr(
                                                    (),
                                                    __hygienic_pos.clone(),
                                                    Expr_::Lvar(Box::new(Lid(
                                                        __hygienic_pos.clone(),
                                                        (0isize, "$result".to_owned()),
                                                    ))),
                                                )))),
                                            ),
                                        ]),
                                    },
                                    fun_kind: FunKind::FSync,
                                    user_attributes: UserAttributes(vec![]),
                                    external: false,
                                    doc_comment: None,
                                },
                                vec![],
                            ))),
                        ),
                        targs: vec![],
                        args: vec![],
                        unpacked_arg: None,
                    })),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_exprs_expand() {
        assert_pat_eq(
            hack_stmt_impl.parse2(quote!(EX r#"return vec[#a, #{b*}, #c];"#)),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = Pos::NONE;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Stmt(
                    __hygienic_pos.clone(),
                    Stmt_::Return(Box::new(Some(Expr(
                        (),
                        __hygienic_pos.clone(),
                        Expr_::ValCollection(Box::new((
                            (__hygienic_pos.clone(), VcKind::Vec),
                            None,
                            std::iter::empty()
                                .chain(
                                    [{
                                        let tmp: Expr = a;
                                        tmp
                                    }]
                                    .into_iter(),
                                )
                                .chain(b.into_iter())
                                .chain(
                                    [{
                                        let tmp: Expr = c;
                                        tmp
                                    }]
                                    .into_iter(),
                                )
                                .collect::<Vec<_>>(),
                        ))),
                    )))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_typed_local_stmt() {
        assert_pat_eq(
            hack_stmt_impl.parse2(quote!(EX pos = p, "let $x: t = #e;")),
            quote!({
                use EX::ast::*;
                let __hygienic_pos: Pos = p;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Stmt(
                    __hygienic_pos.clone(),
                    Stmt_::DeclareLocal(Box::new((
                        Lid(__hygienic_pos.clone(), (0isize, "$x".to_owned())),
                        Hint(
                            __hygienic_pos.clone(),
                            Box::new(Hint_::Happly(
                                Id(__hygienic_pos.clone(), "t".to_owned()),
                                vec![],
                            )),
                        ),
                        Some({
                            let tmp: Expr = e;
                            tmp
                        }),
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }
}
