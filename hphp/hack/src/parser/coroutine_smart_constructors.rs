// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod coroutine_smart_constructors_generated;

use ocaml::core::mlvalues::Value;
use parser_core_types::{
    lexable_token::LexableToken, parser_env::ParserEnv, source_text::SourceText, syntax::*,
    token_kind::TokenKind,
};
use rust_to_ocaml::{SerializationContext, ToOcaml};
use std::marker::PhantomData;
use syntax_smart_constructors::{StateType, SyntaxSmartConstructors};

const PPL_MACRO_STR: &str = "__PPL";

pub trait CoroutineStateType {
    fn update_seen_ppl(&mut self, v: bool) {
        if !self.seen_ppl() {
            self.set_seen_ppl(v)
        }
    }

    fn set_seen_ppl(&mut self, v: bool);
    fn seen_ppl(&self) -> bool;
    fn source(&self) -> &SourceText;
    fn is_codegen(&self) -> bool;
}

pub struct State<'src, S> {
    seen_ppl: bool,
    source: SourceText<'src>,
    is_codegen: bool,
    phantom_s: PhantomData<*const S>,
}

impl<'src, S> State<'src, S> {
    pub fn new(source: &SourceText<'src>, is_codegen: bool) -> Self {
        State {
            seen_ppl: false,
            source: source.clone(),
            is_codegen,
            phantom_s: PhantomData,
        }
    }

    pub fn seen_ppl(&self) -> bool {
        self.seen_ppl
    }
}

impl<'src, S> Clone for State<'src, S> {
    fn clone(&self) -> Self {
        Self {
            seen_ppl: self.seen_ppl,
            source: self.source.clone(),
            is_codegen: self.is_codegen,
            phantom_s: self.phantom_s,
        }
    }
}

impl<'src, S> CoroutineStateType for State<'src, S> {
    fn set_seen_ppl(&mut self, v: bool) {
        self.seen_ppl = v;
    }

    fn is_codegen(&self) -> bool {
        self.is_codegen
    }

    fn seen_ppl(&self) -> bool {
        self.seen_ppl
    }

    fn source(&self) -> &SourceText {
        &self.source
    }
}

impl<'src, S> StateType<'src, S> for State<'src, S> {
    fn initial(env0: &ParserEnv, src: &SourceText<'src>) -> Self {
        State::new(src, env0.codegen)
    }

    fn next(&mut self, _inputs: &[&S]) {}
}

pub use crate::coroutine_smart_constructors_generated::*;

pub struct CoroutineSmartConstructors<'a, S, T: StateType<'a, S> + CoroutineStateType> {
    pub state: T,
    phantom_s: PhantomData<&'a S>,
}
impl<'a, S, T> Clone for CoroutineSmartConstructors<'a, S, T>
where
    T: StateType<'a, S> + CoroutineStateType,
{
    fn clone(&self) -> Self {
        Self {
            state: self.state.clone(),
            phantom_s: PhantomData,
        }
    }
}

fn is_coroutine<'a, S, T>(r: &S) -> bool
where
    S: SyntaxType<'a, T>,
    S::Value: SyntaxValueWithKind,
{
    let value = r.value();
    match value.token_kind() {
        Some(TokenKind::Coroutine) => true,
        _ => false,
    }
}

impl<'a, S, T> SyntaxSmartConstructors<'a, S, T> for CoroutineSmartConstructors<'a, S, T>
where
    T: StateType<'a, S> + CoroutineStateType,
    S: SyntaxType<'a, T>,
    S::Value: SyntaxValueWithKind,
{
    fn new(env: &ParserEnv, src: &SourceText<'a>) -> Self {
        Self {
            state: T::initial(env, src),
            phantom_s: PhantomData,
        }
    }

    fn make_token(&mut self, token: Self::Token) -> Self::R {
        let token = if self.state.is_codegen() {
            token.with_leading(vec![]).with_trailing(vec![])
        } else {
            token
        };
        Self::R::make_token(&self.state, token)
    }

    fn make_list(&mut self, items: Vec<Self::R>, offset: usize) -> Self::R {
        self.state
            .update_seen_ppl(items.iter().any(|i| is_coroutine(i)));
        Self::R::make_list(&self.state, items, offset)
    }

    fn make_closure_type_specifier(
        &mut self,
        r1: Self::R,
        coroutine: Self::R,
        r3: Self::R,
        r4: Self::R,
        r5: Self::R,
        r6: Self::R,
        r7: Self::R,
        r8: Self::R,
        r9: Self::R,
    ) -> Self::R {
        self.state.update_seen_ppl(is_coroutine(&coroutine));
        Self::R::make_closure_type_specifier(&self.state, r1, coroutine, r3, r4, r5, r6, r7, r8, r9)
    }

    fn make_anonymous_function(
        &mut self,
        r1: Self::R,
        r2: Self::R,
        r3: Self::R,
        coroutine: Self::R,
        r5: Self::R,
        r6: Self::R,
        r7: Self::R,
        r8: Self::R,
        r9: Self::R,
        r10: Self::R,
        r11: Self::R,
        r12: Self::R,
    ) -> Self::R {
        self.state.update_seen_ppl(is_coroutine(&coroutine));
        Self::R::make_anonymous_function(
            &self.state,
            r1,
            r2,
            r3,
            coroutine,
            r5,
            r6,
            r7,
            r8,
            r9,
            r10,
            r11,
            r12,
        )
    }

    fn make_lambda_expression(
        &mut self,
        r1: Self::R,
        r2: Self::R,
        coroutine: Self::R,
        r4: Self::R,
        r5: Self::R,
        r6: Self::R,
    ) -> Self::R {
        self.state.update_seen_ppl(is_coroutine(&coroutine));
        Self::R::make_lambda_expression(&self.state, r1, r2, coroutine, r4, r5, r6)
    }

    fn make_awaitable_creation_expression(
        &mut self,
        r1: Self::R,
        r2: Self::R,
        coroutine: Self::R,
        r4: Self::R,
    ) -> Self::R {
        self.state.update_seen_ppl(is_coroutine(&coroutine));
        Self::R::make_awaitable_creation_expression(&self.state, r1, r2, coroutine, r4)
    }

    fn make_constructor_call(
        &mut self,
        call_type: Self::R,
        left_paren: Self::R,
        argument_list: Self::R,
        right_paren: Self::R,
    ) -> Self::R {
        let seen_ppl = if self.state.seen_ppl() {
            true
        } else if !left_paren.value().is_missing()
            || !argument_list.value().is_missing()
            || !right_paren.value().is_missing()
        {
            false
        } else {
            let text: Option<&[u8]> = call_type
                .value()
                .text_range()
                .map(|range| &self.state.source().text()[range.0..range.1]);
            text.map_or(false, |t| String::from_utf8_lossy(t) == PPL_MACRO_STR)
        };
        self.state.set_seen_ppl(seen_ppl);
        Self::R::make_constructor_call(
            &self.state,
            call_type,
            left_paren,
            argument_list,
            right_paren,
        )
    }
}

impl<'a, S> ToOcaml for State<'a, S> {
    unsafe fn to_ocaml(&self, _context: &SerializationContext) -> Value {
        self.seen_ppl().to_ocaml(_context)
    }
}
