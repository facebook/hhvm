use crate::lexable_token::LexableToken;
use crate::parser_env::ParserEnv;
use crate::source_text::SourceText;
use crate::syntax::*;
use crate::syntax_kind::SyntaxKind;
use crate::syntax_smart_constructors::{StateType, SyntaxSmartConstructors};
use crate::token_kind::TokenKind;

use std::marker::PhantomData;

const PPL_MACRO_STR: &str = "__PPL";

#[derive(Clone)]
pub struct State<'src, S> {
    pub seen_ppl: bool,
    source: SourceText<'src>,
    is_codegen: bool,
    phantom_s: PhantomData<S>,
}

impl<'src, S> State<'src, S> {
    fn set_seen_ppl(&mut self, v: bool) {
        self.seen_ppl = v;
    }

    fn is_codegen(&self) -> bool {
        self.is_codegen
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

    fn next(t: Self, _inputs: Vec<&S>) -> Self {
        t
    }
}

pub use crate::coroutine_smart_constructors_generated::*;

pub struct CoroutineSmartConstructors<S> {
    phantom_s: PhantomData<S>,
}
impl<S: SyntaxType> CoroutineSmartConstructors<S> {
    fn is_coroutine(r: &S) -> bool {
        if let SyntaxKind::Token(TokenKind::Coroutine) = r.kind() {
            true
        } else {
            false
        }
    }
}

impl<'a, S: SyntaxType> SyntaxSmartConstructors<'a, S, State<'a, S>>
    for CoroutineSmartConstructors<S>
{
    fn make_token(st: State<'a, S>, token: Self::Token) -> (State<'a, S>, Self::R) {
        let token = if st.is_codegen() {
            token.with_leading(vec![]).with_trailing(vec![])
        } else {
            token
        };
        (st, Self::R::make_token(token))
    }

    fn make_function_declaration_header(
        mut st: State<'a, S>,
        modifiers: Self::R,
        r2: Self::R,
        r3: Self::R,
        r4: Self::R,
        r5: Self::R,
        r6: Self::R,
        r7: Self::R,
        r8: Self::R,
        r9: Self::R,
        r10: Self::R,
    ) -> (State<'a, S>, Self::R) {
        st.set_seen_ppl(st.seen_ppl || modifiers.any(|r| Self::is_coroutine(r)));
        (
            st,
            Self::R::make_function_declaration_header(
                modifiers, r2, r3, r4, r5, r6, r7, r8, r9, r10,
            ),
        )
    }

    fn make_closure_type_specifier(
        mut st: State<'a, S>,
        r1: Self::R,
        coroutine: Self::R,
        r3: Self::R,
        r4: Self::R,
        r5: Self::R,
        r6: Self::R,
        r7: Self::R,
        r8: Self::R,
        r9: Self::R,
    ) -> (State<'a, S>, Self::R) {
        st.set_seen_ppl(st.seen_ppl || Self::is_coroutine(&coroutine));
        (
            st,
            Self::R::make_closure_type_specifier(r1, coroutine, r3, r4, r5, r6, r7, r8, r9),
        )
    }

    fn make_anonymous_function(
        mut st: State<'a, S>,
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
    ) -> (State<'a, S>, Self::R) {
        st.set_seen_ppl(st.seen_ppl || Self::is_coroutine(&coroutine));
        (
            st,
            Self::R::make_anonymous_function(
                r1, r2, r3, coroutine, r5, r6, r7, r8, r9, r10, r11, r12,
            ),
        )
    }

    fn make_lambda_expression(
        mut st: State<'a, S>,
        r1: Self::R,
        r2: Self::R,
        coroutine: Self::R,
        r4: Self::R,
        r5: Self::R,
        r6: Self::R,
    ) -> (State<'a, S>, Self::R) {
        st.set_seen_ppl(st.seen_ppl || Self::is_coroutine(&coroutine));
        (
            st,
            Self::R::make_lambda_expression(r1, r2, coroutine, r4, r5, r6),
        )
    }

    fn make_awaitable_creation_expression(
        mut st: State<'a, S>,
        r1: Self::R,
        r2: Self::R,
        coroutine: Self::R,
        r4: Self::R,
    ) -> (State<'a, S>, Self::R) {
        st.set_seen_ppl(st.seen_ppl || Self::is_coroutine(&coroutine));
        (
            st,
            Self::R::make_awaitable_creation_expression(r1, r2, coroutine, r4),
        )
    }

    fn make_attribute_specification(
        mut st: State<'a, S>,
        left: Self::R,
        attribute_name: Self::R,
        right: Self::R,
    ) -> (State<'a, S>, Self::R) {
        let is_ppl_attr_folder = |has_seen_ppl: bool, r: &Self::R| {
            if has_seen_ppl {
                return true;
            }
            match &r.as_syntax().syntax {
                SyntaxVariant::ConstructorCall(box crate::syntax::ConstructorCallChildren {
                    constructor_call_type,
                    constructor_call_left_paren,
                    constructor_call_argument_list,
                    constructor_call_right_paren,
                }) => {
                    use SyntaxVariant::Missing;
                    if let (Missing, Missing, Missing) = (
                        &constructor_call_left_paren.syntax,
                        &constructor_call_argument_list.syntax,
                        &constructor_call_right_paren.syntax,
                    ) {
                        let text: Option<&[u8]> = constructor_call_type
                            .value
                            .text_range()
                            .map(|range| &st.source.text()[range.0..range.1]);
                        text.map_or(false, |t| String::from_utf8_lossy(t) == PPL_MACRO_STR)
                    } else {
                        false
                    }
                }
                _ => true,
            }
        };
        st.set_seen_ppl(st.seen_ppl || attribute_name.fold_list(false, is_ppl_attr_folder));
        (
            st,
            Self::R::make_attribute_specification(left, attribute_name, right),
        )
    }
}
