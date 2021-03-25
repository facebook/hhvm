// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod decl_mode_smart_constructors_generated;

use bumpalo::Bump;
use ocamlrep::{Allocator, OpaqueValue, ToOcamlRep};
use parser_core_types::{
    lexable_token::LexableToken,
    source_text::SourceText,
    syntax::{SyntaxTypeBase, SyntaxValueType},
    syntax_by_ref::{has_arena::HasArena, syntax::Syntax, syntax_variant_generated::SyntaxVariant},
    syntax_type::SyntaxType,
    token_factory::TokenFactory,
    token_kind::TokenKind,
    trivia_factory::TriviaFactory,
};
use syntax_smart_constructors::{StateType, SyntaxSmartConstructors};

pub struct State<'src, 'arena, S> {
    arena: &'arena Bump,
    source: SourceText<'src>,
    stack: Vec<bool>,
    phantom_s: std::marker::PhantomData<*const S>,
}

impl<'src, 'arena, S> State<'src, 'arena, S> {
    fn new(source: &SourceText<'src>, arena: &'arena Bump) -> Self {
        Self {
            arena,
            source: source.clone(),
            stack: vec![],
            phantom_s: std::marker::PhantomData,
        }
    }
}

impl<S> Clone for State<'_, '_, S> {
    fn clone(&self) -> Self {
        Self {
            arena: self.arena,
            source: self.source.clone(),
            stack: self.stack.clone(),
            phantom_s: self.phantom_s,
        }
    }
}

impl<S> State<'_, '_, S> {
    /// Pops n times and returns the first popped element
    fn pop_n(&mut self, n: usize) -> bool {
        if self.stack.len() < n {
            panic!("invalid state");
        }
        let head = self.stack.pop().unwrap();
        self.stack.truncate(self.stack.len() - (n - 1)); // pop n-1 times efficiently
        head
    }

    fn push(&mut self, s: bool) {
        self.stack.push(s);
    }

    pub fn stack(&self) -> &Vec<bool> {
        &self.stack
    }
}

impl<S> StateType<S> for State<'_, '_, S> {
    fn next(&mut self, inputs: &[&S]) {
        let st_todo = if self.stack.len() > inputs.len() {
            self.stack.split_off(self.stack.len() - inputs.len())
        } else {
            std::mem::replace(&mut self.stack, vec![])
        };
        let res = st_todo.into_iter().any(|b2| b2);
        self.push(res);
    }
}

impl<'arena, S> HasArena<'arena> for State<'_, 'arena, S> {
    fn get_arena(&self) -> &'arena Bump {
        self.arena
    }
}

pub use crate::decl_mode_smart_constructors_generated::*;

pub struct DeclModeSmartConstructors<'src, 'arena, S, T, V, TF> {
    pub state: State<'src, 'arena, S>,
    pub token_factory: TF,
    phantom_value: std::marker::PhantomData<(V, T)>,
}

impl<'src, 'arena, T, V, TF>
    DeclModeSmartConstructors<'src, 'arena, Syntax<'arena, T, V>, T, V, TF>
{
    pub fn new(src: &SourceText<'src>, token_factory: TF, arena: &'arena Bump) -> Self {
        Self {
            state: State::new(src, arena),
            token_factory,
            phantom_value: std::marker::PhantomData,
        }
    }
}

impl<'src, S, T, V, TF: Clone> Clone for DeclModeSmartConstructors<'src, '_, S, T, V, TF> {
    fn clone(&self) -> Self {
        Self {
            state: self.state.clone(),
            token_factory: self.token_factory.clone(),
            phantom_value: self.phantom_value,
        }
    }
}

type SyntaxToken<'src, 'arena, T, V> =
    <Syntax<'arena, T, V> as SyntaxTypeBase<State<'src, 'arena, Syntax<'arena, T, V>>>>::Token;

impl<'src, 'arena, T, V, TF>
    SyntaxSmartConstructors<Syntax<'arena, T, V>, TF, State<'src, 'arena, Syntax<'arena, T, V>>>
    for DeclModeSmartConstructors<'src, 'arena, Syntax<'arena, T, V>, T, V, TF>
