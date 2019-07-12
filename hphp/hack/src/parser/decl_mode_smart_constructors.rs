/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*/
use crate::lexable_token::LexableToken;
use crate::parser_env::ParserEnv;
use crate::smart_constructors::StateType;
use crate::source_text::SourceText;
use crate::syntax::*;
use crate::syntax_smart_constructors::SyntaxSmartConstructors;
use crate::token_kind::TokenKind;

pub struct State<S> {
    phantom_s: std::marker::PhantomData<S>,
}
impl<'src, S> StateType<'src, S> for State<S> {
    type T = Vec<bool>;
    fn initial(_: &ParserEnv, _: &SourceText<'src>) -> Self::T {
        vec![]
    }
    fn next(mut st: Self::T, inputs: Vec<&S>) -> Self::T {
        let st_todo = if st.len() > inputs.len() {
            st.split_off(st.len() - inputs.len())
        } else {
            std::mem::replace(&mut st, vec![])
        };
        let res = st_todo.into_iter().fold(false, |b1, b2| b1 || b2);
        st.push(res);
        st
    }
}

pub use crate::decl_mode_smart_constructors_generated::*;

pub struct DeclModeSmartConstructors<Token, Value> {
    phantom_token: std::marker::PhantomData<Token>,
    phantom_value: std::marker::PhantomData<Value>,
}

impl<'a, Token, Value>
    SyntaxSmartConstructors<'a, Syntax<Token, Value>, State<Syntax<Token, Value>>>
    for DeclModeSmartConstructors<Token, Value>
where
    Token: LexableToken,
    Value: SyntaxValueType<Token>,
{
    fn make_yield_expression(st: Vec<bool>, _r1: Self::R, _r2: Self::R) -> (Vec<bool>, Self::R) {
        let (_, st) = pop_n(st, 2);
        (push(st, true), Self::R::make_missing(0))
    }

    fn make_yield_from_expression(
        st: Vec<bool>,
        _r1: Self::R,
        _r2: Self::R,
        _r3: Self::R,
    ) -> (Vec<bool>, Self::R) {
        let (_, st) = pop_n(st, 3);
        (push(st, true), Self::R::make_missing(0))
    }

    fn make_lambda_expression(
        st: Vec<bool>,
        r1: Self::R,
        r2: Self::R,
        r3: Self::R,
        r4: Self::R,
        r5: Self::R,
        body: Self::R,
    ) -> (Vec<bool>, Self::R) {
        let (saw_yield, st) = pop_n(st, 6);
        let body = replace_body(body, saw_yield);
        (
            push(st, false),
            Self::R::make_lambda_expression(r1, r2, r3, r4, r5, body),
        )
    }

    fn make_anonymous_function(
        st: Vec<bool>,
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
    ) -> (Vec<bool>, Self::R) {
        let (saw_yield, st) = pop_n(st, 12);
        let body = replace_body(body, saw_yield);
        (
            push(st, false),
            Self::R::make_anonymous_function(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, body),
        )
    }

    fn make_awaitable_creation_expression(
        st: Vec<bool>,
        r1: Self::R,
        r2: Self::R,
        r3: Self::R,
        body: Self::R,
    ) -> (Vec<bool>, Self::R) {
        let (saw_yield, st) = pop_n(st, 4);
        let body = replace_body(body, saw_yield);
        (
            push(st, false),
            Self::R::make_awaitable_creation_expression(r1, r2, r3, body),
        )
    }

    fn make_methodish_declaration(
        st: Vec<bool>,
        r1: Self::R,
        r2: Self::R,
        body: Self::R,
        r3: Self::R,
    ) -> (Vec<bool>, Self::R) {
        let (_, st) = pop_n(st, 1);
        let (saw_yield, st) = pop_n(st, 3);
        let body = replace_body(body, saw_yield);
        (
            push(st, false),
            Self::R::make_methodish_declaration(r1, r2, body, r3),
        )
    }

    fn make_function_declaration(
        st: Vec<bool>,
        r1: Self::R,
        r2: Self::R,
        body: Self::R,
    ) -> (Vec<bool>, Self::R) {
        let (saw_yield, st) = pop_n(st, 3);
        let body = replace_body(body, saw_yield);
        (
            push(st, false),
            Self::R::make_function_declaration(r1, r2, body),
        )
    }
}

/// Pops n times and returns the first popped element as well as the resulting vector
fn pop_n(mut st: Vec<bool>, n: usize) -> (bool, Vec<bool>) {
    if st.len() < n {
        panic!("invalid state");
    }
    let head = st.pop().unwrap();
    st.truncate(st.len() - (n - 1)); // pop n-1 times efficiently
    (head, st)
}

fn push(mut st: Vec<bool>, s: bool) -> Vec<bool> {
    st.push(s);
    st
}

fn replace_body<Token, Value>(body: Syntax<Token, Value>, saw_yield: bool) -> Syntax<Token, Value>
where
    Token: LexableToken,
    Value: SyntaxValueType<Token>,
{
    match body.syntax {
        SyntaxVariant::CompoundStatement(children @ box CompoundStatementChildren { .. }) => {
            let stmts = if saw_yield {
                let token = Token::make(TokenKind::Yield, 0, 0, vec![], vec![]);
                let yield_ = Syntax::<Token, Value>::make_token(token);
                Syntax::make_list(vec![yield_], 0)
            } else {
                Syntax::make_missing(0)
            };
            Syntax::make_compound_statement(
                children.compound_left_brace,
                stmts,
                children.compound_right_brace,
            )
        }
        _ => body,
    }
}
