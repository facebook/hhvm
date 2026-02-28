// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// A macro to build Hack Expr trees.
///
/// Usage:
///     hack_expr!(pos = p, "#foo + $bar")
///
/// Returns an ast::Expr representing the given code.
///
/// The `pos` parameter is optional.  If not provided then a Pos::none() will be
/// used.
///
/// The code string must be a string literal. It cannot be a dynamically
/// generated string.
///
/// The code string can contain replacements to inject values into the generated
/// tree:
///
///     hack_expr!("$foo + #bar + $baz")
///
/// The replacements have various forms:
///   - `#name` Inject the Expr or Stmt in local variable `name`.
///   - `#{args*}` As a parameter to a call this inserts a Vec<Expr> as
///     ParamKind::Pnormal parameters.
///   - `#{clone(name)}` Clone the Expr `name` instead of consuming it.
///   - `#{cmd(name)}` Convert name using 'cmd' (see below).
///   - `#{cmd(clone(name))}` Clone `name` and then convert using 'cmd' (see below).
///
/// Conversion commands:
///
///   - `#{id(name)}` builds an Expr_::Ident from a string.
///   - `#{lvar(name)}` builds an Expr_::LVar from a LocalId.
///   - `#{str(name)}` builds an Expr_::String from a String.
///
/// All of the commands can also take an optional position override parameter:
///   - `#{str(name, pos)}`
///   - `#{str(clone(name), pos)}`
///
///
/// Technical note:
///
/// The transformation is done at hackc compile time - the macro takes the input
/// string and uses the aast parser to parse it and generate an Expr.  It then
/// traverses the the Expr (again - at compile time) to generate code to
/// construct the Expr at runtime.  The Hack code needs to be quoted because the
/// Rust parser doesn't like some Hack constructs (in particular backslash
/// separated identifiers).
///
///     hack_expr!(pos = p, "#foo + $bar")
///
/// transforms into something like this:
///
///     Expr(
///         (),
///         p.clone(),
///         Expr_::Binop(Box::new((
///             Bop::Plus,
///             foo,
///             Expr((), p.clone(), Expr_::Lvar(Lid(p.clone(), (0, "$bar")))),
///         )))
///     )
#[macro_export]
macro_rules! hack_expr {
    ($($input:tt)*) => {
        $crate::exports::hack_macros_impl::hack_expr_proc! {
            // See the comment in `hack_expr_impl` for what this is.
            $crate::exports
            $($input)*
        }
    }
}

/// Like `hack_expr!` but produces an ast::Stmt value (see `hack_expr!` for
/// full docs).
#[macro_export]
macro_rules! hack_stmt {
    ($($input:tt)*) => {
        $crate::exports::hack_macros_impl::hack_stmt_proc! {
            // See the comment in `hack_expr_impl` for what this is.
            $crate::exports
            $($input)*
        }
    }
}

/// Like `hack_stmt!` but produces a `Vec<ast::Stmt>` value (see `hack_expr!`
/// for full docs).
#[macro_export]
macro_rules! hack_stmts {
    ($($input:tt)*) => {
        $crate::exports::hack_macros_impl::hack_stmts_proc! {
            // See the comment in `hack_expr_impl` for what this is.
            $crate::exports
            $($input)*
        }
    }
}

pub mod exports {
    pub use hack_macros_impl;
    pub use oxidized::ast;
}
