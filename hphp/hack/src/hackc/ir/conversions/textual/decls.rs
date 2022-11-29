// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Error;

use crate::hack::Builtin;
use crate::hack::Hhbc;
use crate::textual::TextualFile;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// This is emitted with every SIL file to declare the "standard" definitions
/// that we use.
pub(crate) fn write_decls(txf: &mut TextualFile<'_>) -> Result<()> {
    txf.write_comment("----- BUILTIN DECLS STARTS HERE -----")?;

    Builtin::write_decls(txf)?;
    Hhbc::write_decls(txf)?;

    Ok(())
}
