// Copyright (Constructor) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::borrow::Borrow;

use crate::lexable_token::LexableToken;
use crate::source_text::SourceText;
use crate::syntax_by_ref::syntax::Syntax;
use crate::syntax_error::SyntaxError;
use crate::syntax_trait::SyntaxTrait;

pub struct SyntaxTree<'a, Syntax, State> {
    text: &'a SourceText<'a>,
    root: Syntax,
    errors: Vec<SyntaxError>,
    mode: Option<FileMode>,
    state: State,
}

impl<'arena, T, V, State> SyntaxTree<'_, Syntax<'arena, T, V>, State>
where
    T: LexableToken,
    Syntax<'arena, T, V>: SyntaxTrait,
{
    pub fn errors<'r>(&'r self) -> Vec<&SyntaxError>
    where
        State: Clone,
        'r: 'arena,
    {
        let mut errs: Vec<&SyntaxError> = if self.is_hhi() {
            self.errors
                .iter()
                .filter(|e| !Self::is_in_body(&self.root, e.start_offset))
                .collect()
        } else {
            self.errors.iter().collect()
        };
        Self::remove_duplicates(&mut errs, SyntaxError::equal_offset);
        errs
    }

    fn is_in_body<'r>(node: &'r Syntax<'arena, T, V>, position: usize) -> bool
    where
        'r: 'arena,
    {
        let p = Self::parentage(node, position);
        for i in 1..p.len() {
            if p[i - 1].is_compound_statement()
                && (p[i].is_methodish_declaration() || p[i].is_function_declaration())
            {
                return true;
            }
        }
        false
    }

    // TODO:(shiqicao) move this function to SyntaxTrait, see D18359931
    fn parentage<'r>(
        node: &'r Syntax<'arena, T, V>,
        position: usize,
    ) -> Vec<&'r Syntax<'arena, T, V>>
    where
        'r: 'arena,
    {
        let mut acc = vec![];
        if position < node.full_width() {
            Self::parentage_(node, position, &mut acc);
        }
        acc
    }

    fn parentage_<'r>(
        node: &'r Syntax<'arena, T, V>,
        mut position: usize,
        acc: &mut Vec<&'r Syntax<'arena, T, V>>,
    ) where
        'r: 'arena,
    {
        for c in node.iter_children() {
            let width = node.full_width();
            if position < width {
                Self::parentage_(c, position, acc);
                break;
            } else {
                position -= width;
            }
        }
        acc.push(node)
    }
}

