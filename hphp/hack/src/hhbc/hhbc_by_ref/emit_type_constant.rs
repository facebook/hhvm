// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_hhbc_id::{class, Id};
use hhbc_by_ref_hhbc_string_utils as string_utils;
use hhbc_by_ref_instruction_sequence::{unrecoverable, Result};
use hhbc_by_ref_options::Options;
use hhbc_by_ref_runtime::TypedValue;
use naming_special_names_rust::classes;
use oxidized::{
    aast, aast_defs,
    aast_defs::{Hint, NastShapeInfo, ShapeFieldInfo},
    ast_defs,
    ast_defs::ShapeFieldName,
};
use std::collections::BTreeMap;

fn get_kind_num(tparams: &[&str], mut p: &str) -> i64 {
    if tparams.contains(&p) {
        p = "$$internal$$typevar";
    };
    (match p.to_lowercase().as_str() {
        "hh\\void" => 0,
        "hh\\int" => 1,
        "hh\\bool" => 2,
        "hh\\float" => 3,
        "hh\\string" => 4,
        "hh\\resource" => 5,
        "hh\\num" => 6,
        "hh\\noreturn" => 8,
        "hh\\arraykey" => 7,
        "hh\\mixed" => 9,
        "tuple" => 10,
        "$$internal$$fun" => 11,
        "$$internal$$typevar" | "_" => 13, // corresponds to user OF_GENERIC
        "shape" => 14,
        "class" => 15,
        "interface" => 16,
        "trait" => 17,
        "enum" => 18,
        "hh\\dict" => 19,
        "hh\\vec" => 20,
        "hh\\keyset" => 21,
        "hh\\vec_or_dict" => 22,
        "hh\\nonnull" => 23,
        "hh\\darray" => 24,
        "hh\\varray" => 25,
        "hh\\varray_or_darray" => 26,
        "hh\\anyarray" => 27,
        "hh\\null" => 28,
        "hh\\nothing" => 29,
        "hh\\dynamic" => 30,
        "unresolved" => 101,
        "$$internal$$typeaccess" => 102,
        "$$internal$$reifiedtype" => 104,
        _ => {
            if p.len() > 4 && p.starts_with("xhp_") {
                103
            } else {
                101
            }
        }
    }) as i64
}

fn is_prim(s: &str) -> bool {
    match s {
        "HH\\void" | "HH\\int" | "HH\\bool" | "HH\\float" | "HH\\string" | "HH\\resource"
        | "HH\\num" | "HH\\noreturn" | "HH\\arraykey" | "HH\\mixed" | "HH\\nonnull"
        | "HH\\null" | "HH\\nothing" | "HH\\dynamic" => true,
        _ => false,
    }
}

fn is_resolved_classname(s: &str) -> bool {
    match s {
        "HH\\darray"
        | "HH\\varray"
        | "HH\\varray_or_darray"
        | "HH\\vec"
        | "HH\\dict"
        | "HH\\keyset"
        | "HH\\vec_or_dict"
        | "HH\\AnyArray" => true,
        _ => false,
    }
}

#[allow(clippy::needless_lifetimes)]
fn shape_field_name<'arena>(alloc: &'arena bumpalo::Bump, sf: &ShapeFieldName) -> (String, bool) {
    use oxidized::ast_defs::{Id, ShapeFieldName::*};
    match sf {
        SFlitInt((_, s)) => (s.to_string(), false),
        SFlitStr((_, s)) => {
            (
                // FIXME: This is not safe--string literals are binary strings.
                // There's no guarantee that they're valid UTF-8.
                unsafe { String::from_utf8_unchecked(s.clone().into()) },
                false,
            )
        }
        SFclassConst(Id(_, cname), (_, s)) => {
            let id = class::Type::from_ast_name(alloc, &cname);
            (format!("{}::{}", id.to_raw_string(), s), true)
        }
    }
}

fn shape_field_to_pair<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    sfi: &ShapeFieldInfo,
) -> std::result::Result<
    (TypedValue<'arena>, TypedValue<'arena>),
    hhbc_by_ref_instruction_sequence::Error,
> {
    let (name, is_class_const) = shape_field_name(alloc, &sfi.name);
    let mut r = bumpalo::vec![in alloc;];
    if is_class_const {
        r.push((
            TypedValue::string("is_cls_cns", alloc),
            TypedValue::Bool(true),
        ));
    };
    if sfi.optional {
        r.push((
            TypedValue::string("optional_shape_field", alloc),
            TypedValue::Bool(true),
        ));
    };
    r.push((
        TypedValue::string("value", alloc),
        hint_to_type_constant(alloc, opts, tparams, targ_map, &sfi.hint, false, false)?,
    ));
    Ok((
        TypedValue::string(name.as_str(), alloc),
        TypedValue::Dict(r.into_bump_slice()),
    ))
}

