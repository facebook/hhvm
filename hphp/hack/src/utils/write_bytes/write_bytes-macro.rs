// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

use std::borrow::Cow;
use std::collections::HashMap;
use std::collections::HashSet;

use proc_macro2::Literal;
use proc_macro2::Span;
use proc_macro2::TokenStream;
use quote::quote;
use syn::parse::Parser;
use syn::punctuated::Punctuated;
use syn::spanned::Spanned;
use syn::Error;
use syn::Expr;
use syn::LitByteStr;
use syn::Token;

type Result<T> = std::result::Result<T, Error>;

/// Macro like write!() but for use with byte strings.
#[cfg(not(test))]
#[proc_macro]
pub fn write_bytes(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    let input = TokenStream::from(input);
    match write_bytes_macro(input) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

const DEBUG: bool = false;

fn write_bytes_macro(input: TokenStream) -> Result<TokenStream> {
    if DEBUG {
        eprintln!("INPUT: {}", input);
    }

    let parser = Punctuated::<Expr, Token![,]>::parse_terminated;
    let root = parser.parse2(input)?;

    let mut it = root.iter();
    let stream = it
        .next()
        .ok_or_else(|| Error::new(Span::call_site(), "missing stream argument"))?;

    let format_string = read_format_string(&mut it)?;
    let (named, positional) = read_args(it)?;
    let mut named_used: HashSet<String> = HashSet::new();
    let mut positional_used: HashSet<usize> = HashSet::new();

    let this_crate = quote!(::write_bytes);

    let mut args_vars = TokenStream::new();
    let mut named_idx: HashMap<String, usize> = HashMap::new();
    for expr in &positional {
        args_vars.extend(quote!(&#expr,));
    }
    for (i, (name, expr)) in named.iter().enumerate() {
        args_vars.extend(quote!(&#expr,));
        let _i: usize = i + positional.len();
        named_idx.insert(name.to_owned(), i + positional.len());
    }

    let mut literals = TokenStream::new();
    let mut args_values = TokenStream::new();
    let mut cur = Vec::new();
    let mut next_pos = 0;
    let mut it = format_string.value().into_iter().peekable();
    let span = format_string.span();
    while let Some(c) = it.next() {
        if c == b'{' {
            if it.peek().copied() == Some(b'{') {
                it.next();
                cur.push(b'{');
                continue;
            }

            // We need to always do this - even if it's blank.
            let cur_bs = LitByteStr::new(&cur, span);
            literals.extend(quote!(#cur_bs,));
            cur.clear();

            let spec = read_fmt_spec(&mut it, span)?;
            let arg_idx = match spec.name {
                FmtSpecName::Unspecified => {
                    let pos = next_pos;
                    next_pos += 1;
                    if pos >= positional.len() {
                        return Err(Error::new(
                            span,
                            format!(
                                "{} positional arguments in format string, but there is {} argument",
                                pos + 1,
                                positional.len()
                            ),
                        ));
                    }
                    positional_used.insert(pos);
                    pos
                }
                FmtSpecName::Positional(pos) => {
                    if pos >= positional.len() {
                        return Err(Error::new(
                            span,
                            format!("Invalid reference to positional argument {}", pos),
                        ));
                    }
                    positional_used.insert(pos);
                    pos
                }
                FmtSpecName::Named(name) => {
                    if let Some(&pos) = named_idx.get(&name) {
                        named_used.insert(name);
                        pos
                    } else {
                        return Err(Error::new(
                            span,
                            format!("there is no argument named '{}'", name),
                        ));
                    }
                }
            };

            let fmt_spec = quote!(#this_crate::FmtSpec {});

            let arg_idx = Literal::usize_unsuffixed(arg_idx);
            args_values.extend(quote!(#this_crate::Argument::new(_args.#arg_idx, #fmt_spec), ));
        } else if c == b'}' {
            if let Some(nc) = it.next() {
                if nc == b'}' {
                    cur.push(b'}');
                    continue;
                }
            }
            return Err(Error::new(span, "unmatched '}' in format string"));
        } else {
            cur.push(c);
        }
    }
    if !cur.is_empty() {
        let cur_bs = LitByteStr::new(&cur, format_string.span());
        literals.extend(quote!(#cur_bs,));
    }

    if named_used.len() != named.len() {
        let unused = named
            .iter()
            .find(|(k, _v)| !named_used.contains(k))
            .unwrap();
        return Err(Error::new(unused.1.span(), "named argument never used"));
    }

    if positional_used.len() != positional.len() {
        let unused: usize = (0..positional.len())
            .find(|k| !positional_used.contains(k))
            .unwrap();
        return Err(Error::new(positional[unused].span(), "argument never used"));
    }

    let result = quote!(
        #this_crate::write_bytes_fmt(
            #stream,
            #this_crate::Arguments::new(
                &[#literals],
                &match (#args_vars) {
                    _args => [#args_values],
                }
            )
        )
    );

    if DEBUG {
        eprintln!("RESULT: {:?}\n{}", result, result);
    }

    Ok(result)
}

fn read_format_string<'a>(it: &mut impl Iterator<Item = &'a Expr>) -> Result<Cow<'a, LitByteStr>> {
    let fs = it
        .next()
        .ok_or_else(|| Error::new(Span::call_site(), "missing format argument string"))?;

    let bs = match fs {
        Expr::Lit(syn::ExprLit {
            attrs: _,
            lit: syn::Lit::ByteStr(bs),
        }) => Cow::Borrowed(bs),
        Expr::Lit(syn::ExprLit {
            attrs: _,
            lit: syn::Lit::Str(s),
        }) => Cow::Owned(LitByteStr::new(s.value().as_bytes(), s.span())),
        _ => {
            return Err(Error::new(
                fs.span(),
                "format argument must be a string literal",
            ));
        }
    };

    Ok(bs)
}

fn read_args<'a>(
    it: impl Iterator<Item = &'a Expr>,
) -> Result<(Vec<(String, &'a Expr)>, Vec<&'a Expr>)> {
    // positional, positional, positional, ..., name = named, name = named, ...

    let mut named = Vec::new();
    let mut positional = Vec::new();

    for expr in it {
        match expr {
            Expr::Assign(assign) => {
                let name = match &*assign.left {
                    Expr::Path(path) => path.path.get_ident().map(|id| format!("{}", id)),
                    _ => None,
                };
                let name = name.ok_or_else(|| {
                    Error::new(expr.span(), "identifier expected for named parameter name")
                })?;
                named.push((name, &*assign.right));
            }
            _ => {
                if !named.is_empty() {
                    return Err(Error::new(
                        expr.span(),
                        "positional arguments must be before named arguments",
                    ));
                }
                positional.push(expr);
            }
        }
    }

    Ok((named, positional))
}

#[derive(Debug)]
enum FmtSpecName {
    Unspecified,
    Positional(usize),
    Named(String),
}

#[derive(Debug)]
struct FmtSpec {
    name: FmtSpecName,
}

#[allow(clippy::todo)]
fn read_fmt_spec<I: Iterator<Item = u8>>(
    it: &mut std::iter::Peekable<I>,
    span: Span,
) -> Result<FmtSpec> {
    // {[integer|identifier][:[[fill]align][sign]['#']['0'][width]['.' precision]type]}
    let mut spec = FmtSpec {
        name: FmtSpecName::Unspecified,
    };

    fn is_ident(c: u8) -> bool {
        c.is_ascii_alphanumeric() || c == b'_'
    }

    if it.peek().copied().map_or(false, is_ident) {
        // integer | identifier
        let mut pos = String::new();
        while it.peek().copied().map_or(false, is_ident) {
            pos.push(it.next().unwrap() as char);
        }

        if pos.chars().next().unwrap().is_ascii_digit() {
            spec.name = FmtSpecName::Positional(
                pos.parse::<usize>()
                    .map_err(|e| Error::new(span, e.to_string()))?,
            );
        } else {
            // identifier
            spec.name = FmtSpecName::Named(pos);
        }
    }
    if it.peek().copied() == Some(b':') {
        // format spec
        todo!();
    }
    if let Some(c) = it.next() {
        if c != b'}' {
            return Err(Error::new(
                span,
                format!("Unexpected character '{}' in format string", c),
            ));
        }
    } else {
        return Err(Error::new(span, "Expected '}' in format string"));
    }

    Ok(spec)
}

#[cfg(test)]
mod test_helpers {
    use proc_macro2::TokenStream;
    use proc_macro2::TokenTree;
    use syn::Error;

    pub(crate) fn mismatch(
        ta: Option<TokenTree>,
        tb: Option<TokenTree>,
        ax: &TokenStream,
        bx: &TokenStream,
    ) -> ! {
        panic!(
            "Mismatch!\nLeft:  {}\nRight: {}\nwhen comparing\nLeft:  {}\nRight: {}\n",
            ta.map_or("None".into(), |t| t.to_string()),
            tb.map_or("None".into(), |t| t.to_string()),
            ax,
            bx
        );
    }

    pub(crate) fn assert_pat_eq(a: Result<TokenStream, Error>, b: TokenStream) {
        let a = match a {
            Err(err) => {
                panic!("Unexpected error '{}'", err);
            }
            Ok(ok) => ok,
        };

        fn inner_cmp(a: TokenStream, b: TokenStream, ax: &TokenStream, bx: &TokenStream) {
            let mut ia = a.into_iter();
            let mut ib = b.into_iter();

            loop {
                let t_a = ia.next();
                let t_b = ib.next();
                if t_a.is_none() && t_b.is_none() {
                    break;
                }
                if t_a.is_none() || t_b.is_none() {
                    mismatch(t_a, t_b, ax, bx);
                }
                let t_a = t_a.unwrap();
                let t_b = t_b.unwrap();

                match (&t_a, &t_b) {
                    (TokenTree::Ident(_), TokenTree::Ident(_))
                    | (TokenTree::Literal(_), TokenTree::Literal(_))
                    | (TokenTree::Punct(_), TokenTree::Punct(_))
                        if t_a.to_string() == t_b.to_string() => {}
                    (TokenTree::Group(ga), TokenTree::Group(gb)) => {
                        inner_cmp(ga.stream(), gb.stream(), ax, bx);
                    }
                    (TokenTree::Group(_), _)
                    | (TokenTree::Ident(_), _)
                    | (TokenTree::Punct(_), _)
                    | (TokenTree::Literal(_), _) => mismatch(Some(t_a), Some(t_b), ax, bx),
                }
            }
        }

        inner_cmp(a.clone(), b.clone(), &a, &b);
    }

    pub(crate) fn assert_error(a: Result<TokenStream, Error>, b: &str) {
        match a {
            Ok(a) => panic!("Expected error but got:\n{}", a),
            Err(e) => {
                let a = format!("{}", e);
                assert_eq!(a, b, "Incorrect error")
            }
        }
    }
}

#[cfg(test)]
// #[rustfmt::skip] // skip rustfmt because it tends to add unwanted tokens to our quotes.
#[allow(dead_code)]
mod test {
    use quote::quote;

    use super::*;
    use crate::test_helpers::assert_error;
    use crate::test_helpers::assert_pat_eq;

    #[test]
    fn test_basic() {
        assert_pat_eq(
            write_bytes_macro(quote!(w, b"text")),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"text",],
                    &match () {
                        _args => [],
                    }
                )
            )),
        );

        assert_pat_eq(
            write_bytes_macro(quote!(w, b"text",)),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"text",],
                    &match () {
                        _args => [],
                    }
                )
            )),
        );

        assert_pat_eq(
            write_bytes_macro(quote!(w, "text")),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"text",],
                    &match () {
                        _args => [],
                    }
                )
            )),
        );

        assert_error(write_bytes_macro(quote!()), "missing stream argument");

        assert_error(
            write_bytes_macro(quote!(w)),
            "missing format argument string",
        );

        assert_error(
            write_bytes_macro(quote!(w,)),
            "missing format argument string",
        );

        assert_error(
            write_bytes_macro(quote!(w, x)),
            "format argument must be a string literal",
        );
    }

    #[test]
    fn test_escape() {
        assert_pat_eq(
            write_bytes_macro(quote!(w, b"te\nxt")),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"te\nxt",],
                    &match () {
                        _args => [],
                    }
                )
            )),
        );

        assert_pat_eq(
            write_bytes_macro(quote!(w, b"te\\xt")),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"te\\xt",],
                    &match () {
                        _args => [],
                    }
                )
            )),
        );

        assert_pat_eq(
            write_bytes_macro(quote!(w, b"te{{xt")),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"te{xt",],
                    &match () {
                        _args => [],
                    }
                )
            )),
        );

        assert_pat_eq(
            write_bytes_macro(quote!(w, b"te}}xt")),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"te}xt",],
                    &match () {
                        _args => [],
                    }
                )
            )),
        );

        assert_error(
            write_bytes_macro(quote!(w, b"abc{", arg)),
            "Expected '}' in format string",
        );

        assert_error(
            write_bytes_macro(quote!(w, b"abc}def", arg)),
            "unmatched '}' in format string",
        );
    }

    #[test]
    fn test_pos() {
        assert_pat_eq(
            write_bytes_macro(quote!(w, b"abc{}def", arg)),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"abc", b"def",],
                    &match (&arg,) {
                        _args => [::write_bytes::Argument::new(
                            _args.0,
                            ::write_bytes::FmtSpec {}
                        ),],
                    }
                )
            )),
        );

        assert_pat_eq(
            write_bytes_macro(quote!(w, b"abc{}", arg)),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"abc",],
                    &match (&arg,) {
                        _args => [::write_bytes::Argument::new(
                            _args.0,
                            ::write_bytes::FmtSpec {}
                        ),],
                    }
                )
            )),
        );

        assert_pat_eq(
            write_bytes_macro(quote!(w, b"abc{}", 5)),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"abc",],
                    &match (&5,) {
                        _args => [::write_bytes::Argument::new(
                            _args.0,
                            ::write_bytes::FmtSpec {}
                        ),],
                    }
                )
            )),
        );

        assert_pat_eq(
            write_bytes_macro(quote!(w, b"abc{1}def{0}ghi", arg0, arg1)),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"abc", b"def", b"ghi",],
                    &match (&arg0, &arg1,) {
                        _args => [
                            ::write_bytes::Argument::new(_args.1, ::write_bytes::FmtSpec {}),
                            ::write_bytes::Argument::new(_args.0, ::write_bytes::FmtSpec {}),
                        ],
                    }
                )
            )),
        );

        assert_pat_eq(
            write_bytes_macro(quote!(w, b"abc{0}{1}ghi", arg0, arg1)),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"abc", b"", b"ghi",],
                    &match (&arg0, &arg1,) {
                        _args => [
                            ::write_bytes::Argument::new(_args.0, ::write_bytes::FmtSpec {}),
                            ::write_bytes::Argument::new(_args.1, ::write_bytes::FmtSpec {}),
                        ],
                    }
                )
            )),
        );

        assert_pat_eq(
            write_bytes_macro(quote!(w, b"{}abc", arg)),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"", b"abc",],
                    &match (&arg,) {
                        _args => [::write_bytes::Argument::new(
                            _args.0,
                            ::write_bytes::FmtSpec {}
                        ),],
                    }
                )
            )),
        );

        assert_error(
            write_bytes_macro(quote!(w, b"abc{1}def", arg0)),
            "Invalid reference to positional argument 1",
        );

        assert_error(
            write_bytes_macro(quote!(w, b"abc{}def", foo = bar, arg0)),
            "positional arguments must be before named arguments",
        );

        assert_error(
            write_bytes_macro(quote!(w, b"abc{}def{}ghi", arg0)),
            "2 positional arguments in format string, but there is 1 argument",
        );

        assert_error(
            write_bytes_macro(quote!(w, b"abc{}def{}ghi", arg0, arg1, arg2)),
            "argument never used",
        );
    }

    #[test]
    fn test_named() {
        assert_pat_eq(
            write_bytes_macro(quote!(w, b"abc{foo}def", foo = arg)),
            quote!(::write_bytes::write_bytes_fmt(
                w,
                ::write_bytes::Arguments::new(
                    &[b"abc", b"def",],
                    &match (&arg,) {
                        _args => [::write_bytes::Argument::new(
                            _args.0,
                            ::write_bytes::FmtSpec {}
                        ),],
                    }
                )
            )),
        );

        assert_error(
            write_bytes_macro(quote!(w, b"abc{bar}def", foo = arg)),
            "there is no argument named 'bar'",
        );

        assert_error(
            write_bytes_macro(quote!(w, b"abcdef", foo = arg)),
            "named argument never used",
        );
    }
}
