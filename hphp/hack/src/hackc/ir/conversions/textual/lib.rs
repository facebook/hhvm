// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Textual is the name of the text format for Infer's IR.  This crate converts
//! from HackC's IR to Textual for ingestion by Infer.

// ./infer/bin/infer compile --capture-textual-sil infer/tests/codetoanalyze/sil/parsing/basic.sil --help

macro_rules! tx_ty {
    (mixed) => {
        textual::Ty::Mixed
    };
    ($name:ident) => {
        textual::Ty::RawType(stringify!($name).to_string())
    };
    (* $rest:tt) => {
        textual::Ty::RawPtr(Box::new(tx_ty!($rest)))
    };
}

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
