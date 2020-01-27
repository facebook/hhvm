// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ast_constant_folder_rust as constant_folder;
use emit_fatal_rust as emit_fatal;
use emit_type_hint_rust as emit_type_hint;
use env::emitter::Emitter;
use hhas_record_def_rust::{Field as RecordField, HhasRecord};
use hhas_type::constraint;
use instruction_sequence_rust::Result;

use hhbc_string_utils_rust as string_utils;
use oxidized::ast::*;

fn valid_tc_for_record_field(tc: &constraint::Type) -> bool {
    match &tc.name {
        None => true,
        Some(name) => {
            !(string_utils::is_self(name)
                || string_utils::is_parent(name)
                || name.eq_ignore_ascii_case("hh\\this")
                || name.eq_ignore_ascii_case("callable")
                || name.eq_ignore_ascii_case("hh\\nothing")
                || name.eq_ignore_ascii_case("hh\\noreturn"))
        }
    }
}

fn emit_field(emitter: &Emitter, field: (Sid, Hint, Option<Expr>)) -> Result<RecordField> {
    let (Id(pos, mut name), hint, expr_opt) = field;
    let otv = expr_opt.map_or(None, |e| {
        constant_folder::expr_to_opt_typed_value(emitter, &e)
    });
    let ti = emit_type_hint::hint_to_type_info(
        &emit_type_hint::Kind::Property,
        false,
        false,
        &[],
        &hint,
    );
    if valid_tc_for_record_field(&ti.type_constraint) {
        Ok(RecordField(name, ti, otv))
    } else {
        name = string_utils::strip_ns(&name).to_string();
        Err(emit_fatal::raise_fatal_parse(
            &pos,
            format!("Invalid record field type hint for '{}'", name),
        ))
    }
}

fn emit_record_def<'a>(emitter: &Emitter, rd: RecordDef) -> HhasRecord<'a> {
    let elaborate = |Id(_, name)| name.trim_start_matches("\\").to_string().into();
    let parent_name = match rd.extends {
        Some(Hint(_, h)) => {
            if let Hint_::Happly(id, _) = *h {
                Some(elaborate(id))
            } else {
                None
            }
        }
        _ => None,
    };
    HhasRecord {
        name: elaborate(rd.name),
        is_abstract: rd.abstract_,
        base: parent_name,
        fields: rd
            .fields
            .into_iter()
            .map(|f| emit_field(emitter, f).unwrap())
            .collect(),
    }
}

pub fn emit_record_defs_from_program<'a>(emitter: &Emitter, p: Program) -> Vec<HhasRecord<'a>> {
    p.into_iter()
        .filter_map(|def| match def {
            Def::RecordDef(rd) => Some(emit_record_def(emitter, *rd)),
            _ => None,
        })
        .collect()
}