where
    TF: TokenFactory<Token = SyntaxToken<'src, 'arena, T, V>>,

    T: LexableToken + Copy,
    V: SyntaxValueType<T> + Clone,
    Syntax<'arena, T, V>: SyntaxType<State<'src, 'arena, Syntax<'arena, T, V>>>,
{
    fn make_yield_expression(&mut self, _r1: Self::R, _r2: Self::R) -> Self::R {
        self.state.pop_n(2);
        self.state.push(true);
        Self::R::make_missing(0)
    }

    fn make_lambda_expression(
        &mut self,
        r1: Self::R,
        r2: Self::R,
        r3: Self::R,
        r4: Self::R,
        body: Self::R,
    ) -> Self::R {
        let saw_yield = self.state.pop_n(5);
        let body = replace_body(&mut self.token_factory, &mut self.state, body, saw_yield);
        self.state.push(false);
        Self::R::make_lambda_expression(&self.state, r1, r2, r3, r4, body)
    }

    fn make_anonymous_function(
        &mut self,
        r1: Self::R,
        r2: Self::R,
        r3: Self::R,
        r4: Self::R,
        r5: Self::R,
        r6: Self::R,
        r7: Self::R,
        r8: Self::R,
        r9: Self::R,
        r10: Self::R,
        r11: Self::R,
        body: Self::R,
    ) -> Self::R {
        let saw_yield = self.state.pop_n(11);
        let body = replace_body(&mut self.token_factory, &mut self.state, body, saw_yield);
        self.state.push(false);
        Self::R::make_anonymous_function(
            &self.state,
            r1,
            r2,
            r3,
            r4,
            r5,
            r6,
            r7,
            r8,
            r9,
            r10,
            r11,
            body,
        )
    }

    fn make_awaitable_creation_expression(
        &mut self,
        r1: Self::R,
        r2: Self::R,
        body: Self::R,
    ) -> Self::R {
        let saw_yield = self.state.pop_n(3);
        let body = replace_body(&mut self.token_factory, &mut self.state, body, saw_yield);
        self.state.push(false);
        Self::R::make_awaitable_creation_expression(&self.state, r1, r2, body)
    }

    fn make_methodish_declaration(
        &mut self,
        r1: Self::R,
        r2: Self::R,
        body: Self::R,
        r3: Self::R,
    ) -> Self::R {
        self.state.pop_n(1);
        let saw_yield = self.state.pop_n(3);
        let body = replace_body(&mut self.token_factory, &mut self.state, body, saw_yield);
        self.state.push(false);
        Self::R::make_methodish_declaration(&self.state, r1, r2, body, r3)
    }

    fn make_function_declaration(&mut self, r1: Self::R, r2: Self::R, body: Self::R) -> Self::R {
        let saw_yield = self.state.pop_n(3);
        let body = replace_body(&mut self.token_factory, &mut self.state, body, saw_yield);
        self.state.push(false);
        Self::R::make_function_declaration(&self.state, r1, r2, body)
    }
}

fn replace_body<'src, 'arena, T, V, TF>(
    token_factory: &mut TF,
    st: &mut State<'src, 'arena, Syntax<'arena, T, V>>,
    body: Syntax<'arena, T, V>,
    saw_yield: bool,
) -> Syntax<'arena, T, V>
where
    T: LexableToken + Copy,
    V: SyntaxValueType<T> + Clone,
    TF: TokenFactory<Token = SyntaxToken<'src, 'arena, T, V>>,
{
    match body.children {
        SyntaxVariant::CompoundStatement(children) => {
            let stmts = if saw_yield {
                let leading = token_factory.trivia_factory_mut().make();
                let trailing = token_factory.trivia_factory_mut().make();
                let token = token_factory.make(TokenKind::Yield, 0, 0, leading, trailing);
                let yield_ = Syntax::<T, V>::make_token(token);
                Syntax::make_list(st, vec![yield_], 0)
            } else {
                Syntax::make_missing(0)
            };
            let left_brace = children.left_brace.clone();
            let right_brace = children.right_brace.clone();
            Syntax::make_compound_statement(st, left_brace, stmts, right_brace)
        }
        _ => body,
    }
}

impl<S> ToOcamlRep for State<'_, '_, S> {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        self.stack().to_ocamlrep(alloc)
    }
}
