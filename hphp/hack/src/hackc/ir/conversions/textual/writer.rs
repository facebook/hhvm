// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::path::Path;

use anyhow::bail;
use anyhow::Result;
use hash::HashMap;
use itertools::Itertools;
use strum::IntoEnumIterator;

use crate::decls;
use crate::hack;
use crate::mangle::FunctionName;
use crate::textual;
use crate::textual::TextualFile;

const UNIT_START_MARKER: &str = "TEXTUAL UNIT START";
const UNIT_END_MARKER: &str = "TEXTUAL UNIT END";

pub fn textual_writer(
    w: &mut dyn std::io::Write,
    path: &Path,
    unit: ir::Unit,
    no_builtins: bool,
    hide_static_coeffects: bool,
    enable_var_cache: bool,
) -> Result<()> {
    let mut txf = TextualFile::new(w, hide_static_coeffects, enable_var_cache, path);

    let escaped_path = escaper::escape(path.display().to_string());
    txf.write_comment(&format!("{UNIT_START_MARKER} {escaped_path}"))?;

    txf.set_attribute(textual::FileAttribute::SourceLanguage("hack".to_string()))?;
    txf.debug_separator()?;

    check_fatal(path, unit.fatal.as_ref())?;

    // Merge classes and functions so we can sort them and emit in source file
    // order.
    let mut things = unit
        .classes
        .into_iter()
        .map(Thing::Class)
        .chain(unit.functions.into_iter().map(Thing::Func))
        .collect_vec();
    things.sort_by(Thing::cmp);

    for thing in things {
        match thing {
            Thing::Class(cls) => crate::class::write_class(&mut txf, cls)?,
            Thing::Func(func) => crate::func::write_function(&mut txf, func)?,
        }
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

enum Thing {
    Class(ir::Class),
    Func(ir::Function),
}

impl Thing {
    fn line(&self) -> usize {
        match self {
            Thing::Class(c) => c.src_loc.line_begin as usize,
            Thing::Func(f) => f.func.locs[f.func.loc_id].line_begin as usize,
        }
    }

    fn name(&self) -> ir::StringId {
        match self {
            Thing::Class(c) => c.name.as_string_id(),
            Thing::Func(f) => f.name.as_string_id(),
        }
    }

    fn num_params(&self) -> usize {
        match self {
            Thing::Class(_) => 0,
            Thing::Func(f) => f.func.params.len(),
        }
    }

    fn cmp(a: &Thing, b: &Thing) -> Ordering {
        // Start with source line
        a.line()
            .cmp(&b.line())
            .then_with(|| {
                // Same source line? Sort by name.
                a.name().cmp(&b.name())
            })
            .then_with(|| {
                // Same name??? Use number of parameters.
                a.num_params().cmp(&b.num_params())
            })
    }
}
