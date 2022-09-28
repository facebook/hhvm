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

macro_rules! tx_ty {
    (mixed) => {
        crate::textual::Ty::Mixed
    };
    ($name:ident) => {
        crate::textual::Ty::RawType(stringify!($name).to_string())
    };
    (* $($rest:tt)+) => {
        crate::textual::Ty::RawPtr(Box::new(tx_ty!($($rest)+)))
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
mod util;
mod writer;

pub use decls::write_decls;
pub use writer::textual_writer;
