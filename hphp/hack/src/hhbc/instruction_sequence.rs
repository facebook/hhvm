// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_ast_rust as ast;
// use label_rust as label;
// use local_rust as local;
//
// use env::iterator::Iter;
// use runtime::TypedValue;

/// The various from_X functions below take some kind of AST (expression,
/// statement, etc.) and produce what is logically a sequence of instructions.
/// This could simply be represented by a list, but then we would need to
/// use an accumulator to avoid the quadratic complexity associated with
/// repeated appending to a list. Instead, we simply build a tree of
/// instructions which can easily be flattened at the end.
pub enum Instr {
    Empty,
    One(Box<Instr>),
    List(Vec<Instr>),
    Concat(Vec<ast::Instruct>),
}

// NOTE(hrust): *fcall_args/flags are ported in hhbc_ast already

// TODO(hrust) port the rest using codegen and/or regexes

#[cfg(test)]
mod test {
    // use super::*;
    // use pretty_assertions::assert_eq; // make assert_eq print huge diffs more human-readable

    #[test]
    fn test_gather() {}
}
