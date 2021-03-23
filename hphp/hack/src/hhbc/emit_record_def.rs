// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ast_constant_folder_rust as constant_folder;
use emit_fatal_rust as emit_fatal;
use emit_type_hint_rust as emit_type_hint;
use env::emitter::Emitter;
use hhas_pos_rust::Span;
use hhas_record_def_rust::{Field as RecordField, HhasRecord};
use hhas_type::constraint;
use hhbc_id_rust::record;
use instruction_sequence::Result;
use oxidized::namespace_env::Env as Namespace;

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

fn emit_field<'a>(
    emitter: &Emitter,
    namespace: &Namespace,
    field: &'a (Sid, Hint, Option<Expr>),
) -> Result<RecordField<'a>> {
    let (Id(pos, name), hint, expr_opt) = field;
    let otv = expr_opt.as_ref().map_or(None, |e| {
        constant_folder::expr_to_typed_value(emitter, namespace, e).ok()
    });
    let ti = emit_type_hint::hint_to_type_info(
        &emit_type_hint::Kind::Property,
        false,
        false,
        &[],
        &hint,
    )?;
    if valid_tc_for_record_field(&ti.type_constraint) {
        Ok(RecordField(name.as_str(), ti, otv))
    } else {
        let name = string_utils::strip_global_ns(name);
        Err(emit_fatal::raise_fatal_parse(
            &pos,
            format!("Invalid record field type hint for '{}'", name),
        ))
    }
}

fn emit_record_def<'a>(emitter: &Emitter, rd: &'a RecordDef) -> Result<HhasRecord<'a>> {
    fn elaborate<'b>(Id(_, name): &'b Id) -> record::Type<'b> {
        name.trim_start_matches("\\").into()
    }
    let parent_name = match &rd.extends {
        Some(Hint(_, h)) => {
            if let Hint_::Happly(id, _) = h.as_ref() {
                Some(elaborate(id))
            } else {
                None
            }
        }
        _ => None,
    };
    Ok(HhasRecord {
        name: elaborate(&rd.name),
        is_abstract: rd.abstract_,
        base: parent_name,
        fields: rd
            .fields
            .iter()
            .map(|f| emit_field(emitter, rd.namespace.as_ref(), &f))
            .collect::<Result<Vec<_>>>()?,
        span: Span::from_pos(&rd.span),
    })
}

pub fn emit_record_defs_from_program<'a>(
    emitter: &Emitter,
    p: &'a Program,
) -> Result<Vec<HhasRecord<'a>>> {
    p.into_iter()
        .filter_map(|d| d.as_record_def().map(|r| emit_record_def(emitter, r)))
        .collect::<Result<Vec<_>>>()
}
