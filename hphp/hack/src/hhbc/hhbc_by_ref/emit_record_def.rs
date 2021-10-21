// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ffi::{Maybe, Maybe::*, Slice, Str};
use hhbc_by_ref_ast_constant_folder as constant_folder;
use hhbc_by_ref_emit_fatal as emit_fatal;
use hhbc_by_ref_emit_type_hint as emit_type_hint;
use hhbc_by_ref_env::emitter::Emitter;
use hhbc_by_ref_hhas_pos::HhasSpan;
use hhbc_by_ref_hhas_record_def::{Field as RecordField, HhasRecord};
use hhbc_by_ref_hhas_type::constraint;
use hhbc_by_ref_hhbc_id::record::RecordType;
use hhbc_by_ref_hhbc_string_utils as string_utils;
use hhbc_by_ref_instruction_sequence::Result;
use oxidized::ast::*;

fn valid_tc_for_record_field(tc: &constraint::Constraint<'_>) -> bool {
    match &tc.name {
        Nothing => true,
        Just(name) => {
            !(name.as_str().eq_ignore_ascii_case("hh\\this")
                || name.as_str().eq_ignore_ascii_case("callable")
                || name.as_str().eq_ignore_ascii_case("hh\\nothing")
                || name.as_str().eq_ignore_ascii_case("hh\\noreturn"))
        }
    }
}

fn emit_field<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &Emitter<'arena, 'decl>,
    field: &'a (Sid, Hint, Option<Expr>),
) -> Result<RecordField<'arena>> {
    let (Id(pos, name), hint, expr_opt) = field;
    let otv = expr_opt
        .as_ref()
        .and_then(|e| constant_folder::expr_to_typed_value(alloc, emitter, e).ok());
    let ti = emit_type_hint::hint_to_type_info(
        alloc,
        &emit_type_hint::Kind::Property,
        false,
        false,
        &[],
        &hint,
    )?;
    if valid_tc_for_record_field(&ti.type_constraint) {
        Ok(RecordField(
            Str::new_str(alloc, name.as_str()),
            ti,
            Maybe::from(otv),
        ))
    } else {
        let name = string_utils::strip_global_ns(name);
        Err(emit_fatal::raise_fatal_parse(
            &pos,
            format!("Invalid record field type hint for '{}'", name),
        ))
    }
}

fn emit_record_def<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &Emitter<'arena, 'decl>,
    rd: &'a RecordDef,
) -> Result<HhasRecord<'arena>> {
    fn elaborate<'arena>(alloc: &'arena bumpalo::Bump, Id(_, name): &Id) -> RecordType<'arena> {
        (alloc, name.trim_start_matches('\\')).into()
    }
    let parent_name = match &rd.extends {
        Some(Hint(_, h)) => {
            if let Hint_::Happly(id, _) = h.as_ref() {
                Some(elaborate(alloc, id))
            } else {
                None
            }
        }
        _ => None,
    };
    let fields = rd
        .fields
        .iter()
        .map(|f| emit_field(alloc, emitter, &f))
        .collect::<Result<Vec<_>>>()?;
    Ok(HhasRecord {
        name: elaborate(alloc, &rd.name),
        is_abstract: rd.abstract_,
        base: Maybe::from(parent_name),
        fields: Slice::fill_iter(alloc, fields.into_iter()),
        span: HhasSpan::from_pos(&rd.span),
    })
}

pub fn emit_record_defs_from_program<'a, 'arena, 'decl>(
    alloc: &'arena bumpalo::Bump,
    emitter: &Emitter<'arena, 'decl>,
    p: &'a [Def],
) -> Result<Vec<HhasRecord<'arena>>> {
    p.iter()
        .filter_map(|d| {
            d.as_record_def()
                .map(|r| emit_record_def(alloc, emitter, r))
        })
        .collect::<Result<Vec<_>>>()
}
