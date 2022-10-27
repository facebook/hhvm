// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Error;
use strum::IntoEnumIterator as _;

use crate::hack::Builtin;
use crate::hack::Hhbc;
use crate::textual;

type Result<T = (), E = Error> = std::result::Result<T, E>;

/// This is emitted with every SIL file to declare the "standard" definitions
/// that we use.
pub fn write_decls(w: &mut dyn std::io::Write) -> Result<()> {
    writeln!(w, "// ----- BUILTIN DELS STARTS HERE -----")?;

    use textual::declare_function;
    use tx_ty as ty;

    // It's tempting to make a derive macro so we can encode this stuff right in
    // the definition... but that's probably too much work for now. Maybe once
    // strum can support non-string constants we can use that (otherwise we have
    // to do string-parsing at runtime).
    for builtin in Builtin::iter() {
        let name = builtin.to_string();
        match builtin {
            Builtin::Bool => declare_function(w, &name, &[ty!(bool)], ty!(*HackBool))?,
            Builtin::Int => declare_function(w, &name, &[ty!(int)], ty!(*HackInt))?,
            Builtin::Null => declare_function(w, &name, &[], ty!(*HackNull))?,
            Builtin::String => declare_function(w, &name, &[ty!(string)], ty!(*HackString))?,

            Builtin::AllocWords => declare_function(w, &name, &[ty!(int)], ty!(*void))?,
            Builtin::BadMethodCall | Builtin::BadProperty => {
                declare_function(w, &name, &[], ty!(noreturn))?
            }
            Builtin::IsTrue => declare_function(w, &name, &[ty!(mixed)], ty!(bool))?,
            Builtin::RawPtrIsNull => declare_function(w, &name, &[ty!(*void)], ty!(bool))?,
            Builtin::VerifyParamCount => declare_function(
                w,
                &name,
                &[ty!(*HackParam), ty!(int), ty!(int)],
                ty!(noreturn),
            )?,

            Builtin::Hhbc(_) => {}
        }
    }

    for hhbc in Hhbc::iter() {
        let builtin = Builtin::Hhbc(hhbc);
        let name = builtin.to_string();
        match hhbc {
            Hhbc::Add
            | Hhbc::CmpEq
            | Hhbc::CmpGt
            | Hhbc::CmpGte
            | Hhbc::CmpLt
            | Hhbc::CmpLte
            | Hhbc::CmpNSame
            | Hhbc::CmpNeq
            | Hhbc::CmpSame
            | Hhbc::Modulo
            | Hhbc::Sub => declare_function(w, &name, &[ty!(mixed), ty!(mixed)], ty!(mixed))?,

            Hhbc::Print | Hhbc::IsTypeInt | Hhbc::IsTypeStr | Hhbc::Not => {
                declare_function(w, &name, &[ty!(mixed)], ty!(mixed))?
            }
            Hhbc::VerifyFailed => declare_function(w, &name, &[], ty!(noreturn))?,
        }
    }

    Ok(())
}
