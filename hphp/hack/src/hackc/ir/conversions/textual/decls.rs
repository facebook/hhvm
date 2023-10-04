// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Error;
use hash::HashSet;

use crate::hack::Builtin;
use crate::hack::HackConst;
use crate::hack::Hhbc;
use crate::textual::TextualFile;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// This is emitted with every SIL file to declare the "standard" definitions
/// that we use.
pub(crate) fn write_decls(txf: &mut TextualFile<'_>, subset: &HashSet<Builtin>) -> Result<()> {
    txf.write_comment("----- BUILTIN DECLS STARTS HERE -----")?;

    Builtin::write_decls(txf, subset)?;

    let hhbc_subset = subset
        .iter()
        .filter_map(|builtin| match builtin {
            Builtin::Hhbc(hhbc) => Some(*hhbc),
            _ => None,
        })
        .collect();
    Hhbc::write_decls(txf, &hhbc_subset)?;

    let hackconst_subset = subset
        .iter()
        .filter_map(|builtin| match builtin {
            Builtin::HackConst(hc) => Some(*hc),
            _ => None,
        })
        .collect();
    HackConst::write_decls(txf, &hackconst_subset)?;

    txf.debug_separator()?;

    Ok(())
}
