// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

//! Textual is the name of the text format for Infer's IR.  This crate converts
//! from HackC's IR to Textual for ingestion by Infer.

mod writer;

pub use writer::textual_writer;
