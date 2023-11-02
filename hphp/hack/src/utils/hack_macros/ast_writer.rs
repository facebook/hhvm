// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#![allow(clippy::todo)]

use std::collections::HashMap;
use std::fmt;
use std::rc::Rc;

use proc_macro2::Delimiter;
use proc_macro2::Group;
use proc_macro2::Ident;
use proc_macro2::Literal;
use proc_macro2::Punct;
use proc_macro2::Spacing;
use proc_macro2::Span;
use proc_macro2::TokenStream;
use proc_macro2::TokenTree;
use quote::quote;
use quote::quote_spanned;
use quote::ToTokens;
use quote::TokenStreamExt;
use serde::ser;
use serde::ser::Serialize;
use thiserror::Error;

type Result<T = AstValue, E = AstError> = std::result::Result<T, E>;

/// Given an AST write out a TokenStream that builds it.  Processes variable
/// replacements in the AST while traversing.
///
/// We use Serialize to drive the conversion. This causes a couple of oddities:
///
///    1. Box<T> is transparent to serde - `F(Box<T>)` is serialized as `F(T)`
///    so we need to synthetically add `Box` where we know it's needed.
///
///    2. We can't "look down" at a tree during serialization (we only know
///    something is a 'T' and can't downcast_ref it). So if a node is a possible
///    substitution ref we need to keep it in a form that preserves that
///    information and pass it up until we determine that it is or isn't
///    actually part of a substitution.
///
pub(crate) fn write_ast<T: Serialize + fmt::Debug>(
    exports: syn::Path,
    span: Span,
    replacements: HashMap<String, Replacement>,
    pos_src: TokenStream,
    pos_id: TokenStream,
    t: T,
) -> Result<TokenStream, AstError> {
    let state = Rc::new(AstState::new(exports, pos_id, replacements));
    let exports = &state.exports;

    let t = t.serialize(AstWriter::new(state.clone()))?.into_tokens()?;

    // If this fires it means that we tried to swap out a replacement
    // but the emitter didn't convert it properly.
    debug_assert!(!t.to_string().contains("__hack_repl_"));
    let pos_id = &state.pos;
    // We assign to a temp and then return so that we can silence some
    // clippy lints (you can't use an attribute on an expression).
    let tmp = Ident::new("__hygienic_tmp", span);
    Ok(quote!({
        use #exports::ast::*;
        let #pos_id: Pos = #pos_src;
        #[allow(clippy::redundant_clone)]
        let #tmp = #t;
        #tmp
    }))
}

pub(crate) fn hygienic_pos(span: Span) -> TokenStream {
    Ident::new("__hygienic_pos", span).to_token_stream()
}

/// Describes replacements that should occur while we build the AST.
#[derive(Debug, Clone)]
pub(crate) enum Replacement {
    // #foo
    Simple {
        pat: TokenStream,
        span: Span,
    },
    // #{as_expr(foo)}
    AsExpr {
        pat: TokenStream,
        span: Span,
    },
    // #{id(foo)}
    Id {
        pat: TokenStream,
        pos: TokenStream,
        span: Span,
    },
    // #{lvar(foo)}
    Lvar {
        pat: TokenStream,
        pos: TokenStream,
        span: Span,
    },
    // #{foo*}
    Repeat {
        pat: TokenStream,
        span: Span,
    },
    // #{str(foo)}
    Str {
        pat: TokenStream,
        pos: TokenStream,
        span: Span,
    },
}