fn shape_info_to_typed_value<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    si: &NastShapeInfo,
) -> std::result::Result<TypedValue<'arena>, hhbc_by_ref_instruction_sequence::Error> {
    let info = si
        .field_map
        .iter()
        .map(|sfi| shape_field_to_pair(alloc, opts, tparams, targ_map, &sfi))
        .collect::<Result<Vec<_>>>()?;
    Ok(TypedValue::Dict(
        bumpalo::collections::Vec::from_iter_in(info.into_iter(), alloc).into_bump_slice(),
    ))
}

fn shape_allows_unknown_fields<'arena>(
    alloc: &'arena bumpalo::Bump,
    si: &NastShapeInfo,
) -> Option<(TypedValue<'arena>, TypedValue<'arena>)> {
    if si.allows_unknown_fields {
        Some((
            TypedValue::string("allows_unknown_fields", alloc),
            TypedValue::Bool(true),
        ))
    } else {
        None
    }
}

fn type_constant_access_list<'arena>(
    alloc: &'arena bumpalo::Bump,
    sl: &[aast::Sid],
) -> TypedValue<'arena> {
    let sl_ = bumpalo::collections::Vec::from_iter_in(
        sl.iter()
            .map(|ast_defs::Id(_, s)| TypedValue::string(s, alloc)),
        alloc,
    )
    .into_bump_slice();
    TypedValue::Vec(sl_)
}

fn resolve_classname<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    mut s: String,
) -> (Option<(TypedValue<'arena>, TypedValue<'arena>)>, String) {
    let is_tparam = s == "_" || tparams.contains(&s.as_str());
    if !is_tparam {
        s = class::Type::from_ast_name(alloc, s.as_str()).into()
    };
    if is_prim(&s) || is_resolved_classname(&s) {
        (None, s)
    } else {
        let id = if is_tparam { "name" } else { "classname" };
        (
            Some((
                TypedValue::string(id, alloc),
                TypedValue::string(s.as_str(), alloc),
            )),
            s,
        )
    }
}

fn get_generic_types<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hints: &[Hint],
) -> std::result::Result<
    bumpalo::collections::vec::Vec<'arena, (TypedValue<'arena>, TypedValue<'arena>)>,
    hhbc_by_ref_instruction_sequence::Error,
> {
    Ok(if hints.is_empty() {
        bumpalo::vec![in alloc;]
    } else {
        bumpalo::vec![in alloc; (
            TypedValue::string("generic_types", alloc),
            hints_to_type_constant(alloc, opts, tparams, targ_map, hints)?,
        )]
    })
}

fn get_kind<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    s: &str,
) -> bumpalo::collections::vec::Vec<'arena, (TypedValue<'arena>, TypedValue<'arena>)> {
    bumpalo::vec![in alloc; (
        TypedValue::string("kind", alloc),
        TypedValue::Int(get_kind_num(tparams, s)),
    )]
}

#[allow(clippy::needless_lifetimes)]
fn root_to_string<'arena>(alloc: &'arena bumpalo::Bump, s: &str) -> String {
    if s == "this" {
        string_utils::prefix_namespace("HH", s)
    } else {
        class::Type::from_ast_name(alloc, s).into()
    }
}

fn get_typevars<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
) -> bumpalo::collections::Vec<'arena, (TypedValue<'arena>, TypedValue<'arena>)> {
    if tparams.is_empty() {
        bumpalo::vec![in alloc;]
    } else {
        bumpalo::vec![in alloc; (
            TypedValue::string("typevars", alloc),
            TypedValue::string(tparams.join(","), alloc),
        )]
    }
}

fn hint_to_type_constant_list<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hint: &Hint,
) -> std::result::Result<
    bumpalo::collections::vec::Vec<'arena, (TypedValue<'arena>, TypedValue<'arena>)>,
    hhbc_by_ref_instruction_sequence::Error,
