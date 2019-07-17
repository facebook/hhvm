use crate::lexable_token::LexableToken;
use crate::parser_env::ParserEnv;
use crate::smart_constructors::StateType;
use crate::source_text::SourceText;
use crate::syntax::*;
use crate::syntax_kind::SyntaxKind;
use crate::syntax_smart_constructors::SyntaxSmartConstructors;
use crate::token_kind::TokenKind;

use std::cell::RefCell;
use std::marker::PhantomData;

const PPL_MACRO_STR: &str = "__PPL";

// Note: In OCaml, module State is first-class and can refer to heap-allocated data.  However,
// and these references leak into the CoroutineSmartConstructors module.  Translating this
// naively to Rust would mean every make_* method in SC would need to be an instance method
// (i.e., have a &self param) which would break encapsulation of the SC interface.  Therefore,
// work around this by using thread-local env, which will work fine provided that the
// same thread doesn't concurrently work with 2 or more instances of CoroutineSmartConstructors,
// which is a reasonable assumption given the current usage of this module.
thread_local!(static ENV_LOCAL: RefCell<ParserEnv> = RefCell::new(ParserEnv::default()));

pub struct State<S> {
    phantom_s: PhantomData<S>,
}

pub type Bool<'src> = (bool, SourceText<'src>);

impl<'src, S> StateType<'src, S> for State<S> {
    type T = Bool<'src>;
    fn initial(env0: &ParserEnv, src: &SourceText<'src>) -> Bool<'src> {
        ENV_LOCAL.with(|env| env.replace(env0.clone()));
        (false, *src)
    }
    fn next(t: Bool<'src>, _inputs: Vec<&S>) -> Bool<'src> {
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

impl<'a, S: SyntaxType> SyntaxSmartConstructors<'a, S, State<S>> for CoroutineSmartConstructors<S> {
    fn make_token(st: Bool<'a>, token: Self::Token) -> (Bool<'a>, Self::R) {
        let codegen = ENV_LOCAL.with(|env| env.borrow().codegen);
        let token = if codegen {
            token.with_leading(vec![]).with_trailing(vec![])
        } else {
            token
        };
        (st, Self::R::make_token(token))
    }

    fn make_function_declaration_header(
        st: Bool<'a>,
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
    ) -> (Bool<'a>, Self::R) {
        (
            (st.0 || modifiers.any(|r| Self::is_coroutine(r)), st.1),
            Self::R::make_function_declaration_header(
                modifiers, r2, r3, r4, r5, r6, r7, r8, r9, r10,
            ),
        )
    }

    fn make_closure_type_specifier(
        st: Bool<'a>,
        r1: Self::R,
        coroutine: Self::R,
        r3: Self::R,
        r4: Self::R,
        r5: Self::R,
        r6: Self::R,
        r7: Self::R,
        r8: Self::R,
        r9: Self::R,
    ) -> (Bool<'a>, Self::R) {
        (
            (st.0 || Self::is_coroutine(&coroutine), st.1),
            Self::R::make_closure_type_specifier(r1, coroutine, r3, r4, r5, r6, r7, r8, r9),
        )
    }

    fn make_anonymous_function(
        st: Bool<'a>,
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
    ) -> (Bool<'a>, Self::R) {
        (
            (st.0 || Self::is_coroutine(&coroutine), st.1),
            Self::R::make_anonymous_function(
                r1, r2, r3, coroutine, r5, r6, r7, r8, r9, r10, r11, r12,
            ),
        )
    }

    fn make_lambda_expression(
        st: Bool<'a>,
        r1: Self::R,
        r2: Self::R,
        coroutine: Self::R,
        r4: Self::R,
        r5: Self::R,
        r6: Self::R,
    ) -> (Bool<'a>, Self::R) {
        (
            (st.0 || Self::is_coroutine(&coroutine), st.1),
            Self::R::make_lambda_expression(r1, r2, coroutine, r4, r5, r6),
        )
    }

    fn make_awaitable_creation_expression(
        st: Bool<'a>,
        r1: Self::R,
        r2: Self::R,
        coroutine: Self::R,
        r4: Self::R,
    ) -> (Bool<'a>, Self::R) {
        (
            (st.0 || Self::is_coroutine(&coroutine), st.1),
            Self::R::make_awaitable_creation_expression(r1, r2, coroutine, r4),
        )
    }

    fn make_attribute_specification(
        st: Bool<'a>,
        left: Self::R,
        attribute_name: Self::R,
        right: Self::R,
    ) -> (Bool<'a>, Self::R) {
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
                            .map(|range| &st.1.text()[range.0..range.1]);
                        text.map_or(false, |t| String::from_utf8_lossy(t) == PPL_MACRO_STR)
                    } else {
                        false
                    }
                }
                _ => true,
            }
        };
        (
            (
                st.0 || attribute_name.fold_list(false, is_ppl_attr_folder),
                st.1,
            ),
            Self::R::make_attribute_specification(left, attribute_name, right),
        )
    }
}
