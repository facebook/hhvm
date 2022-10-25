// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Textual is the name of the text format for Infer's IR.  This crate converts
//! from HackC's IR to Textual for ingestion by Infer.
//!
//! End-to-end usage example:
//!   hackc compile-infer test/infer/basic.hack > basic.sil
//!   ./infer/bin/infer compile --capture-textual basic.sil

#![feature(box_patterns)]

/// Helper for tx_ty! Called like `tx_ty_sub!(AllowNaked Pat)` where
/// AllowNaked=0 means naked raw-types not allowed and AllowNaked=1 means naked
/// raw-types are allowed.
macro_rules! tx_ty_sub {
    ($_:tt bool) => {
        textual::Ty::Bool
    };
    ($_:tt int) => {
        textual::Ty::Int
    };
    ($_:tt mixed) => {
        crate::textual::Ty::Mixed
    };
    ($_:tt noreturn) => {
        crate::textual::Ty::Noreturn
    };
    ($_:tt string) => {
        textual::Ty::String
    };
    ($_:tt void) => {
        textual::Ty::Void
    };
    (1 $name:ident) => {
        crate::textual::Ty::Type(stringify!($name).to_owned())
    };
    ($_:tt * $($rest:tt)+) => {
        crate::textual::Ty::Ptr(Box::new(tx_ty_sub!(1 $($rest)+)))
    };
}

/// Build a textual::Ty for a well-defined type.  Handles primitives like `int`
/// or `string` as well as pointer types like `*int` or `*Foo`.  Doesn't allow
/// naked raw-types like `Foo`.
macro_rules! tx_ty {
    ( $($rest:tt)+ ) => {
        tx_ty_sub!(0 $($rest)+)
    };
}

mod class;
mod decls;
mod func;
mod hack;
mod lower;
mod mangle;
mod member_op;
mod state;
mod textual;
mod types;
mod util;
mod writer;

pub use decls::write_decls;
pub use writer::textual_writer;
