// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::io::Result;
use std::io::Write;

use hhbc::hhas_coeffects::HhasCoeffects;
use write_bytes::write_bytes;
use write_bytes::DisplayBytes;

use crate::context::Context;
use crate::context::FmtIndent;
use crate::write::fmt_separated;
use crate::write::fmt_separated_with;

pub(crate) fn coeffects_to_hhas(
    ctx: &Context<'_>,
    w: &mut dyn Write,
    coeffects: &HhasCoeffects<'_>,
) -> Result<()> {
    let indent = FmtIndent(ctx);

    let static_coeffects = coeffects.get_static_coeffects();
    let unenforced_static_coeffects = coeffects.get_unenforced_static_coeffects();
    if !static_coeffects.is_empty() || !unenforced_static_coeffects.is_empty() {
        write_bytes!(
            w,
            "\n{}.coeffects_static {};",
            indent,
            fmt_separated(
                " ",
                static_coeffects
                    .iter()
                    .map(|co| co as &dyn DisplayBytes)
                    .chain(
                        unenforced_static_coeffects
                            .iter()
                            .map(|co| co as &dyn DisplayBytes),
                    ),
            ),
        )?;
    }

    let fun_params = coeffects.get_fun_param();
    if !fun_params.is_empty() {
        write_bytes!(
            w,
            "\n{}.coeffects_fun_param {};",
            indent,
            fmt_separated(" ", fun_params)
        )?;
    }

    let cc_params = coeffects.get_cc_param();
    if !cc_params.is_empty() {
        write_bytes!(
            w,
            "\n{}.coeffects_cc_param {};",
            indent,
            fmt_separated_with(" ", cc_params, |w, c| write_bytes!(w, "{} {}", c.0, c.1))
        )?;
    }

    for v in coeffects.get_cc_this() {
        write_bytes!(
            w,
            "\n{}.coeffects_cc_this {};",
            indent,
            fmt_separated(" ", v.iter())
        )?;
    }

    for v in coeffects.get_cc_reified() {
        write_bytes!(
            w,
            "\n{}.coeffects_cc_reified {}{} {};",
            indent,
            if v.0 { "isClass " } else { "" },
            v.1,
            fmt_separated(" ", v.2.iter())
        )?;
    }

    if coeffects.is_closure_parent_scope() {
        write!(w, "\n{}.coeffects_closure_parent_scope;", indent)?;
    }

    if coeffects.generator_this() {
        write!(w, "\n{}.coeffects_generator_this;", indent)?;
    }

    if coeffects.caller() {
        write!(w, "\n{}.coeffects_caller;", indent)?;
    }

    Ok(())
}
