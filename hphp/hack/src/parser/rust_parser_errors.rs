// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::{syntax::Syntax, syntax_error::SyntaxError, syntax_tree::SyntaxTree};

use oxidized::parser_options::ParserOptions;

pub struct ParserErrors<'a, Token, Value, State> {
    _tree: &'a SyntaxTree<'a, Syntax<Token, Value>, State>,
    phanotm: std::marker::PhantomData<(*const Token, *const Value, *const State)>,
}

impl<'a, Token, Value, State> ParserErrors<'a, Token, Value, State>
where
    State: Clone,
{
    fn new(_tree: &'a SyntaxTree<Syntax<Token, Value>, State>) -> Self {
        Self {
            _tree,
            phanotm: std::marker::PhantomData,
        }
    }

    fn parse_errors_impl(&self) -> Vec<SyntaxError> {
        vec![]
    }

    pub fn parse_errors(
        tree: &'a SyntaxTree<Syntax<Token, Value>, State>,
        _parser_options: ParserOptions,
    ) -> Vec<SyntaxError> {
        match tree.required_stack_size() {
            None => Self::new(tree).parse_errors_impl(),
            Some(stack_size) => {
                // We want to use new thread ONLY for it's capability of adjustable stack size.
                // Rust is against it because SyntaxTree is not a thread safe structure. Moreover,
                // spawned thread could outlive the 'a lifetime.
                // Since the only thing the main thread will do after spawning the child is to wait
                // for it to finish, there will be no actual concurrency, and we can "safely" unsafe
                // it. The usize cast is to fool the borrow checker about the thread lifetime and 'a.
                let raw_pointer = Box::leak(Box::new(Self::new(tree))) as *mut Self as usize;
                std::thread::Builder::new()
                    .stack_size(stack_size)
                    .spawn(move || {
                        let self_ = unsafe { Box::from_raw(raw_pointer as *mut Self) };
                        self_.parse_errors_impl()
                    })
                    .expect("ERROR: thread::spawn")
                    .join()
                    .expect("ERROR: failed to wait on new thread")
            }
        }
    }
}
