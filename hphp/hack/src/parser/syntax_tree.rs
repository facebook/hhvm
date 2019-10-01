// Copyright (Constructor) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::cmp::Ordering;

pub mod mode_parser;

use crate::mode_parser::parse_mode;
use oxidized::file_info::Mode;
use parser_core_types::{source_text::SourceText, syntax_error::SyntaxError};
use parser_rust::{
    parser::Parser,
    parser_env::ParserEnv,
    smart_constructors::{NodeType, SmartConstructors},
};

pub struct SyntaxTree<'a, Syntax, State> {
    text: &'a SourceText<'a>,
    root: Syntax,
    errors: Vec<SyntaxError>,
    mode: Option<Mode>,
    state: State,
    required_stack_size: Option<usize>,
}

fn dedup_errors(errors: Vec<SyntaxError>) -> Vec<SyntaxError> {
    let mut errors = errors.clone();
    errors.dedup();
    errors.reverse();
    errors
}

fn process_errors(errors: Vec<SyntaxError>) -> Vec<SyntaxError> {
    /* We've got the lexical errors and the parser errors together, both
    with lexically later errors earlier in the list. We want to reverse the
    list so that later errors come later, and then do a stable sort to put the
    lexical and parser errors together. */
    let mut errors = errors.clone();
    errors.reverse();
    // Vec::sort_by is a stable sort, which is required here
    errors.sort_by(|e1, e2| {
        if e1.start_offset < e2.start_offset {
            Ordering::Less
        } else if e1.start_offset > e2.start_offset {
            Ordering::Greater
        } else if e1.end_offset < e2.end_offset {
            Ordering::Less
        } else if e1.end_offset > e2.end_offset {
            Ordering::Greater
        } else {
            Ordering::Equal
        }
    });
    dedup_errors(errors)
}

impl<'a, Syntax, State> SyntaxTree<'a, Syntax, State>
where
    State: Clone,
{
    pub fn build(
        text: &'a SourceText<'a>,
        root: Syntax,
        errors: Vec<SyntaxError>,
        mode: Option<Mode>,
        state: State,
        required_stack_size: Option<usize>,
    ) -> Self {
        Self {
            text,
            root,
            errors,
            mode,
            state,
            required_stack_size,
        }
    }

    // Convert a foreign pointer to a `SyntaTree` reference, the tree is borrowed
    // from forgein caller, therefore Rust shouldn't drop the content.
    pub unsafe fn ffi_pointer_as_ref(
        ptr: usize,
        text: &'a SourceText<'a>,
    ) -> Result<&'a Self, String> {
        let raw_tree = ptr as *mut SyntaxTree<Syntax, State>;
        let tree = match raw_tree.as_mut() {
            Some(t) => t,
            None => return Err(String::from("null raw tree pointer")),
        };
        // The tree already contains source text, but this source text contains a pointer into OCaml
        // heap, which might have been invalidated by GC in the meantime. Replacing the source text
        // with a current one prevents it. This will still end horribly if the tree starts storing some
        // other pointers into source text, but it's not the case at the moment.
        tree.replace_text_unsafe(text);
        Ok(tree)
    }

    // Convert a foreign pointer to boxed `SyntaxTree`. This function can be used when foreign
    // caller moves a `SyntaxTree` to Rust, when `Box` goes out of scope the SyntaxTree will
    // be dropped.
    pub unsafe fn ffi_pointer_into_boxed(ptr: usize, text: &'a SourceText<'a>) -> Box<Self> {
        let tree_pointer = ptr as *mut Self;
        let mut tree = Box::from_raw(tree_pointer);
        // The tree already contains source text, but this source text contains a pointer into OCaml
        // heap, which might have been invalidated by GC in the meantime. Replacing the source text
        // with a current one prevents it. This will still end horribly if the tree starts storing some
        // other pointers into source text, but it's not the case at the moment.
        tree.replace_text_unsafe(text);
        tree
    }

    pub fn create(
        text: &'a SourceText<'a>,
        root: Syntax,
        errors: Vec<SyntaxError>,
        mode: Option<Mode>,
        state: State,
        required_stack_size: Option<usize>,
    ) -> Self {
        let errors = process_errors(errors);
        Self::build(text, root, errors, mode, state, required_stack_size)
    }

    pub fn root(&self) -> &Syntax {
        &self.root
    }

    pub fn text(&self) -> &'a SourceText<'a> {
        self.text
    }

    pub fn all_errors(&self) -> &[SyntaxError] {
        &self.errors
    }

    pub fn mode(&self) -> &Option<Mode> {
        &self.mode
    }

    pub fn sc_state(&self) -> &State {
        &self.state
    }

    pub fn is_hack(&self) -> bool {
        self.mode != Some(Mode::Mphp)
    }

    pub fn is_php(&self) -> bool {
        self.mode == Some(Mode::Mphp)
    }

    pub fn is_strict(&self) -> bool {
        self.mode == Some(Mode::Mstrict)
    }

    pub fn is_decl(&self) -> bool {
        self.mode == Some(Mode::Mdecl)
    }

    pub fn required_stack_size(&self) -> Option<usize> {
        self.required_stack_size
    }

    // "unsafe" because it can break the invariant that text is consistent with other syntax
    // tree members
    pub fn replace_text_unsafe(&mut self, text: &'a SourceText<'a>) {
        self.text = text
    }

    //TODO: errors and to_json require some unimplemented methods in syntax.rs, particularly
    // is_in_body and and Hh_json.to_json
}

pub fn make_syntax_tree<'a, S, T>(
    source: &'a SourceText<'a>,
    env: ParserEnv,
) -> SyntaxTree<<S::R as NodeType>::R, S>
where
    T: Clone,
    S: SmartConstructors<'a, T>,
    S::R: NodeType,
{
    let mode = parse_mode(&source);
    let mut parser = Parser::make(&source, env);
    let root = parser.parse_script(None);
    let (_, errors, _, state) = parser.into_parts();
    SyntaxTree::create(source, root, errors, mode, state, None)
}

#[cfg(test)]
mod tests {
    use crate::process_errors;
    use parser_core_types::syntax_error;

    #[test]
    fn test_process_errors() {
        let test_errors = vec![
            syntax_error::SyntaxError::make(0, 10, syntax_error::error0001),
            syntax_error::SyntaxError::make(10, 20, syntax_error::error0001),
            syntax_error::SyntaxError::make(0, 10, syntax_error::error0001),
        ];
        let processed_test_errors = process_errors(test_errors);
        assert_eq!(
            vec![
                syntax_error::SyntaxError::make(10, 20, syntax_error::error0001),
                syntax_error::SyntaxError::make(0, 10, syntax_error::error0001),
            ],
            processed_test_errors
        );
    }
}