impl Replacement {
    fn into_expr(self) -> Result<TokenStream> {
        Ok(match self {
            Replacement::Simple { pat, span } => {
                // Variable should already be an Expr.
                let tmp = Ident::new("tmp", span);
                quote_spanned!(span=> {
                    let #tmp: Expr = #pat;
                    #tmp
                })
            }
            Replacement::AsExpr { .. } => todo!(),
            Replacement::Id {
                ref pat,
                ref pos,
                span,
            } => {
                // Variable should be a String and we want to build an
                // Expr_::Ident from it.
                let tmp = Ident::new("tmp", span);
                quote_spanned!(span=> {
                    let #tmp: String = #pat;
                    Expr((), #pos.clone(), Expr_::Id(Box::new(Id(#pos.clone(), #tmp))))
                })
            }
            Replacement::Lvar {
                ref pat,
                ref pos,
                span,
            } => {
                // Variable should be a LocalId and we want to build an
                // Expr_::Lvar from it.
                let tmp = Ident::new("tmp", span);
                quote_spanned!(span=> {
                    let #tmp: LocalId = #pat;
                    Expr((), #pos.clone(), Expr_::Lvar(Box::new(Lid(#pos.clone(), #tmp))))
                })
            }
            Replacement::Str {
                ref pat,
                ref pos,
                span,
            } => {
                // Variable should be a String and we want to build an
                // Expr_::String from it.
                let tmp = Ident::new("tmp", span);
                quote_spanned!(span=> {
                    let #tmp: String = #pat;
                    Expr((), #pos.clone(), Expr_::String(#tmp.into()))
                })
            }
            Replacement::Repeat { span, .. } => {
                // This can't be turned into a simple Expr.
                return Err(
                    syn::Error::new(span, "Unable to expand repeat in Expr position").into(),
                );
            }
        })
    }
}

#[derive(Clone, Debug)]
enum AstValue {
    // This node produced raw tokens.
    Tokens(TokenStream),
    // This node represents a replacement state.
    Replace(ReplaceState),
}

// Because of the way serialization works we need to view replacements as a
// little state machine where we use the state to determine if we have a
// replacement or not.
//
// For example - for the expression:
//
//   #a + $b
//
// We get the AST:
//
//   Expr((), Pos, Binop(Binop { bop: Plus, lhs: Expr((), Pos, Lvar(Lid(Pos, (0, "$__hack_repl_0")))), rhs: Expr((), Pos, Lvar(Lid(Pos, (0, "$b")))) }))
//
// As we serialize we see:
//
//   serialize_str("$__hack_repl_0")
//
// so we return a `Str` indicating that we've seen some replacement. Later when
// we get the tuple:
//
//   serialize_tuple((0, Str))
//
// we turn that into a Tuple.  This continues until we either see:
//
//   serialize_tuple_struct("Expr", ((), _, Lvar))
//
// and we replace it with the desired tokens or we unroll it back into the
// original token stream (other states are possible as well).
//
//
#[derive(Clone, Debug)]
enum ReplaceState {
    /// AsExpr::AsV(Expr)
    AsV(Replacement),
    /// Expr(_, _, Lvar)
    Expr(Replacement),
    /// Stmt_::Expr(Expr)
    ExprStmt(Replacement),
    /// Lid(_, Tuple)
    Lid(Replacement),
    /// Expr_::Lvar(Lid)
    Lvar(Replacement),
    /// Stmt(_, ExprStmt)
    Stmt(Replacement),
    /// "$__hack_repl_0"
    Str(Replacement),
    /// "(_, Str)
    Tuple(Replacement),
    /// (_, Expr)
    Tuple2(Box<(AstValue, Replacement)>),
}

impl AstValue {
    fn into_tokens(self) -> Result<TokenStream> {
        Ok(match self {
            AstValue::Tokens(s) => s,
            AstValue::Replace(ReplaceState::AsV(repl)) => match repl {
                Replacement::AsExpr { pat, span } => {
                    let tmp = Ident::new("tmp", Span::mixed_site());
                    quote_spanned!(span=> {
                        let #tmp: AsExpr = #pat;
                        #tmp
                    })
                }
                Replacement::Id { .. } => todo!(),
                Replacement::Lvar { .. } => todo!(),
                Replacement::Repeat { .. } => todo!(),
                Replacement::Simple { pat, span } => {
                    // This is something like: `foreach ($x as <replacement>)`
                    // - they probably meant to replace the AsV expr rather than
                    // the binding.
                    let tmp = Ident::new("tmp", Span::mixed_site());
                    quote_spanned!(span=> {
                        let #tmp: Expr = #pat;
                        AsExpr::AsV(tmp)
                    })
                }
                Replacement::Str { .. } => todo!(),
            },
            AstValue::Replace(ReplaceState::Str(_)) => todo!(),
            AstValue::Replace(ReplaceState::Tuple(_)) => todo!(),
            AstValue::Replace(ReplaceState::Lid(repl)) => match repl {
                Replacement::AsExpr { .. } => todo!(),
                Replacement::Id { .. } => todo!(),
                Replacement::Lvar { pat, pos, span } => {
                    let tmp = Ident::new("tmp", Span::mixed_site());
                    quote_spanned!(span=> {
                        let #tmp: LocalId = #pat;
                        Lid(#pos.clone(), #tmp)
                    })
                }
                Replacement::Repeat { .. } => todo!(),
                Replacement::Simple { .. } => todo!(),
                Replacement::Str { .. } => todo!(),
            },
            AstValue::Replace(ReplaceState::Lvar(_)) => todo!(),
            AstValue::Replace(ReplaceState::Expr(repl)) => repl.into_expr()?,
            AstValue::Replace(ReplaceState::ExprStmt(_)) => todo!(),
            AstValue::Replace(ReplaceState::Stmt(repl)) => match repl {
                Replacement::AsExpr { .. } => todo!(),
                Replacement::Id { .. } => todo!(),
                Replacement::Lvar { .. } => todo!(),
                Replacement::Repeat { .. } => todo!(),
                Replacement::Simple { pat, span } => {
                    let tmp = Ident::new("tmp", span);
                    quote_spanned!(span=> {
                        let #tmp: Stmt = #pat;
                        #tmp
                    })
                }
                Replacement::Str { .. } => todo!(),
            },
            AstValue::Replace(ReplaceState::Tuple2(_)) => todo!(),
        })
    }
}

struct AstState {
    exports: syn::Path,
    pos: TokenStream,
    replacements: HashMap<String, Replacement>,
}

impl AstState {
    fn new(
        exports: syn::Path,
        pos: TokenStream,
        replacements: HashMap<String, Replacement>,
    ) -> Self {
        Self {
            exports,
            pos,
            replacements,
        }
    }

    fn lookup_replacement(&self, name: &str) -> Option<&Replacement> {
        self.replacements.get(name)
    }
}

struct AstWriter {
    state: Rc<AstState>,
}

impl AstWriter {
    fn new(state: Rc<AstState>) -> Self {
        Self { state }
    }
}

impl ser::Serializer for AstWriter {
    type Ok = AstValue;
    type Error = AstError;
    type SerializeSeq = SerializeSeq;
    type SerializeTuple = SerializeTuple;
    type SerializeTupleStruct = SerializeTupleStruct;
    type SerializeTupleVariant = SerializeTupleVariant;
    type SerializeMap = SerializeMap;
    type SerializeStruct = SerializeStruct;
    type SerializeStructVariant = SerializeStructVariant;

    fn serialize_bool(self, value: bool) -> Result {
        let span = Span::call_site();
        let mut s = TokenStream::new();
        s.push_ident(span, if value { "true" } else { "false" });
        Ok(AstValue::Tokens(s))
    }

    fn serialize_i8(self, _: i8) -> Result {
        todo!()
    }

    fn serialize_i16(self, _: i16) -> Result {
        todo!()
    }

    fn serialize_i32(self, _: i32) -> Result {
        todo!()
    }

    fn serialize_i64(self, i: i64) -> Result {
        Ok(AstValue::Tokens(
            TokenTree::Literal(Literal::isize_suffixed(i as isize)).into(),
        ))
    }

    fn serialize_u8(self, _: u8) -> Result {
        todo!()
    }

    fn serialize_u16(self, _: u16) -> Result {
        todo!()
    }

    fn serialize_u32(self, _: u32) -> Result {
        todo!()
    }

    fn serialize_u64(self, _: u64) -> Result {
        todo!()
    }

    fn serialize_f32(self, _: f32) -> Result {
        todo!()
    }

    fn serialize_f64(self, _: f64) -> Result {
        todo!()
    }

    fn serialize_char(self, _: char) -> Result {
        todo!()
    }

    fn serialize_str(self, value: &str) -> Result {
        if let Some(repl) = self.state.lookup_replacement(value) {
            return Ok(AstValue::Replace(ReplaceState::Str(repl.clone())));
        }

        let mut s = TokenStream::new();
        let span = Span::call_site();
        s.append(TokenTree::Literal(Literal::string(value)));
        s.push_dot();
        s.push_ident(span, "to_owned");
        s.push_paren_group(TokenStream::new());
        Ok(AstValue::Tokens(s))
    }

    fn serialize_bytes(self, _: &[u8]) -> Result {
        todo!()
    }

    fn serialize_none(self) -> Result {
        Ok(AstValue::Tokens(quote!(None)))
    }

    fn serialize_some<T: Serialize + ?Sized>(self, value: &T) -> Result {
        let inner = value.serialize(AstWriter::new(self.state))?.into_tokens()?;

        let mut s = TokenStream::new();
        let span = Span::call_site();
        s.push_ident(span, "Some");
        s.push_paren_group(inner);
        Ok(AstValue::Tokens(s))
    }

    fn serialize_unit(self) -> Result {
        let mut s = TokenStream::new();
        s.push_paren_group(TokenStream::new());
        Ok(AstValue::Tokens(s))
    }

    fn serialize_unit_struct(self, _: &'static str) -> Result {
        todo!()
    }

    fn serialize_unit_variant(
        self,
        name: &'static str,
        _index: u32,
        variant: &'static str,
    ) -> Result {
        let mut s = TokenStream::new();
        let span = Span::call_site();
        s.push_ident(span, name);
        s.push_colon2();
        s.push_ident(span, variant);
        Ok(AstValue::Tokens(s))
    }

    fn serialize_newtype_struct<T: Serialize + ?Sized>(
        self,
        name: &'static str,
        value: &T,
    ) -> Result {
        if name == "Pos" {
            // Use the user-supplied `pos` instead of the original Pos (which
            // would be from our macro's AST parse).
            let pos = &self.state.pos;
            return Ok(AstValue::Tokens(quote!(#pos.clone())));
        }

        let value = value.serialize(AstWriter::new(self.state))?;

        let value = match (name, value) {
            ("Block" | "FinallyBlock", AstValue::Replace(_)) => {
                todo!();
            }
            (_, value) => value.into_tokens()?,
        };

        let span = Span::call_site();
        let mut s = TokenStream::new();
        s.push_ident(span, name);
        s.push_paren_group(value);
        Ok(AstValue::Tokens(s))
    }

    fn serialize_newtype_variant<T: Serialize + ?Sized>(
        self,
        name: &'static str,
        _variant_index: u32,
        variant: &'static str,
        value: &T,
    ) -> Result {
        let inner = value.serialize(AstWriter::new(self.state))?;

        let inner = match (name, variant, inner) {
            ("AsExpr", "AsV", AstValue::Replace(ReplaceState::Expr(repl))) => {
                return Ok(AstValue::Replace(ReplaceState::AsV(repl)));
            }
            ("Expr_", "Lvar", AstValue::Replace(ReplaceState::Lid(repl))) => {
                return Ok(AstValue::Replace(ReplaceState::Lvar(repl)));
            }
            ("Stmt_", "Expr", AstValue::Replace(ReplaceState::Expr(repl))) => {
                return Ok(AstValue::Replace(ReplaceState::ExprStmt(repl)));
            }
            (_, _, inner) => inner,
        };

        let mut s = TokenStream::new();
        let span = Span::call_site();
        s.push_ident(span, name);
        s.push_colon2();
        s.push_ident(span, variant);

        let inner = inner.into_tokens()?;

        // Handle boxing.
        let inner = match (name, variant) {
            ("Expr_", "Float")
            | ("Expr_", "Int")
            | ("Expr_", "List")
            | ("Expr_", "Shape")
            | ("Expr_", "String")
            | ("Expr_", "String2")
            | ("Expr_", "Tuple") => inner,
            ("Stmt_", "Block") | ("Expr_", _) | ("Stmt_", _) => box_stream(inner, span),
            _ => inner,
        };
        s.push_paren_group(inner);
        Ok(AstValue::Tokens(s))
    }

    fn serialize_seq(self, _: std::option::Option<usize>) -> Result<SerializeSeq> {
        Ok(SerializeSeq {
            seq: Vec::new(),
            state: self.state,
            has_repeat: false,
        })
    }

    fn serialize_tuple(self, _: usize) -> Result<SerializeTuple> {
        Ok(SerializeTuple {
            state: self.state,
            fields: Vec::new(),
        })
    }

    fn serialize_tuple_struct(
        self,
        name: &'static str,
        _length: usize,
    ) -> Result<SerializeTupleStruct> {
        Ok(SerializeTupleStruct {
            state: self.state,
            name,
            fields: Vec::new(),
        })
    }

    fn serialize_tuple_variant(
        self,
        name: &'static str,
        _variant_index: u32,
        variant: &'static str,
        _len: usize,
    ) -> Result<SerializeTupleVariant> {
        Ok(SerializeTupleVariant {
            state: self.state,
            name,
            variant,
            fields: Vec::new(),
        })
    }

    fn serialize_map(self, _: std::option::Option<usize>) -> Result<SerializeMap> {
        todo!()
    }

    fn serialize_struct(self, name: &'static str, _len: usize) -> Result<SerializeStruct> {
        Ok(SerializeStruct {
            state: self.state,
            name,
            fields: Default::default(),
        })
    }

    fn serialize_struct_variant(
        self,
        _name: &'static str,
        _variant_index: u32,
        _variant: &'static str,
        _len: usize,
    ) -> Result<SerializeStructVariant> {
        Ok(SerializeStructVariant {})
    }
}

struct SerializeMap {}

impl ser::SerializeMap for SerializeMap {
    type Ok = AstValue;
    type Error = AstError;

    fn serialize_key<T: Serialize + ?Sized>(&mut self, _: &T) -> Result<()> {
        todo!()
    }

    fn serialize_value<T: Serialize + ?Sized>(&mut self, _: &T) -> Result<()> {
        todo!()
    }

    fn end(self) -> Result {
        todo!()
    }
}

struct SerializeStruct {
    state: Rc<AstState>,
    name: &'static str,
    fields: Vec<(&'static str, AstValue)>,
}

impl ser::SerializeStruct for SerializeStruct {
    type Ok = AstValue;
    type Error = AstError;

    fn serialize_field<T: Serialize + ?Sized>(
        &mut self,
        name: &'static str,
        value: &T,
    ) -> Result<()> {
        let value = value.serialize(AstWriter::new(self.state.clone()))?;
        self.fields.push((name, value));
        Ok(())
    }

    fn end(self) -> Result {
        let span = Span::call_site();

        let mut inner = TokenStream::new();
        let mut it = self.fields.into_iter();
        if let Some((name, field)) = it.next() {
            inner.push_ident(span, name);
            inner.push_colon();
            inner.extend(field.into_tokens()?);
        }
        for (name, field) in it {
            inner.push_comma();
            inner.push_ident(span, name);
            inner.push_colon();
            inner.extend(field.into_tokens()?);
        }

        let mut s = TokenStream::new();
        s.push_ident(span, self.name);
        s.push_brace_group(inner);
        Ok(AstValue::Tokens(s))
    }
}

/// Helpers for TokenStream manipulation.
trait TokenStreamEx {
    fn push_boxed(&mut self, span: Span, inner: TokenStream);
    fn push_colon(&mut self);
    fn push_colon2(&mut self);
    fn push_comma(&mut self);
    fn push_comma_sequence(&mut self, seq: impl IntoIterator<Item = TokenStream>);
    fn push_dot(&mut self);
    fn push_ident(&mut self, span: Span, name: &str);
    fn push_brace_group(&mut self, inner: TokenStream);
    fn push_paren_group(&mut self, inner: TokenStream);
}

impl TokenStreamEx for TokenStream {
    fn push_boxed(&mut self, span: Span, inner: TokenStream) {
        self.push_ident(span, "Box");
        self.push_colon2();
        self.push_ident(span, "new");
        self.push_paren_group(inner);
    }

    fn push_colon(&mut self) {
        self.append(TokenTree::Punct(Punct::new(':', Spacing::Alone)));
    }

    fn push_colon2(&mut self) {
        self.append(TokenTree::Punct(Punct::new(':', Spacing::Joint)));
        self.append(TokenTree::Punct(Punct::new(':', Spacing::Alone)));
    }

    fn push_comma(&mut self) {
        self.append(TokenTree::Punct(Punct::new(',', Spacing::Alone)));
    }

    fn push_comma_sequence(&mut self, seq: impl IntoIterator<Item = TokenStream>) {
        let mut seq = seq.into_iter();
        if let Some(item) = seq.next() {
            self.extend(item);
        }
        for item in seq {
            self.push_comma();
            self.extend(item);
        }
    }

    fn push_dot(&mut self) {
        self.append(TokenTree::Punct(Punct::new('.', Spacing::Alone)));
    }

    fn push_ident(&mut self, span: Span, name: &str) {
        self.append(Ident::new(name, span))
    }

    fn push_brace_group(&mut self, inner: TokenStream) {
        self.append(TokenTree::Group(Group::new(Delimiter::Brace, inner)))
    }

    fn push_paren_group(&mut self, inner: TokenStream) {
        self.append(TokenTree::Group(Group::new(Delimiter::Parenthesis, inner)))
    }
}

fn box_stream(inner: TokenStream, span: Span) -> TokenStream {
    let mut b = TokenStream::new();
    b.push_boxed(span, inner);
    b
}

// A wrapper around syn::Error that provides Debug, Display, and Error which are
// needed for Serializer.
#[derive(Error, Debug)]
pub(crate) enum AstError {
    #[error("Custom error '{0}'")]
    Custom(String),
    #[error("Syntax error")]
    Syn(#[from] syn::Error),
}

impl ser::Error for AstError {
    fn custom<T: fmt::Display>(err: T) -> Self {
        Self::Custom(err.to_string())
    }
}

impl From<AstError> for syn::Error {
    fn from(err: AstError) -> Self {
        match err {
            AstError::Custom(s) => Self::new(Span::call_site(), s),
            AstError::Syn(s) => s,
        }
    }
}

struct SerializeSeq {
    seq: Vec<AstValue>,
    state: Rc<AstState>,
    has_repeat: bool,
}

impl ser::SerializeSeq for SerializeSeq {
    type Ok = AstValue;
    type Error = AstError;

    fn serialize_element<T: Serialize + ?Sized>(&mut self, value: &T) -> Result<()> {
        let inner = value.serialize(AstWriter::new(self.state.clone()))?;
        let has_repeat = match &inner {
            AstValue::Replace(
                ReplaceState::Expr(repl)
                | ReplaceState::Stmt(repl)
                | ReplaceState::Tuple2(box (_, repl)),
            ) => {
                matches!(repl, Replacement::Repeat { .. })
            }
            _ => false,
        };
        self.has_repeat |= has_repeat;
        self.seq.push(inner);
        Ok(())
    }

    fn end(self) -> Result {
        // vec![a, b, c...]

        match self.has_repeat {
            false => {
                let seq = self
                    .seq
                    .into_iter()
                    .map(AstValue::into_tokens)
                    .collect::<Result<Vec<TokenStream>>>()?;
                Ok(AstValue::Tokens(quote!(vec![#(#seq),*])))
            }
            true => {
                // #{args*} replacement...
                // Generates something that looks like:
                //   std::iter::empty()
                //     .chain([a, b, c].into_iter())
                //     .chain(args.into_iter())
                //     .chain([d, e, f].into_iter())
                //     .collect::<Vec<_>>()

                let mut outer = TokenStream::new();

                fn flush(outer: &mut TokenStream, cur: &mut Vec<TokenStream>) {
                    if !cur.is_empty() {
                        outer.extend(quote!(.chain([#(#cur),*].into_iter())));
                        cur.clear();
                    }
                }

                let mut cur = Vec::new();
                for item in self.seq {
                    match item {
                        AstValue::Replace(
                            ReplaceState::Expr(Replacement::Repeat { pat, .. })
                            | ReplaceState::Stmt(Replacement::Repeat { pat, .. }),
                        )
                        | AstValue::Replace(ReplaceState::Tuple2(box (
                            _,
                            Replacement::Repeat { pat, .. },
                        ))) => {
                            flush(&mut outer, &mut cur);
                            outer.extend(quote!(.chain(#pat.into_iter())));
                        }
                        _ => cur.push(item.into_tokens()?),
                    }
                }

                flush(&mut outer, &mut cur);
                Ok(AstValue::Tokens(
                    quote!(std::iter::empty() #outer .collect::<Vec<_>>()),
                ))
            }
        }
    }
}

struct SerializeTuple {
    state: Rc<AstState>,
    fields: Vec<AstValue>,
}

impl ser::SerializeTuple for SerializeTuple {
    type Ok = AstValue;
    type Error = AstError;

    fn serialize_element<T: Serialize + ?Sized>(&mut self, f: &T) -> Result<()> {
        let inner = f.serialize(AstWriter::new(self.state.clone()))?;
        self.fields.push(inner);
        Ok(())
    }

    fn end(self) -> Result {
        Ok(match &self.fields[..] {
            [
                v0,
                AstValue::Replace(ReplaceState::Expr(repl @ Replacement::Repeat { .. })),
            ] => AstValue::Replace(ReplaceState::Tuple2(Box::new((v0.clone(), repl.clone())))),
            [_, AstValue::Replace(ReplaceState::Str(repl))] => {
                AstValue::Replace(ReplaceState::Tuple(repl.clone()))
            }
            _ => {
                let mut inner = TokenStream::new();
                inner.push_comma_sequence(
                    self.fields
                        .into_iter()
                        .map(AstValue::into_tokens)
                        .collect::<Result<Vec<TokenStream>>>()?,
                );

                let mut s = TokenStream::new();
                s.push_paren_group(inner);
                AstValue::Tokens(s)
            }
        })
    }
}

struct SerializeTupleStruct {
    state: Rc<AstState>,
    name: &'static str,
    fields: Vec<AstValue>,
}

impl ser::SerializeTupleStruct for SerializeTupleStruct {
    type Ok = AstValue;
    type Error = AstError;

    fn serialize_field<T: Serialize + ?Sized>(&mut self, f: &T) -> Result<()> {
        let inner = f.serialize(AstWriter::new(self.state.clone()))?;
        self.fields.push(inner);
        Ok(())
    }

    fn end(self) -> Result {
        match (self.name, &self.fields[..]) {
            (
                "Expr",
                [
                    _,
                    _,
                    AstValue::Replace(ReplaceState::Lvar(repl @ Replacement::Repeat { .. })),
                ],
            )
            | ("Expr", [_, _, AstValue::Replace(ReplaceState::Lvar(repl))]) => {
                return Ok(AstValue::Replace(ReplaceState::Expr(repl.clone())));
            }
            ("Lid", [_, AstValue::Replace(ReplaceState::Tuple(repl))]) => {
                return Ok(AstValue::Replace(ReplaceState::Lid(repl.clone())));
            }
            ("Stmt", [_, AstValue::Replace(ReplaceState::ExprStmt(repl))]) => {
                return Ok(AstValue::Replace(ReplaceState::Stmt(repl.clone())));
            }
            _ => {}
        }

        let mut fields = Vec::with_capacity(self.fields.len());

        for field in self.fields {
            fields.push(field.into_tokens()?);
        }

        let span = Span::call_site();

        match self.name {
            "Hint" if fields.len() == 2 => {
                let unboxed = fields.pop().unwrap();
                fields.push(box_stream(unboxed, span));
            }
            _ => {}
        }

        let mut inner = TokenStream::new();
        inner.push_comma_sequence(fields);

        let mut s = TokenStream::new();
        s.push_ident(span, self.name);
        s.push_paren_group(inner);
        Ok(AstValue::Tokens(s))
    }
}

struct SerializeTupleVariant {
    state: Rc<AstState>,
    name: &'static str,
    variant: &'static str,
    fields: Vec<AstValue>,
}

impl ser::SerializeTupleVariant for SerializeTupleVariant {
    type Ok = AstValue;
    type Error = AstError;

    fn serialize_field<T: Serialize + ?Sized>(&mut self, f: &T) -> Result<()> {
        let inner = f.serialize(AstWriter::new(self.state.clone()))?;
        self.fields.push(inner);
        Ok(())
    }

    fn end(self) -> Result {
        let mut fields = Vec::with_capacity(self.fields.len());
        for field in self.fields {
            fields.push(field.into_tokens()?);
        }

        let span = Span::call_site();

        let mut inner = TokenStream::new();
        inner.push_comma_sequence(fields);

        let mut s = TokenStream::new();
        s.push_ident(span, self.name);
        s.push_colon2();
        s.push_ident(span, self.variant);
        s.push_paren_group(inner);
        Ok(AstValue::Tokens(s))
    }
}

struct SerializeStructVariant {}

impl ser::SerializeStructVariant for SerializeStructVariant {
    type Ok = AstValue;
    type Error = AstError;

    fn serialize_field<T: Serialize + ?Sized>(&mut self, _: &'static str, _: &T) -> Result<()> {
        todo!()
    }

    fn end(self) -> Result {
        todo!()
    }
}
