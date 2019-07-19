use crate::lexable_token::LexableToken;
use crate::parser_env::ParserEnv;
use crate::source_text::SourceText;
use crate::syntax::*;
use crate::syntax_smart_constructors::{StateType, SyntaxSmartConstructors};
use crate::token_kind::TokenKind;

use std::marker::PhantomData;

const PPL_MACRO_STR: &str = "__PPL";

pub trait CoroutineStateType {
    fn set_seen_ppl(&mut self, v: bool);
    fn seen_ppl(&self) -> bool;
    fn source(&self) -> &SourceText;
    fn is_codegen(&self) -> bool;
}

#[derive(Clone)]
pub struct State<'src, S> {
    seen_ppl: bool,
    source: SourceText<'src>,
    is_codegen: bool,
    phantom_s: PhantomData<S>,
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
        State {
            seen_ppl: false,
            source: *src,
            is_codegen: env0.codegen,
            phantom_s: PhantomData,
        }
    }

    fn next(t: Self, _inputs: &[&S]) -> Self {
        t
    }
}

pub use crate::coroutine_smart_constructors_generated::*;

pub struct CoroutineSmartConstructors<S> {
    phantom_s: PhantomData<S>,
}

fn is_coroutine<S, T>(r: &S) -> bool
where
    S: SyntaxType<T>,
    S::Value: SyntaxValueWithKind,
{
    let value = r.value();
    match value.token_kind() {
        Some(TokenKind::Coroutine) => true,
        _ => false,
    }
}

impl<'a, S, T> SyntaxSmartConstructors<'a, S, T> for CoroutineSmartConstructors<S>
where
    T: StateType<'a, S> + CoroutineStateType,
    S: SyntaxType<T>,
    S::Value: SyntaxValueWithKind,
{
    fn make_token(st: T, token: Self::Token) -> (T, Self::R) {
        let token = if st.is_codegen() {
            token.with_leading(vec![]).with_trailing(vec![])
        } else {
            token
        };
        let r = Self::R::make_token(&st, token);
        (st, r)
    }

    fn make_list(mut st: T, items: Vec<Self::R>, offset: usize) -> (T, Self::R) {
        st.set_seen_ppl(st.seen_ppl() || items.iter().any(|i| is_coroutine(i)));
        let r = Self::R::make_list(&st, items, offset);
        (st, r)
    }

    fn make_closure_type_specifier(
        mut st: T,
        r1: Self::R,
        coroutine: Self::R,
        r3: Self::R,
        r4: Self::R,
        r5: Self::R,
        r6: Self::R,
        r7: Self::R,
        r8: Self::R,
        r9: Self::R,
    ) -> (T, Self::R) {
        st.set_seen_ppl(st.seen_ppl() || is_coroutine(&coroutine));
        let r =
            Self::R::make_closure_type_specifier(&st, r1, coroutine, r3, r4, r5, r6, r7, r8, r9);
        (st, r)
    }

    fn make_anonymous_function(
        mut st: T,
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
    ) -> (T, Self::R) {
        st.set_seen_ppl(st.seen_ppl() || is_coroutine(&coroutine));
        let r = Self::R::make_anonymous_function(
            &st, r1, r2, r3, coroutine, r5, r6, r7, r8, r9, r10, r11, r12,
        );
        (st, r)
    }

    fn make_lambda_expression(
        mut st: T,
        r1: Self::R,
        r2: Self::R,
        coroutine: Self::R,
        r4: Self::R,
        r5: Self::R,
        r6: Self::R,
    ) -> (T, Self::R) {
        st.set_seen_ppl(st.seen_ppl() || is_coroutine(&coroutine));
        let r = Self::R::make_lambda_expression(&st, r1, r2, coroutine, r4, r5, r6);
        (st, r)
    }

    fn make_awaitable_creation_expression(
        mut st: T,
        r1: Self::R,
        r2: Self::R,
        coroutine: Self::R,
        r4: Self::R,
    ) -> (T, Self::R) {
        st.set_seen_ppl(st.seen_ppl() || is_coroutine(&coroutine));
        let r = Self::R::make_awaitable_creation_expression(&st, r1, r2, coroutine, r4);
        (st, r)
    }

    fn make_constructor_call(
        mut st: T,
        call_type: Self::R,
        left_paren: Self::R,
        argument_list: Self::R,
        right_paren: Self::R,
    ) -> (T, Self::R) {
        let seen_ppl = if st.seen_ppl() {
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
                .map(|range| &st.source().text()[range.0..range.1]);
            text.map_or(false, |t| String::from_utf8_lossy(t) == PPL_MACRO_STR)
        };
        st.set_seen_ppl(seen_ppl);
        let r =
            Self::R::make_constructor_call(&st, call_type, left_paren, argument_list, right_paren);
        (st, r)
    }
}
