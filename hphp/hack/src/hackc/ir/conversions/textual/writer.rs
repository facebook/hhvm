// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;
use std::sync::Arc;

use anyhow::bail;
use anyhow::Result;
use hash::HashMap;
use strum::IntoEnumIterator;

use crate::decls;
use crate::hack;
use crate::mangle::FunctionName;
use crate::state::UnitState;
use crate::textual;
use crate::textual::TextualFile;

const UNIT_START_MARKER: &str = "TEXTUAL UNIT START";
const UNIT_END_MARKER: &str = "TEXTUAL UNIT END";

pub fn textual_writer(
    w: &mut dyn std::io::Write,
    path: &Path,
    unit: ir::Unit<'_>,
    no_builtins: bool,
) -> Result<()> {
    let mut txf = TextualFile::new(w, Arc::clone(&unit.strings));

    let escaped_path = escaper::escape(path.display().to_string());
    txf.write_comment(&format!("{UNIT_START_MARKER} {escaped_path}"))?;

    txf.set_attribute(textual::FileAttribute::SourceLanguage("hack".to_string()))?;
    txf.debug_separator()?;

    let mut state = UnitState::new(Arc::clone(&unit.strings));
    check_fatal(path, unit.fatal.as_ref())?;

    for cls in unit.classes {
        crate::class::write_class(&mut txf, &mut state, cls)?;
    }

    for func in unit.functions {
        crate::func::write_function(&mut txf, &mut state, func)?;
    }

    use hack::Builtin;
    use hack::HackConst;
    use hack::Hhbc;
    let all_builtins: HashMap<FunctionName, Builtin> = Builtin::iter()
        .map(|b| (FunctionName::Builtin(b), b))
        .chain(Hhbc::iter().map(|b| (FunctionName::Builtin(Builtin::Hhbc(b)), Builtin::Hhbc(b))))
        .chain(HackConst::iter().map(|b| {
            let name = FunctionName::Builtin(Builtin::HackConst(b));
            (name, Builtin::HackConst(b))
        }))
        .collect();

    let builtins = txf.write_epilogue(&all_builtins)?;

    if !no_builtins {
        decls::write_decls(&mut txf, &builtins)?;
    }

    txf.write_comment(&format!("{UNIT_END_MARKER} {escaped_path}"))?;
    txf.debug_separator()?;

    Ok(())
}

fn check_fatal(path: &Path, fatal: Option<&ir::Fatal>) -> Result<()> {
    if let Some(fatal) = fatal {
        let err = match fatal.op {
            ir::FatalOp::Parse => "Parse",
            ir::FatalOp::Runtime => "Runtime",
            ir::FatalOp::RuntimeOmitFrame => "Runtime Omit",
            _ => unreachable!(),
        };

        bail!(
            "{err} error in {}[{}]: {}",
            path.display(),
            fatal.loc.line_begin,
            fatal.message
        );
    }

    Ok(())
}
