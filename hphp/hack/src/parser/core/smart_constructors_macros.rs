// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// Usage: S!(make_foo, parser, arg1, ...)  (in OCaml: Make.foo parser arg1 ...)
// Corresponds to a call to Make.foo followed by sc_call in OCaml (see *{precedence,simple}_parser)
// - Make.foo first calls SmartConstructorsWrapper, which calls parser's call method,
// - parser's call method forwards to SmartConstructors' make_foo(parser.sc_state, ...),
//   returning (sc_state, result)
// - parser's sc_state is updated with sc_state, and result is forwarded
// Instead of generating two 183-method SC wrappers in Rust, just use a simple macro.
macro_rules! S {
    // special cases to avoid borrow checker error in common cases (150+) such as:
    //    S!(make_missing, self, self.pos())
    (make_list, $parser: expr, $r: expr, $pos: expr) => {{
        let pos = $pos;
        S_!(make_list, $parser, $r, pos)
    }};
    (make_missing, $parser: expr, $pos: expr) => {{
        let pos = $pos;
        S_!(make_missing, $parser, pos)
    }};
    // general case
    ($f: ident, $parser: expr, $($rs:expr),* $(,)*) => {
        S_!($f, $parser, $($rs),+)
    }
}
macro_rules! S_ {
    ($f: ident, $parser: expr, $($rs:expr),* $(,)*) => {{
        let result = $parser.sc_mut().$f($($rs),+);
        $parser.check_stack_limit();
        result
    }}
}
