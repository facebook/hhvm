// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::HashMap;

use aast_parser::rust_aast_parser_types::Env;
use aast_parser::Error as AastError;
use ocamlrep::rc::RcOc;
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
use proc_macro2::Span;
use proc_macro2::TokenStream;
use quote::quote;
use quote::quote_spanned;
use quote::ToTokens;
use regex::Match;
use regex::Regex;
use relative_path::Prefix;
use relative_path::RelativePath;
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
use syn::Result;
use syn::Token;

/// A macro to build Hack Expr trees.
///
/// Usage:
///     hack_expr!(pos = p, "#foo + $bar")
///
/// Returns an ast::Expr representing the given code.
///
/// The `pos` parameter is optional.  If not provided then a Pos::none() will be
/// used.
///
/// The code string must be a string literal. It cannot be a dynamically
/// generated string.
///
/// The code string can contain replacements to inject values into the generated
/// tree:
///
///     hack_expr!("$foo + #bar + $baz")
///
/// The replacements have various forms:
///   - `#name` Inject the Expr in local variable `name`.
///   - `#{args*}` As a parameter to a call this inserts a Vec<Expr> as
///     ParamKind::Pnormal parameters.
///   - `#{clone(name)}` Clone the Expr `name` instead of consuming it.
///   - `#{cmd(name)}` Convert name using 'cmd' (see below).
///   - `#{cmd(clone(name))}` Clone `name` and then convert using 'cmd' (see below).
///
/// Conversion commands:
///
///   - `#{id(name)}` builds an Expr_::Ident from a string.
///   - `#{lvar(name)}` builds an Expr_::LVar from a LocalId.
///   - `#{str(name)}` builds an Expr_::String from a &str.
///
/// All of the commands can also take an optional position override parameter:
///   - `#{str(name, pos)}`
///   - `#{str(clone(name), pos)}`
///
///
/// Technical note:
///
/// The transformation is done at hackc compile time - the macro takes the input
/// string and uses the aast parser to parse it and generate an Expr.  It then
/// traverses the the Expr (again - at compile time) to generate code to
/// construct the Expr at runtime.  The Hack code needs to be quoted because the
/// Rust parser doesn't like some Hack constructs (in particular backslash
/// separated identifiers).
///
///     hack_expr!(pos = p, "#foo + $bar")
///
/// transforms into something like this:
///
///     Expr(
///         (),
///         p.clone(),
///         Expr_::Binop(Box::new((
///             Bop::Plus,
///             foo,
///             Expr((), p.clone(), Expr_::Lvar(Lid(p.clone(), (0, "$bar")))),
///         )))
///     )
///
#[proc_macro]
pub fn hack_expr(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    let input: TokenStream = input.into();
    match hack_expr_impl.parse2(input) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

/// Like `hack_expr!` but produces an ast::Stmt value (see `hack_expr!` for
/// full docs).
#[proc_macro]
pub fn hack_stmt(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    let input: TokenStream = input.into();
    match hack_stmt_impl.parse2(input) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

/// Like `hack_stmt!` but produces a `Vec<ast::Stmt>` value (see `hack_expr!`
/// for full docs).
#[proc_macro]
pub fn hack_stmts(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    let input: TokenStream = input.into();
    match hack_stmts_impl.parse2(input) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

fn hack_stmts_impl(input: ParseStream<'_>) -> Result<TokenStream> {
    let input = Input::parse(input)?;
    let stmts = parse_stmts(&input.hack_src, 0, input.span)?;
    emit::Emitter::build(input.span, input.replacements, input.pos, stmts)
}

fn hack_stmt_impl(input: ParseStream<'_>) -> Result<TokenStream> {
    let input = Input::parse(input)?;
    let stmt = parse_stmt(&input.hack_src, 0, input.span)?;
    emit::Emitter::build(input.span, input.replacements, input.pos, stmt)
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
        panic!("Expected a single Def");
    }
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
    let input = Input::parse(input)?;
    let expr = parse_expr(&input.hack_src, input.span)?;
    emit::Emitter::build(input.span, input.replacements, input.pos, expr)
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
    pos: TokenStream,
    span: Span,
    hack_src: String,
    replacements: HashMap<String, ReplVar>,
}

impl Input {
    fn parse(input: ParseStream<'_>) -> Result<Input> {
        let input: Punctuated<Expr, Token![,]> = Punctuated::parse_terminated(input)?;

        let mut pos = None;
        let mut hack_src = None;

        for expr in input.into_iter() {
            match expr {
                Expr::Assign(assign) => {
                    let left = match *assign.left {
                        Expr::Path(path) => path.path,
                        left => return Err(Error::new(left.span(), "Identifier expected")),
                    };
                    let target = if left.is_ident("pos") {
                        &mut pos
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

        let pos = pos.map_or_else(|| quote!(Pos::NONE), |p| p.to_token_stream());
        let hack_src =
            hack_src.ok_or_else(|| Error::new(Span::call_site(), "Missing hack source string"))?;

        let span = hack_src.span();

        let (hack_src, replacements) = prepare_hack(hack_src, &pos)?;

        Ok(Input {
            pos,
            span,
            hack_src,
            replacements,
        })
    }
}

#[derive(Debug)]
struct ReplVar {
    expr: TokenStream,
    op: VarOp,
    pos: TokenStream,
    span: Span,
}

impl ReplVar {
    fn to_expr(&self) -> Result<TokenStream> {
        let expr = &self.expr;
        let tmp = syn::Ident::new("tmp", Span::mixed_site());
        let out = match self.op {
            VarOp::None => {
                quote_spanned!(self.span=> {
                    let #tmp: Expr = #expr;
                    #tmp
                })
            }
            VarOp::Id => {
                let pos = &self.pos;
                quote_spanned!(self.span=> {
                    let #tmp: String = #expr;
                    Expr((), #pos.clone(), Expr_::Id(Box::new(Id(#pos.clone(), #tmp))))
                })
            }
            VarOp::Lvar => {
                let pos = &self.pos;
                quote_spanned!(self.span=> {
                    let #tmp: LocalId = #expr;
                    Expr((), #pos.clone(), Expr_::Lvar(Box::new(Lid(#pos.clone(), #tmp))))
                })
            }
            VarOp::Str => {
                let pos = &self.pos;
                quote_spanned!(self.span=> {
                    let #tmp: String = #expr;
                    Expr((), #pos.clone(), Expr_::String(#tmp.into()))
                })
            }
            VarOp::Args => {
                return Err(syn::Error::new(
                    self.span,
                    "Replacement used in bad spot (this is probably a bug in the hack_expr macro)",
                ));
            }
        };
        Ok(out)
    }
}

#[derive(Copy, Clone, Eq, PartialEq, Hash, Debug)]
enum VarOp {
    None,
    Id,
    Lvar,
    Str,
    Args,
}

fn prepare_hack(
    input: LitStr,
    default_pos: &TokenStream,
) -> Result<(String, HashMap<String, ReplVar>)> {
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
    let mut replacements: HashMap<String, ReplVar> = HashMap::default();

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
                let expr = Ident::new(name_str, subspan).to_token_stream();
                ReplVar {
                    expr,
                    op: VarOp::None,
                    pos: default_pos.clone(),
                    span: subspan,
                }
            }
            4 => {
                // long form - #{cmd(...)}
                parse_repl_var(name_str, subspan, default_pos)?
            }
            6 => {
                // args form - #{name*}
                let expr = Ident::new(name_str, subspan).to_token_stream();
                ReplVar {
                    expr,
                    op: VarOp::Args,
                    pos: default_pos.clone(),
                    span: subspan,
                }
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

fn parse_repl_var(input: &str, span: Span, default_pos: &TokenStream) -> Result<ReplVar> {
    let (cmd, args) = unwrap_call(input, span)?;

    let op = match cmd {
        "clone" => VarOp::None,
        "id" => VarOp::Id,
        "lvar" => VarOp::Lvar,
        "str" => VarOp::Str,
        _ => return Err(Error::new(span, format!("Unknown command '{}'", cmd))),
    };

    let pos = match args.len() {
        0 => return Err(Error::new(span, format!("Too few arguments to '{}'", cmd))),
        1 => default_pos.clone(),
        2 if op != VarOp::None => Ident::new(args[1], span).to_token_stream(),
        _ => {
            return Err(Error::new(span, format!("Too many arguments to '{}'", cmd)));
        }
    };

    let inner = if op == VarOp::None { input } else { args[0] };
    let expr = if is_word(inner) {
        Ident::new(inner, span).to_token_stream()
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
        match op {
            VarOp::None | VarOp::Lvar => quote!(#var.clone()),
            VarOp::Id => quote!(#var.to_string()),
            VarOp::Str => quote!(#var.to_owned()),
            VarOp::Args => unreachable!("in parse_repl_var"),
        }
    };

    Ok(ReplVar {
        expr,
        op,
        pos,
        span,
    })
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
        keep_errors: true,
        parser_options,
        php5_compat_mode: false,
        quick_mode: false,
        show_all_errors: true,
        is_systemlib: true,
        scour_comments: false,
    };

    let rel_path = RelativePath::make(Prefix::Dummy, "".into());
    let source_text = SourceText::make(RcOc::new(rel_path), input.as_bytes());
    let indexed_source_text = IndexedSourceText::new(source_text);

    let aast = aast_parser::AastParser::from_text(&env, &indexed_source_text)
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
    let mut tmp = proc_macro2::Literal::string(&src[internal_offset..]);
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

mod emit {
    // This section is more focused on `ast` than `syn` - so don't import too
    // much conflicting stuff from syn.
    use std::collections::HashMap;

    use oxidized::ast;
    use proc_macro2::Span;
    use proc_macro2::TokenStream;
    use quote::quote;
    use quote::quote_spanned;
    use quote::ToTokens;
    use syn::Result;

    use super::ReplVar;
    use super::VarOp;

    pub(crate) trait EmitTokens {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream>;

        fn emit_enum_from_debug(&self, enum_: &str) -> Result<TokenStream>
        where
            Self: std::fmt::Debug,
        {
            let enum_ = syn::Ident::new(enum_, Span::call_site());
            let variant = syn::Ident::new(&format!("{:?}", self), Span::call_site());
            Ok(quote!(#enum_::#variant))
        }
    }

    pub(crate) struct Emitter {
        pub(crate) pos: TokenStream,
        pub(crate) replacements: HashMap<String, ReplVar>,
    }

    impl Emitter {
        fn new(span: Span, replacements: HashMap<String, ReplVar>) -> Self {
            let pos = proc_macro2::Ident::new("__hygienic_pos", span).to_token_stream();
            Self { pos, replacements }
        }

        pub(crate) fn build<T: EmitTokens>(
            span: Span,
            replacements: HashMap<String, ReplVar>,
            pos_src: TokenStream,
            t: T,
        ) -> Result<TokenStream> {
            let emitter = Self::new(span, replacements);

            let t = t.emit_tokens(&emitter)?;
            // If this fires it means that we tried to swap out a replacement
            // but the emitter didn't convert it properly.
            debug_assert!(!t.to_string().contains("__hack_repl_"));
            let pos_id = emitter.pos;
            // We assign to a temp and then return so that we can silence some
            // clippy lints (you can't use an attribute on an expression).
            let tmp = proc_macro2::Ident::new("__hygienic_tmp", span);
            Ok(quote!({
                use oxidized::ast::*;
                let #pos_id: Pos = #pos_src;
                #[allow(clippy::redundant_clone)]
                let #tmp = #t;
                #tmp
            }))
        }

        fn lookup_replacement(&self, name: &str) -> Option<&ReplVar> {
            self.replacements.get(name)
        }

        fn lookup_param_replacement(
            &self,
            (_, expr): &(ast::ParamKind, ast::Expr),
        ) -> Option<&ReplVar> {
            match &expr.2 {
                ast::Expr_::Lvar(inner) => self.lookup_replacement(&inner.1.1).and_then(|repl| {
                    if repl.op == VarOp::Args {
                        Some(repl)
                    } else {
                        None
                    }
                }),
                _ => None,
            }
        }

        fn lookup_stmt_replacement(&self, stmt: &ast::Stmt) -> Option<&ReplVar> {
            match &stmt.1 {
                ast::Stmt_::Expr(inner) => match &inner.2 {
                    ast::Expr_::Lvar(inner) => self.lookup_replacement(&inner.1.1),
                    _ => None,
                },
                _ => None,
            }
        }
    }

    fn emit_wrapper<T: EmitTokens>(id: TokenStream, t: T, e: &Emitter) -> Result<TokenStream> {
        let t = t.emit_tokens(e)?;
        Ok(quote!(#id(#t)))
    }

    fn emit_wrapper2<T0: EmitTokens, T1: EmitTokens>(
        id: TokenStream,
        t0: T0,
        t1: T1,
        e: &Emitter,
    ) -> Result<TokenStream> {
        let t0 = t0.emit_tokens(e)?;
        let t1 = t1.emit_tokens(e)?;
        Ok(quote!(#id(#t0, #t1)))
    }

    fn emit_sequence_helper<T: EmitTokens>(slice: &[T], e: &Emitter) -> Result<TokenStream> {
        let out: Vec<_> = slice
            .iter()
            .map(|p| p.emit_tokens(e))
            .collect::<Result<_>>()?;
        Ok(quote!(vec![#(#out),*]))
    }

    fn emit_repl_sequence_helper<T: EmitTokens>(
        slice: &[T],
        e: &Emitter,
        chk: impl for<'a> Fn(&'a Emitter, &'a T) -> Option<&'a ReplVar>,
    ) -> Result<TokenStream> {
        let has_repl = slice.iter().any(|p| chk(e, p).is_some());
        if !has_repl {
            return emit_sequence_helper(slice, e);
        }

        // #{args*} replacement...

        // Generates something that looks like:
        //   std::iter::empty()
        //     .chain([a, b, c].into_iter())
        //     .chain(args.into_iter())
        //     .chain([d, e, f].into_iter())
        //     .collect::<Vec<_>>()
        let mut result = quote!(std::iter::empty());

        let mut cur = Vec::new();
        for v in slice {
            if let Some(repl) = chk(e, v) {
                if !cur.is_empty() {
                    result = quote!(#result.chain([#(#cur),*].into_iter()));
                }
                cur.clear();

                let args = &repl.expr;
                result = quote_spanned!(repl.span=> #result.chain(#args.into_iter()));
            } else {
                cur.push(v.emit_tokens(e)?);
            }
        }

        if !cur.is_empty() {
            result = quote!(#result.chain([#(#cur),*].into_iter()));
        }

        Ok(quote!(#result.collect::<Vec<_>>()))
    }

    impl EmitTokens for () {
        fn emit_tokens(&self, _e: &Emitter) -> Result<TokenStream> {
            Ok(quote!(()))
        }
    }

    impl EmitTokens for isize {
        fn emit_tokens(&self, _e: &Emitter) -> Result<TokenStream> {
            Ok(quote!(#self))
        }
    }

    impl<T: EmitTokens> EmitTokens for &T {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            <T>::emit_tokens(*self, e)
        }
    }

    impl EmitTokens for ast::Binop {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            let bop = self.bop.emit_tokens(e)?;
            let lhs = self.lhs.emit_tokens(e)?;
            let rhs = self.rhs.emit_tokens(e)?;
            Ok(quote!(Binop{bop:#bop, lhs:#lhs, rhs:#rhs}))
        }
    }

    impl EmitTokens for ast::Bop {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            use ast::Bop;
            match self {
                Bop::Eq(sub) => emit_wrapper(quote!(Bop::Eq), sub, e),
                _ => self.emit_enum_from_debug("Bop"),
            }
        }
    }

    impl<T: EmitTokens> EmitTokens for Box<T> {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_wrapper(
                quote!(Box::new),
                <Self as std::borrow::Borrow<T>>::borrow(self),
                e,
            )
        }
    }

    impl EmitTokens for ast::Catch {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            let clsid = self.0.emit_tokens(e)?;
            let var = self.1.emit_tokens(e)?;
            let body = self.2.emit_tokens(e)?;
            Ok(quote!(Catch(#clsid, #var, #body)))
        }
    }

    impl EmitTokens for ast::ClassGetExpr {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            use ast::ClassGetExpr;
            match self {
                ClassGetExpr::CGstring(s) => emit_wrapper(quote!(ClassGetExpr::CGstring), s, e),
                ClassGetExpr::CGexpr(ex) => emit_wrapper(quote!(ClassGetExpr::CGexpr), ex, e),
            }
        }
    }

    impl EmitTokens for ast::ClassId {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            let pos = self.1.emit_tokens(e)?;
            let cid_ = self.2.emit_tokens(e)?;
            Ok(quote!(ClassId((), #pos, #cid_)))
        }
    }

    impl EmitTokens for ast::ClassId_ {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            use ast::ClassId_;
            match self {
                ClassId_::CIexpr(expr) => emit_wrapper(quote!(ClassId_::CIexpr), expr, e),
                _ => self.emit_enum_from_debug("ClassId_"),
            }
        }
    }

    impl EmitTokens for ast::Expr {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            match &self.2 {
                ast::Expr_::Id(inner) => {
                    if let Some(repl) = e.lookup_replacement(&inner.1) {
                        return repl.to_expr();
                    }
                }
                ast::Expr_::Lvar(inner) => {
                    if let Some(repl) = e.lookup_replacement(&inner.1.1) {
                        return repl.to_expr();
                    }
                }
                _ => {}
            }

            let pos = self.1.emit_tokens(e)?;
            let expr_ = self.2.emit_tokens(e)?;
            Ok(quote!(Expr((), #pos, #expr_)))
        }
    }

    impl EmitTokens for ast::Expr_ {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            use ast::Expr_;
            match self {
                Expr_::Binop(inner) => emit_wrapper(quote!(Expr_::Binop), inner, e),
                Expr_::Call(inner) => emit_wrapper(quote!(Expr_::Call), inner, e),
                Expr_::ClassConst(inner) => emit_wrapper(quote!(Expr_::ClassConst), inner, e),
                Expr_::ClassGet(inner) => emit_wrapper(quote!(Expr_::ClassGet), inner, e),
                Expr_::Id(inner) => emit_wrapper(quote!(Expr_::Id), inner, e),
                Expr_::Int(inner) => emit_wrapper(quote!(Expr_::Int), inner, e),
                Expr_::Is(inner) => emit_wrapper(quote!(Expr_::Is), inner, e),
                Expr_::Lvar(inner) => emit_wrapper(quote!(Expr_::Lvar), inner, e),
                Expr_::New(inner) => emit_wrapper(quote!(Expr_::New), inner, e),
                Expr_::Null => Ok(quote!(Expr_::Null)),
                Expr_::ObjGet(inner) => emit_wrapper(quote!(Expr_::ObjGet), inner, e),
                expr => todo!(
                    "Unhandled expr (please implement this missing Expr_ case): {:?}",
                    expr
                ),
            }
        }
    }

    impl EmitTokens for ast::Hint {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_wrapper2(quote!(Hint), &self.0, &self.1, e)
        }
    }

    impl EmitTokens for ast::Hint_ {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            use ast::Hint_;
            match self {
                Hint_::Hoption(inner) => emit_wrapper(quote!(Hint_::Hoption), inner, e),
                Hint_::Hlike(inner) => emit_wrapper(quote!(Hint_::Hlike), inner, e),
                Hint_::Hfun(inner) => emit_wrapper(quote!(Hint_::Hfun), inner, e),
                Hint_::Htuple(inner) => emit_wrapper(quote!(Hint_::Hlike), inner, e),
                Hint_::Happly(p0, p1) => emit_wrapper2(quote!(Hint_::Happly), p0, p1, e),
                Hint_::Hshape(inner) => emit_wrapper(quote!(Hint_::Hshape), inner, e),
                Hint_::Haccess(p0, p1) => emit_wrapper2(quote!(Hint_::Haccess), p0, p1, e),
                Hint_::Hrefinement(p0, p1) => emit_wrapper2(quote!(Hint_::Hrefinement), p0, p1, e),
                Hint_::Hsoft(inner) => emit_wrapper(quote!(Hint_::Hsoft), inner, e),
                Hint_::Hany => Ok(quote!(Hint_::Hany)),
                Hint_::Herr => Ok(quote!(Hint_::Herr)),
                Hint_::Hmixed => Ok(quote!(Hint_::Hmixed)),
                Hint_::Hnonnull => Ok(quote!(Hint_::Hnonnull)),
                Hint_::Habstr(p0, p1) => emit_wrapper2(quote!(Hint_::Habstr), p0, p1, e),
                Hint_::HvecOrDict(p0, p1) => emit_wrapper2(quote!(Hint_::HvecOrDict), p0, p1, e),
                Hint_::Hprim(inner) => emit_wrapper(quote!(Hint_::Hprim), inner, e),
                Hint_::Hthis => Ok(quote!(Hint_::Hthis)),
                Hint_::Hdynamic => Ok(quote!(Hint_::Hdynamic)),
                Hint_::Hnothing => Ok(quote!(Hint_::Hnothing)),
                Hint_::Hunion(inner) => emit_wrapper(quote!(Hint_::Hunion), inner, e),
                Hint_::Hintersection(inner) => emit_wrapper(quote!(Hint_::Hintersection), inner, e),
                Hint_::HfunContext(inner) => emit_wrapper(quote!(Hint_::HfunContext), inner, e),
                Hint_::Hvar(inner) => emit_wrapper(quote!(Hint_::Hvar), inner, e),
            }
        }
    }

    impl EmitTokens for ast::HintFun {
        fn emit_tokens(&self, _e: &Emitter) -> Result<TokenStream> {
            todo!("EmitTokens: HintFun")
        }
    }

    impl EmitTokens for ast::Id {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_wrapper2(quote!(Id), &self.0, &self.1, e)
        }
    }

    impl EmitTokens for ast::Lid {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            let ast::Lid(pos, (scope, name)) = self;
            if let Some(repl) = e.lookup_replacement(name) {
                let pos = &repl.pos;
                let tmp = syn::Ident::new("tmp", Span::mixed_site());
                let expr = &repl.expr;
                Ok(match repl.op {
                    VarOp::None => quote_spanned!(repl.span=> {
                        let #tmp: Lid = #expr;
                        #tmp
                    }),
                    VarOp::Lvar => {
                        quote_spanned!(repl.span=> {
                            let #tmp: LocalId = #expr;
                            Lid(#pos.clone(), #tmp)
                        })
                    }
                    VarOp::Id | VarOp::Str | VarOp::Args => {
                        return Err(syn::Error::new(
                            repl.span,
                            "Bad type for this position (must be either 'none' or 'lvar')",
                        ));
                    }
                })
            } else {
                emit_wrapper2(quote!(Lid), pos, (scope, name), e)
            }
        }
    }

    impl EmitTokens for ast::NastShapeInfo {
        fn emit_tokens(&self, _e: &Emitter) -> Result<TokenStream> {
            todo!("EmitTokens: NastShapeInfo")
        }
    }

    impl EmitTokens for ast::OgNullFlavor {
        fn emit_tokens(&self, _e: &Emitter) -> Result<TokenStream> {
            self.emit_enum_from_debug("OgNullFlavor")
        }
    }

    impl<T: EmitTokens> EmitTokens for Option<T> {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            if let Some(value) = self.as_ref() {
                emit_wrapper(quote!(Some), value, e)
            } else {
                Ok(quote!(None))
            }
        }
    }

    impl EmitTokens for ast::ParamKind {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            use ast::ParamKind;
            match self {
                ParamKind::Pinout(pos) => emit_wrapper(quote!(ParamKind::Pinout), pos, e),
                ParamKind::Pnormal => Ok(quote!(ParamKind::Pnormal)),
            }
        }
    }

    impl EmitTokens for ast::Pos {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            // Use the user-supplied `pos` instead of the original Pos (which
            // would be from our macro's AST parse).
            let pos = &e.pos;
            Ok(quote!(#pos.clone()))
        }
    }

    impl EmitTokens for ast::PropOrMethod {
        fn emit_tokens(&self, _e: &Emitter) -> Result<TokenStream> {
            self.emit_enum_from_debug("PropOrMethod")
        }
    }

    impl EmitTokens for ast::Block {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_wrapper(quote!(Block), &self.0, e)
        }
    }

    impl EmitTokens for ast::FinallyBlock {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_wrapper(quote!(FinallyBlock), &self.0, e)
        }
    }

    impl EmitTokens for ast::Stmt {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_wrapper2(quote!(Stmt), &self.0, &self.1, e)
        }
    }

    impl EmitTokens for ast::Stmt_ {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            use ast::Stmt_;
            match self {
                Stmt_::Expr(inner) => emit_wrapper(quote!(Stmt_::Expr), inner, e),
                Stmt_::If(inner) => emit_wrapper(quote!(Stmt_::If), inner, e),
                Stmt_::Noop => Ok(quote!(Stmt_::Noop)),
                Stmt_::Return(inner) => emit_wrapper(quote!(Stmt_::Return), inner, e),
                Stmt_::Try(inner) => emit_wrapper(quote!(Stmt_::Try), inner, e),
                stmt => todo!(
                    "Unhandled stmt (please implement this missing Stmt_ case): {:?}",
                    stmt
                ),
            }
        }
    }

    impl EmitTokens for String {
        fn emit_tokens(&self, _e: &Emitter) -> Result<TokenStream> {
            Ok(quote!(#self.to_owned()))
        }
    }

    impl EmitTokens for ast::Targ {
        fn emit_tokens(&self, _e: &Emitter) -> Result<TokenStream> {
            todo!("EmitTokens: Targ")
        }
    }

    impl EmitTokens for ast::Tprim {
        fn emit_tokens(&self, _e: &Emitter) -> Result<TokenStream> {
            todo!("EmitTokens: Tprim")
        }
    }

    // All of these 'for Vec' impls could be collapsed into one main and a
    // couple overrides if they just finished specialization...

    impl EmitTokens for Vec<ast::Catch> {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_sequence_helper(self, e)
        }
    }

    impl EmitTokens for Vec<ast::Expr> {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_sequence_helper(self, e)
        }
    }

    impl EmitTokens for Vec<ast::Hint> {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_sequence_helper(self, e)
        }
    }

    impl EmitTokens for Vec<ast::Id> {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_sequence_helper(self, e)
        }
    }

    impl EmitTokens for Vec<ast::Stmt> {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_repl_sequence_helper(self, e, Emitter::lookup_stmt_replacement)
        }
    }

    impl EmitTokens for Vec<ast::Targ> {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_sequence_helper(self, e)
        }
    }

    impl EmitTokens for Vec<(ast::ParamKind, ast::Expr)> {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            emit_repl_sequence_helper(self, e, Emitter::lookup_param_replacement)
        }
    }

    impl EmitTokens for Vec<ast::Refinement> {
        fn emit_tokens(&self, _e: &Emitter) -> Result<TokenStream> {
            todo!("EmitTokens: Refinements")
        }
    }

    impl<T0, T1> EmitTokens for (T0, T1)
    where
        T0: EmitTokens,
        T1: EmitTokens,
    {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            let t0 = self.0.emit_tokens(e)?;
            let t1 = self.1.emit_tokens(e)?;
            Ok(quote!((#t0, #t1)))
        }
    }

    impl<T0, T1, T2> EmitTokens for (T0, T1, T2)
    where
        T0: EmitTokens,
        T1: EmitTokens,
        T2: EmitTokens,
    {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            let t0 = self.0.emit_tokens(e)?;
            let t1 = self.1.emit_tokens(e)?;
            let t2 = self.2.emit_tokens(e)?;
            Ok(quote!((#t0, #t1, #t2)))
        }
    }

    impl<T0, T1, T2, T3> EmitTokens for (T0, T1, T2, T3)
    where
        T0: EmitTokens,
        T1: EmitTokens,
        T2: EmitTokens,
        T3: EmitTokens,
    {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            let t0 = self.0.emit_tokens(e)?;
            let t1 = self.1.emit_tokens(e)?;
            let t2 = self.2.emit_tokens(e)?;
            let t3 = self.3.emit_tokens(e)?;
            Ok(quote!((#t0, #t1, #t2, #t3)))
        }
    }

    impl<T0, T1, T2, T3, T4> EmitTokens for (T0, T1, T2, T3, T4)
    where
        T0: EmitTokens,
        T1: EmitTokens,
        T2: EmitTokens,
        T3: EmitTokens,
        T4: EmitTokens,
    {
        fn emit_tokens(&self, e: &Emitter) -> Result<TokenStream> {
            let t0 = self.0.emit_tokens(e)?;
            let t1 = self.1.emit_tokens(e)?;
            let t2 = self.2.emit_tokens(e)?;
            let t3 = self.3.emit_tokens(e)?;
            let t4 = self.4.emit_tokens(e)?;
            Ok(quote!((#t0, #t1, #t2, #t3, #t4)))
        }
    }
}

#[cfg(test)]
mod tests {
    use macro_test_util::assert_pat_eq;

    use super::*;

    #[test]
    fn test_basic() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(pos = p, "#foo + $bar")),
            quote!({
                use oxidized::ast::*;
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
            hack_expr_impl.parse2(quote!(
                pos = pos(),
                r#"#obj_lvar->#meth_lvar(...#{lvar(args_var)})"#
            )),
            quote!({
                use oxidized::ast::*;
                let __hygienic_pos: Pos = pos();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new((
                        Expr(
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
                        vec![],
                        vec![],
                        Some({
                            let tmp: LocalId = args_var;
                            Expr(
                                (),
                                pos().clone(),
                                Expr_::Lvar(Box::new(Lid(pos().clone(), tmp))),
                            )
                        }),
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex2() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(
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
                use oxidized::ast::*;
                let __hygienic_pos: Pos = pos();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new((
                        Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(
                                __hygienic_pos.clone(),
                                "\\__SystemLib\\dynamic_meth_caller".to_owned(),
                            ))),
                        ),
                        vec![],
                        vec![
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
                        None,
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex3() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(
                pos = pos(),
                r#"\__SystemLib\meth_caller(#{str(clone(mangle_name))})"#
            )),
            quote!({
                use oxidized::ast::*;
                let __hygienic_pos: Pos = pos();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new((
                        Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(
                                __hygienic_pos.clone(),
                                "\\__SystemLib\\meth_caller".to_owned(),
                            ))),
                        ),
                        vec![],
                        vec![(ParamKind::Pnormal, {
                            let tmp: String = mangle_name.to_owned();
                            Expr((), pos().clone(), Expr_::String(tmp.into()))
                        })],
                        None,
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex4() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(
                pos = pos(),
                r#"\HH\invariant(
                       \is_a(#{clone(obj_lvar)}, #{str(clone(cls), pc)}),
                       #{str(msg)}
                   )
                "#
            )),
            quote!({
                use oxidized::ast::*;
                let __hygienic_pos: Pos = pos();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new((
                        Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(
                                __hygienic_pos.clone(),
                                "\\HH\\invariant".to_owned(),
                            ))),
                        ),
                        vec![],
                        vec![
                            (
                                ParamKind::Pnormal,
                                Expr(
                                    (),
                                    __hygienic_pos.clone(),
                                    Expr_::Call(Box::new((
                                        Expr(
                                            (),
                                            __hygienic_pos.clone(),
                                            Expr_::Id(Box::new(Id(
                                                __hygienic_pos.clone(),
                                                "\\is_a".to_owned(),
                                            ))),
                                        ),
                                        vec![],
                                        vec![
                                            (ParamKind::Pnormal, {
                                                let tmp: Expr = obj_lvar.clone();
                                                tmp
                                            }),
                                            (ParamKind::Pnormal, {
                                                let tmp: String = cls.to_owned();
                                                Expr((), pc.clone(), Expr_::String(tmp.into()))
                                            }),
                                        ],
                                        None,
                                    ))),
                                ),
                            ),
                            (ParamKind::Pnormal, {
                                let tmp: String = msg;
                                Expr((), pos().clone(), Expr_::String(tmp.into()))
                            }),
                        ],
                        None,
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex5() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!(
                pos = pos(),
                r#"#obj_lvar->#{id(clone(fname), pf)}(...#{lvar(args_var)})"#
            )),
            quote!({
                use oxidized::ast::*;
                let __hygienic_pos: Pos = pos();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new((
                        Expr(
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
                        vec![],
                        vec![],
                        Some({
                            let tmp: LocalId = args_var;
                            Expr(
                                (),
                                pos().clone(),
                                Expr_::Lvar(Box::new(Lid(pos().clone(), tmp))),
                            )
                        }),
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex6() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!("echo #{str(tail)}")),
            quote!({
                use oxidized::ast::*;
                let __hygienic_pos: Pos = Pos::NONE;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new((
                        Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Id(Box::new(Id(__hygienic_pos.clone(), "echo".to_owned()))),
                        ),
                        vec![],
                        vec![(ParamKind::Pnormal, {
                            let tmp: String = tail;
                            Expr((), Pos::NONE.clone(), Expr_::String(tmp.into()))
                        })],
                        None,
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex7() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!("parent::__xhpAttributeDeclaration()")),
            quote!({
                use oxidized::ast::*;
                let __hygienic_pos: Pos = Pos::NONE;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new((
                        Expr(
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
                        vec![],
                        vec![],
                        None,
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex8() {
        assert_pat_eq(
            hack_expr_impl.parse2(quote!("#{id(s)}::__xhpAttributeDeclaration()")),
            quote!({
                use oxidized::ast::*;
                let __hygienic_pos: Pos = Pos::NONE;
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Expr(
                    (),
                    __hygienic_pos.clone(),
                    Expr_::Call(Box::new((
                        Expr(
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
                                            Pos::NONE.clone(),
                                            Expr_::Id(Box::new(Id(Pos::NONE.clone(), tmp))),
                                        )
                                    }),
                                ),
                                (
                                    __hygienic_pos.clone(),
                                    "__xhpAttributeDeclaration".to_owned(),
                                ),
                            ))),
                        ),
                        vec![],
                        vec![],
                        None,
                    ))),
                );
                __hygienic_tmp
            }),
        );
    }

    #[test]
    fn test_ex9() {
        assert_pat_eq(
            hack_stmt_impl.parse2(quote!(
                "if (#{lvar(clone(name))} is __uninitSentinel) { unset(#{lvar(name)}); }"
            )),
            quote!({
                use oxidized::ast::*;
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
                                        Pos::NONE.clone(),
                                        Expr_::Lvar(Box::new(Lid(Pos::NONE.clone(), tmp))),
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
                                Expr_::Call(Box::new((
                                    Expr(
                                        (),
                                        __hygienic_pos.clone(),
                                        Expr_::Id(Box::new(Id(
                                            __hygienic_pos.clone(),
                                            "unset".to_owned(),
                                        ))),
                                    ),
                                    vec![],
                                    vec![(ParamKind::Pnormal, {
                                        let tmp: LocalId = name;
                                        Expr(
                                            (),
                                            Pos::NONE.clone(),
                                            Expr_::Lvar(Box::new(Lid(Pos::NONE.clone(), tmp))),
                                        )
                                    })],
                                    None,
                                ))),
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
            hack_stmt_impl.parse2(quote!(
                pos = p(),
                r#"if (\__SystemLib\__debugger_is_uninit(#{lvar(clone(name))})) {
                       #{lvar(name)} = new __uninitSentinel();
                     }
                "#
            )),
            quote!({
                use oxidized::ast::*;
                let __hygienic_pos: Pos = p();
                #[allow(clippy::redundant_clone)]
                let __hygienic_tmp = Stmt(
                    __hygienic_pos.clone(),
                    Stmt_::If(Box::new((
                        Expr(
                            (),
                            __hygienic_pos.clone(),
                            Expr_::Call(Box::new((
                                Expr(
                                    (),
                                    __hygienic_pos.clone(),
                                    Expr_::Id(Box::new(Id(
                                        __hygienic_pos.clone(),
                                        "\\__SystemLib\\__debugger_is_uninit".to_owned(),
                                    ))),
                                ),
                                vec![],
                                vec![(ParamKind::Pnormal, {
                                    let tmp: LocalId = name.clone();
                                    Expr(
                                        (),
                                        p().clone(),
                                        Expr_::Lvar(Box::new(Lid(p().clone(), tmp))),
                                    )
                                })],
                                None,
                            ))),
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
                                            p().clone(),
                                            Expr_::Lvar(Box::new(Lid(p().clone(), tmp))),
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
            hack_stmt_impl.parse2(quote!(
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
                use oxidized::ast::*;
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
                                Lid(p().clone(), tmp)
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
            hack_stmts_impl.parse2(quote!(
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
                                            Expr_::Call(Box::new((
                                                Expr(
                                                    (),
                                                    __hygienic_pos.clone(),
                                                    Expr_::Id(Box::new(Id(
                                                        __hygienic_pos.clone(),
                                                        "__SystemLib\\merge_xhp_attr_declarations"
                                                            .to_owned()
                                                    )))
                                                ),
                                                vec![],
                                                std::iter::empty()
                                                    .chain(args.into_iter())
                                                    .collect::<Vec<_>>(),
                                                None
                                            )))
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
                    use oxidized::ast::*;
                    let __hygienic_pos: Pos = Pos::NONE;
                    #[allow(clippy::redundant_clone)]
                    let __hygienic_tmp = vec![#stmt1, #stmt2, #stmt3];
                    __hygienic_tmp
                })
            },
        );
    }
}