> {
    use aast_defs::Hint_::*;
    let Hint(_, h) = hint;
    Ok(match h.as_ref() {
        Happly(s, hints) => {
            let ast_defs::Id(_, name) = s;
            if hints.is_empty() {
                if let Some(id) = targ_map.get(name.as_str()) {
                    let mut r = get_kind(alloc, tparams, "$$internal$$reifiedtype");
                    r.push((TypedValue::string("id", alloc), TypedValue::Int(*id)));
                    return Ok(r);
                }
            }
            let (classname, s_res) = resolve_classname(alloc, tparams, name.to_owned());
            let mut r =
                if s_res.eq_ignore_ascii_case("tuple") || s_res.eq_ignore_ascii_case("shape") {
                    get_kind(alloc, tparams, "unresolved")
                } else {
                    get_kind(alloc, tparams, s_res.as_str())
                };
            if let Some(c) = classname {
                r.push(c);
            }
            if !(name.eq_ignore_ascii_case(classes::CLASS_NAME)
                || name.eq_ignore_ascii_case(classes::TYPE_NAME))
            {
                r.append(&mut get_generic_types(
                    alloc, opts, tparams, targ_map, hints,
                )?);
            };
            r
        }
        Hshape(si) => {
            let mut r = bumpalo::vec![in alloc;];
            if let Some(v) = shape_allows_unknown_fields(alloc, si) {
                r.push(v);
            }
            r.append(&mut get_kind(alloc, tparams, "shape"));
            r.push((
                TypedValue::string("fields", alloc),
                shape_info_to_typed_value(alloc, opts, tparams, targ_map, si)?,
            ));
            r
        }
        Haccess(Hint(_, h), ids) => match h.as_happly() {
            Some((root_id, hs)) if hs.is_empty() => {
                let mut r = get_kind(alloc, tparams, "$$internal$$typeaccess");
                r.push((
                    TypedValue::string("root_name", alloc),
                    TypedValue::string(root_to_string(alloc, &root_id.1).as_str(), alloc),
                ));
                r.push((
                    TypedValue::string("access_list", alloc),
                    type_constant_access_list(alloc, ids),
                ));
                r
            }
            _ => {
                return Err(unrecoverable(
                    "Structure not translated according to ast_to_nast",
                ));
            }
        },
        Hfun(hf) => {
            let mut kind = get_kind(alloc, tparams, "$$internal$$fun");
            let single_hint = |name: &str, h| {
                hint_to_type_constant(alloc, opts, tparams, targ_map, h, false, false)
                    .map(|tc| (bumpalo::vec![in alloc; (TypedValue::string(name, alloc), tc)]))
            };
            let mut return_type = single_hint("return_type", &hf.return_ty)?;
            let mut variadic_type = hf.variadic_ty.as_ref().map_or_else(
                || Ok(bumpalo::vec![in alloc;]),
                |h| single_hint("variadic_type", &h),
            )?;
            let mut param_types = bumpalo::vec![in alloc; (
                TypedValue::string("param_types", alloc),
                hints_to_type_constant(alloc, opts, tparams, targ_map, &hf.param_tys)?,
            )];
            param_types.append(&mut variadic_type);
            return_type.append(&mut param_types);
            kind.append(&mut return_type);
            kind
        }
        Htuple(hints) => {
            let mut kind = get_kind(alloc, tparams, "tuple");
            let mut elem_types = bumpalo::vec![in alloc; (
                TypedValue::string("elem_types", alloc),
                hints_to_type_constant(alloc, opts, tparams, targ_map, hints)?,
            )];
            kind.append(&mut elem_types);
            kind
        }
        Hoption(h) => {
            let mut r = bumpalo::vec![in alloc; (TypedValue::string("nullable", alloc), TypedValue::Bool(true))];
            r.append(&mut hint_to_type_constant_list(
                alloc, opts, tparams, targ_map, h,
            )?);
            r
        }
        Hsoft(h) => {
            let mut r = bumpalo::vec![in alloc; (TypedValue::string("soft", alloc), TypedValue::Bool(true))];
            r.append(&mut hint_to_type_constant_list(
                alloc, opts, tparams, targ_map, h,
            )?);
            r
        }
        Hlike(h) => {
            let mut r = bumpalo::vec![in alloc; (TypedValue::string("like", alloc), TypedValue::Bool(true))];
            r.append(&mut hint_to_type_constant_list(
                alloc, opts, tparams, targ_map, h,
            )?);
            r
        }
        // TODO(coeffects) actually handle emission of context constants
        Hintersection(_) => bumpalo::vec![in alloc; (
            TypedValue::string("kind", alloc),
            TypedValue::Int(get_kind_num(tparams, "HH\\mixed")),
        )],
        _ => return Err(unrecoverable("Hints not available on the original AST")),
    })
}

pub fn hint_to_type_constant<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hint: &Hint,
    is_typedef: bool,
    is_opaque: bool,
) -> std::result::Result<TypedValue<'arena>, hhbc_by_ref_instruction_sequence::Error> {
    let mut tconsts = hint_to_type_constant_list(alloc, opts, tparams, targ_map, &hint)?;
    if is_typedef {
        tconsts.extend_from_slice(get_typevars(alloc, tparams).as_slice());
    };
    if is_opaque {
        tconsts.extend_from_slice(&[(TypedValue::string("opaque", alloc), TypedValue::Bool(true))])
    };
    Ok(TypedValue::Dict(tconsts.into_bump_slice()))
}

fn hints_to_type_constant<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hints: &[Hint],
) -> std::result::Result<TypedValue<'arena>, hhbc_by_ref_instruction_sequence::Error> {
    let hints = hints
        .iter()
        .map(|h| hint_to_type_constant(alloc, opts, tparams, targ_map, h, false, false))
        .collect::<Result<Vec<_>>>();
    hints.map(|hs| {
        TypedValue::Vec(
            bumpalo::collections::vec::Vec::from_iter_in(hs.into_iter(), alloc).into_bump_slice(),
        )
    })
}
