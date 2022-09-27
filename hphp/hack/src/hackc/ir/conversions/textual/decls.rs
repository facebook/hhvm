// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Error;
use itertools::Itertools;
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
        let name = builtin.into_str();
        match builtin {
            Builtin::ArgPack(_) => {
                for n in 0..5 {
                    let builtin = Builtin::ArgPack(n);
                    let args = (0..=n).into_iter().map(|_| ty!(mixed)).collect_vec();
                    let name = builtin.into_str();
                    declare_function(w, &name, &args, ty!(*HackParams))?
                }
            }

            Builtin::Bool => declare_function(w, &name, &[ty!(bool)], ty!(mixed))?,
            Builtin::Int => declare_function(w, &name, &[ty!(int)], ty!(mixed))?,
            Builtin::Null => declare_function(w, &name, &[], ty!(mixed))?,
            Builtin::String => declare_function(w, &name, &[ty!(string)], ty!(mixed))?,

            Builtin::AllocWords => declare_function(w, &name, &[ty!(int)], ty!(*void))?,
            Builtin::BadMethodCall | Builtin::BadProperty => {
                declare_function(w, &name, &[], ty!(noreturn))?
            }
            Builtin::GetParam => {
                declare_function(w, &name, &[ty!(*HackParam), ty!(int)], ty!(mixed))?
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
        let name = builtin.into_str();
        match hhbc {
            Hhbc::Add
            | Hhbc::AddO
            | Hhbc::CmpEq
            | Hhbc::CmpGt
            | Hhbc::CmpGte
            | Hhbc::CmpLt
            | Hhbc::CmpLte
            | Hhbc::CmpNSame
            | Hhbc::CmpNeq
            | Hhbc::CmpSame
            | Hhbc::Modulo
            | Hhbc::Sub
            | Hhbc::SubO => declare_function(w, &name, &[ty!(mixed), ty!(mixed)], ty!(mixed))?,

            Hhbc::Print | Hhbc::IsTypeInt | Hhbc::Not => {
                declare_function(w, &name, &[ty!(mixed)], ty!(mixed))?
            }
            Hhbc::VerifyFailed => declare_function(w, &name, &[], ty!(noreturn))?,
        }
    }

    Ok(())
}