impl<'a, Syntax, State> SyntaxTree<'a, Syntax, State>
where
    State: Clone,
{
    pub fn build(
        text: &'a SourceText<'a>,
        root: Syntax,
        errors: Vec<SyntaxError>,
        mode: Option<FileMode>,
        state: State,
    ) -> Self {
        Self {
            text,
            root,
            errors,
            mode,
            state,
        }
    }

    // Convert a foreign pointer to a `SyntaTree` reference, the tree is borrowed
    // from forgein caller, therefore Rust shouldn't drop the content.
    pub unsafe fn ffi_pointer_as_ref(
        ptr: usize,
        text: &'a SourceText<'a>,
    ) -> Result<&'a Self, String> {
        let raw_tree = ptr as *mut SyntaxTree<'_, Syntax, State>;
        let tree = match raw_tree.as_mut() {
            Some(t) => t,
            None => return Err("null raw tree pointer".into()),
        };
        // The tree already contains source text, but this source text contains a pointer
        // into OCaml heap, which might have been invalidated by GC in the meantime.
        // Replacing the source text with a current one prevents it. This will still end
        // horribly if the tree starts storing some other pointers into source text,
        // but it's not the case at the moment.
        tree.replace_text_unsafe(text);
        Ok(tree)
    }

    // Convert a foreign pointer to boxed `SyntaxTree`. This function can be used when foreign
    // caller moves a `SyntaxTree` to Rust, when `Box` goes out of scope the SyntaxTree will
    // be dropped.
    pub unsafe fn ffi_pointer_into_boxed(ptr: usize, text: &'a SourceText<'a>) -> Box<Self> {
        let tree_pointer = ptr as *mut Self;
        let mut tree = Box::from_raw(tree_pointer);
        // The tree already contains source text, but this source text contains a pointer
        // into OCaml heap, which might have been invalidated by GC in the meantime.
        // Replacing the source text with a current one prevents it.
        // This will still end horribly if the tree starts storing some
        // other pointers into source text, but it's not the case at the moment.
        tree.replace_text_unsafe(text);
        tree
    }

    pub fn create(
        text: &'a SourceText<'a>,
        root: Syntax,
        mut errors: Vec<SyntaxError>,
        mode: Option<FileMode>,
        state: State,
    ) -> Self {
        Self::process_errors(&mut errors);
        Self::build(text, root, errors, mode, state)
    }

    fn remove_duplicates<F>(errors: &mut Vec<impl Borrow<SyntaxError>>, equals: F)
    where
        F: Fn(&SyntaxError, &SyntaxError) -> bool,
    {
        errors.dedup_by(|e1, e2| equals((*e1).borrow(), (*e2).borrow()));
    }

    fn process_errors(errors: &mut Vec<SyntaxError>) {
        /* We've got the lexical errors and the parser errors together, both
        with lexically later errors earlier in the list. We want to reverse the
        list so that later errors come later, and then do a stable sort to put the
        lexical and parser errors together. */
        errors.reverse();
        // Vec::sort_by is a stable sort, which is required here
        errors.sort_by(SyntaxError::compare_offset);
        Self::remove_duplicates(errors, SyntaxError::weak_equal);
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

    pub fn mode(&self) -> Option<FileMode> {
        self.mode
    }

    pub fn sc_state(&self) -> &State {
        &self.state
    }

    pub fn is_strict(&self) -> bool {
        self.mode == Some(FileMode::Strict)
    }

    pub fn is_hhi(&self) -> bool {
        self.mode == Some(FileMode::Hhi)
    }

    // "unsafe" because it can break the invariant that text is consistent with other syntax
    // tree members
    pub fn replace_text_unsafe(&mut self, text: &'a SourceText<'a>) {
        self.text = text
    }

    pub fn drop_state(self) -> SyntaxTree<'a, Syntax, ()> {
        SyntaxTree {
            text: self.text,
            root: self.root,
            errors: self.errors,
            mode: self.mode,
            state: (),
        }
    }

    //TODO: to_json require some unimplemented methods in syntax.rs, particularly
    // Hh_json.to_json
}

impl<'a, Syntax, State> AsRef<SyntaxTree<'a, Syntax, State>> for SyntaxTree<'a, Syntax, State> {
    fn as_ref(&self) -> &Self {
        self
    }
}

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
#[repr(u8)]
pub enum FileMode {
    /// just declare signatures, don't check anything
    Hhi,
    /// check everything!
    Strict,
}

#[cfg(test)]
mod tests {
    use crate::syntax_error;
    use crate::syntax_tree::SyntaxTree;

    #[test]
    fn test_process_errors() {
        let mut test_errors = vec![
            syntax_error::SyntaxError::make(0, 10, syntax_error::error0001, vec![]),
            syntax_error::SyntaxError::make(10, 20, syntax_error::error0001, vec![]),
            syntax_error::SyntaxError::make(0, 10, syntax_error::error0001, vec![]),
        ];
        SyntaxTree::<(), ()>::process_errors(&mut test_errors);
        assert_eq!(
            vec![
                syntax_error::SyntaxError::make(0, 10, syntax_error::error0001, vec![]),
                syntax_error::SyntaxError::make(10, 20, syntax_error::error0001, vec![]),
            ],
            test_errors
        );
    }
}
