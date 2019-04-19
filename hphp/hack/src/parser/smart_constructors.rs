/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
pub use crate::smart_constructors_generated::*;

// Usage: sc_call(make_foo, parser, arg1, ...)  (in OCaml: Make.foo parser arg1 ...)
// Corresponds to a call to Make.foo followed by sc_call in OCaml (see *{precedence,simple}_parser)
// - Make.foo first calls SmartConstructorsWrapper, which calls parser's call method,
// - parser's call method forwards to SmartConstructors' make_foo(parser.sc_state, ...),
//   returning (sc_state, result)
// - parser's sc_state is updated with sc_state, and result is forwarded
// Instead of generating two 183-method SC wrappers in Rust, just use a simple macro.
macro_rules! S {
    ($f: ident, $parser: expr, $($rs:expr),* $(,)*) => {{
        // Invariant: sc_state is None only during this macro call, so unwrap calls don't panic
        let sc_state0 = std::mem::replace(&mut *($parser.sc_state_mut()), None).unwrap();
        let (sc_state, result) = S::$f(sc_state0, $($rs),+);
        *($parser.sc_state_mut()) = Some(sc_state);
        result
    }}
}

#[derive(Clone)]
pub struct NoState;  // zero-overhead placeholder when there is no state

pub trait NodeType {
    fn is_missing(&self) -> bool;
    fn is_abstract(&self) -> bool;
    fn is_variable_expression(&self) -> bool;
    fn is_subscript_expression(&self) -> bool;
    fn is_member_selection_expression(&self) -> bool;
    fn is_scope_resolution_expression(&self) -> bool;
    fn is_object_creation_expression(&self) -> bool;
    fn is_qualified_name(&self) -> bool;
    fn is_safe_member_selection_expression(&self) -> bool;
    fn is_function_call_expression(&self) -> bool;
    fn is_list_expression(&self) -> bool;
    fn is_name(&self) -> bool;
    fn is_halt_compiler_expression(&self) -> bool;
    fn is_prefix_unary_expression(&self) -> bool;
}
