// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::path::Path;

use anyhow::bail;
use anyhow::Result;

use crate::state::UnitState;
use crate::textual;

pub fn textual_writer(
    w: &mut dyn std::io::Write,
    path: &Path,
    mut unit: ir::Unit<'_>,
) -> Result<()> {
    // steal the StringInterner so we can mutate it while reading the Unit.
    let strings = std::mem::take(&mut unit.strings);

    textual::write_attribute(w, textual::Attribute::SourceLanguage("hack".to_string()))?;
    writeln!(w)?;

    let mut state = UnitState::new(strings);
    check_fatal(path, &unit.fatal)?;

    for cls in unit.classes {
        crate::class::write_class(w, &mut state, cls)?;
    }

    for func in unit.functions {
        crate::func::write_function(w, &mut state, func)?;
    }

    writeln!(w, "// ----- EXTERNALS -----")?;
    for external in state.external_funcs {
        writeln!(w, "declare {}(...): mixed", external)?;
    }
    writeln!(w)?;

    Ok(())
}

fn check_fatal(path: &Path, fatal: &ir::FatalOp<'_>) -> Result<()> {
    match fatal {
        ir::FatalOp::None => Ok(()),
        ir::FatalOp::Parse(loc, msg) => {
            bail!(
                "Parse error in {}[{}]: {}",
                path.display(),
                loc.line_begin,
                msg.as_bstr()
            )
        }
        ir::FatalOp::Runtime(loc, msg) | ir::FatalOp::RuntimeOmitFrame(loc, msg) => {
            bail!(
                "Runtime error in {}[{}]: {}",
                path.display(),
                loc.line_begin,
                msg.as_bstr()
            )
        }
    }
}
