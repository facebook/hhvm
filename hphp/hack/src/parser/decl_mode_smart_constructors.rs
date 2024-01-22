// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod decl_mode_smart_constructors_generated;

use bumpalo::Bump;
use ocamlrep::Allocator;
use ocamlrep::ToOcamlRep;
use parser_core_types::lexable_token::LexableToken;
use parser_core_types::source_text::SourceText;
use parser_core_types::syntax::SyntaxTypeBase;
use parser_core_types::syntax::SyntaxValueType;
use parser_core_types::syntax_by_ref::has_arena::HasArena;
use parser_core_types::syntax_by_ref::syntax::Syntax;
use parser_core_types::syntax_by_ref::syntax_variant_generated::SyntaxVariant;
use parser_core_types::syntax_type::SyntaxType;
use parser_core_types::token_factory::TokenFactory;
use parser_core_types::token_kind::TokenKind;
use parser_core_types::trivia_factory::TriviaFactory;
use syntax_smart_constructors::StateType;
use syntax_smart_constructors::SyntaxSmartConstructors;

pub struct State<'s, 'a, S> {
    arena: &'a Bump,
    source: SourceText<'s>,
    stack: Vec<bool>,
    phantom_s: std::marker::PhantomData<*const S>,
}

impl<'s, 'a, S> State<'s, 'a, S> {
    fn new(source: &SourceText<'s>, arena: &'a Bump) -> Self {
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
            std::mem::take(&mut self.stack)
        };
        let res = st_todo.into_iter().any(|b2| b2);
        self.push(res);
    }
}

impl<'a, S> HasArena<'a> for State<'_, 'a, S> {
    fn get_arena(&self) -> &'a Bump {
        self.arena
    }
}

pub struct DeclModeSmartConstructors<'s, 'a, S, T, V, TF> {
    pub state: State<'s, 'a, S>,
    pub token_factory: TF,
    phantom_value: std::marker::PhantomData<(V, T)>,
}

impl<'s, 'a, T, V, TF> DeclModeSmartConstructors<'s, 'a, Syntax<'a, T, V>, T, V, TF> {
    pub fn new(src: &SourceText<'s>, token_factory: TF, arena: &'a Bump) -> Self {
        Self {
            state: State::new(src, arena),
            token_factory,
            phantom_value: std::marker::PhantomData,
        }
    }
}

impl<'s, S, T, V, TF: Clone> Clone for DeclModeSmartConstructors<'s, '_, S, T, V, TF> {
    fn clone(&self) -> Self {
        Self {
            state: self.state.clone(),
            token_factory: self.token_factory.clone(),
            phantom_value: self.phantom_value,
        }
    }
}

type SyntaxToken<'s, 'a, T, V> =
    <Syntax<'a, T, V> as SyntaxTypeBase<State<'s, 'a, Syntax<'a, T, V>>>>::Token;

impl<'s, 'a, T, V, TF>
    SyntaxSmartConstructors<Syntax<'a, T, V>, TF, State<'s, 'a, Syntax<'a, T, V>>>
    for DeclModeSmartConstructors<'s, 'a, Syntax<'a, T, V>, T, V, TF>
where
    TF: TokenFactory<Token = SyntaxToken<'s, 'a, T, V>>,
    T: LexableToken + Copy,
    V: SyntaxValueType<T> + Clone,
    Syntax<'a, T, V>: SyntaxType<State<'s, 'a, Syntax<'a, T, V>>>,
{
    fn make_yield_expression(&mut self, _r1: Self::Output, _r2: Self::Output) -> Self::Output {
        self.state.pop_n(2);
        self.state.push(true);
        Self::Output::make_missing(0)
    }

    fn make_lambda_expression(
        &mut self,
        r1: Self::Output,
        r2: Self::Output,
        r3: Self::Output,
        r4: Self::Output,
        body: Self::Output,
    ) -> Self::Output {
        let saw_yield = self.state.pop_n(5);
        let body = replace_body(&mut self.token_factory, &mut self.state, body, saw_yield);
        self.state.push(false);
        Self::Output::make_lambda_expression(&self.state, r1, r2, r3, r4, body)
    }

    fn make_anonymous_function(
        &mut self,
        r1: Self::Output,
        r2: Self::Output,
        r3: Self::Output,
        r4: Self::Output,
        r5: Self::Output,
        r6: Self::Output,
        r7: Self::Output,
        r8: Self::Output,
        r9: Self::Output,
        r10: Self::Output,
        r11: Self::Output,
        body: Self::Output,
    ) -> Self::Output {
        let saw_yield = self.state.pop_n(11);
        let body = replace_body(&mut self.token_factory, &mut self.state, body, saw_yield);
        self.state.push(false);
        Self::Output::make_anonymous_function(
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
        r1: Self::Output,
        r2: Self::Output,
        body: Self::Output,
    ) -> Self::Output {
        let saw_yield = self.state.pop_n(3);
        let body = replace_body(&mut self.token_factory, &mut self.state, body, saw_yield);
        self.state.push(false);
        Self::Output::make_awaitable_creation_expression(&self.state, r1, r2, body)
    }

    fn make_methodish_declaration(
        &mut self,
        r1: Self::Output,
        r2: Self::Output,
        body: Self::Output,
        r3: Self::Output,
    ) -> Self::Output {
        self.state.pop_n(1);
        let saw_yield = self.state.pop_n(3);
        let body = replace_body(&mut self.token_factory, &mut self.state, body, saw_yield);
        self.state.push(false);
        Self::Output::make_methodish_declaration(&self.state, r1, r2, body, r3)
    }

    fn make_function_declaration(
        &mut self,
        r1: Self::Output,
        r2: Self::Output,
        body: Self::Output,
    ) -> Self::Output {
        let saw_yield = self.state.pop_n(3);
        let body = replace_body(&mut self.token_factory, &mut self.state, body, saw_yield);
        self.state.push(false);
        Self::Output::make_function_declaration(&self.state, r1, r2, body)
    }
}

fn replace_body<'s, 'a, T, V, TF>(
    token_factory: &mut TF,
    st: &mut State<'s, 'a, Syntax<'a, T, V>>,
    body: Syntax<'a, T, V>,
    saw_yield: bool,
) -> Syntax<'a, T, V>
where
    T: LexableToken + Copy,
    V: SyntaxValueType<T> + Clone,
    TF: TokenFactory<Token = SyntaxToken<'s, 'a, T, V>>,
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
    fn to_ocamlrep<'a, A: Allocator>(&'a self, alloc: &'a A) -> ocamlrep::Value<'a> {
        self.stack().to_ocamlrep(alloc)
    }
}
