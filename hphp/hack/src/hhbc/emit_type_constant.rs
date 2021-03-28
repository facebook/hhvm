// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_id_rust::{class, Id};
use hhbc_string_utils_rust as string_utils;
use instruction_sequence::{unrecoverable, Result};
use naming_special_names_rust::classes;
use options::Options;
use oxidized::{
    aast, aast_defs,
    aast_defs::{Hint, NastShapeInfo, ShapeFieldInfo},
    ast_defs,
    ast_defs::ShapeFieldName,
};
use runtime::TypedValue;
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

fn shape_field_name(sf: &ShapeFieldName) -> (String, bool) {
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
            let id = class::Type::from_ast_name(&cname);
            (format!("{}::{}", id.to_raw_string(), s), true)
        }
    }
}

fn shape_field_to_pair(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    sfi: &ShapeFieldInfo,
) -> Result<(TypedValue, TypedValue)> {
    let (name, is_class_const) = shape_field_name(&sfi.name);
    let mut r = vec![];
    if is_class_const {
        r.push((TypedValue::string("is_cls_cns"), TypedValue::Bool(true)));
    };
    if sfi.optional {
        r.push((
            TypedValue::string("optional_shape_field"),
            TypedValue::Bool(true),
        ));
    };
    r.push((
        TypedValue::string("value"),
        hint_to_type_constant(opts, tparams, targ_map, &sfi.hint, false, false)?,
    ));
    Ok((TypedValue::String(name), TypedValue::Dict(r)))
}

fn shape_info_to_typed_value(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    si: &NastShapeInfo,
) -> Result<TypedValue> {
    let info = si
        .field_map
        .iter()
        .map(|sfi| shape_field_to_pair(opts, tparams, targ_map, &sfi))
        .collect::<Result<_>>()?;
    Ok(TypedValue::Dict(info))
}

fn shape_allows_unknown_fields(si: &NastShapeInfo) -> Option<(TypedValue, TypedValue)> {
    if si.allows_unknown_fields {
        Some((
            TypedValue::string("allows_unknown_fields"),
            TypedValue::Bool(true),
        ))
    } else {
        None
    }
}

fn type_constant_access_list(sl: &[aast::Sid]) -> TypedValue {
    TypedValue::Vec(
        sl.iter()
            .map(|ast_defs::Id(_, s)| TypedValue::string(s))
            .collect(),
    )
}

fn resolve_classname(
    tparams: &[&str],
    mut s: String,
) -> (Option<(TypedValue, TypedValue)>, String) {
    let is_tparam = s == "_" || tparams.contains(&s.as_str());
    if !is_tparam {
        s = class::Type::from_ast_name(s.as_str()).into()
    };
    if is_prim(&s) || is_resolved_classname(&s) {
        (None, s)
    } else {
        let id = if is_tparam { "name" } else { "classname" };
        (
            Some((TypedValue::String(id.into()), TypedValue::String(s.clone()))),
            s,
        )
    }
}

fn get_generic_types(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hints: &Vec<Hint>,
) -> Result<Vec<(TypedValue, TypedValue)>> {
    Ok(if hints.is_empty() {
        vec![]
    } else {
        vec![(
            TypedValue::String("generic_types".into()),
            hints_to_type_constant(opts, tparams, targ_map, hints)?,
        )]
    })
}

fn get_kind(tparams: &[&str], s: &str) -> Vec<(TypedValue, TypedValue)> {
    vec![(
        TypedValue::String("kind".into()),
        TypedValue::Int(get_kind_num(tparams, s)),
    )]
}

fn root_to_string(s: &str) -> String {
    if s == "this" {
        string_utils::prefix_namespace("HH", s)
    } else {
        class::Type::from_ast_name(s).into()
    }
}

fn get_typevars(tparams: &[&str]) -> Vec<(TypedValue, TypedValue)> {
    if tparams.is_empty() {
        vec![]
    } else {
        vec![(
            TypedValue::String("typevars".into()),
            TypedValue::String(tparams.join(",")),
        )]
    }
}

fn hint_to_type_constant_list(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hint: &Hint,
) -> Result<Vec<(TypedValue, TypedValue)>> {
    use aast_defs::Hint_::*;
    let Hint(_, h) = hint;
    Ok(match h.as_ref() {
        Happly(s, hints) => {
            let ast_defs::Id(_, name) = s;
            if hints.is_empty() {
                if let Some(id) = targ_map.get(name.as_str()) {
                    let mut r = get_kind(tparams, "$$internal$$reifiedtype");
                    r.push((TypedValue::String("id".into()), TypedValue::Int(*id)));
                    return Ok(r);
                }
            }
            let (classname, s_res) = resolve_classname(tparams, name.to_owned());
            let mut r =
                if s_res.eq_ignore_ascii_case("tuple") || s_res.eq_ignore_ascii_case("shape") {
                    get_kind(tparams, "unresolved")
                } else {
                    get_kind(tparams, s_res.as_str())
                };
            if let Some(c) = classname {
                r.push(c);
            }
            if !(name.eq_ignore_ascii_case(classes::CLASS_NAME)
                || name.eq_ignore_ascii_case(classes::TYPE_NAME))
            {
                r.append(&mut get_generic_types(opts, tparams, targ_map, hints)?);
            };
            r
        }
        Hshape(si) => {
            let mut r = vec![];
            if let Some(v) = shape_allows_unknown_fields(si) {
                r.push(v);
            }
            r.append(&mut get_kind(tparams, "shape"));
            r.push((
                TypedValue::string("fields"),
                shape_info_to_typed_value(opts, tparams, targ_map, si)?,
            ));
            r
        }
        Haccess(Hint(_, h), ids) => match h.as_happly() {
            Some((root_id, hs)) if hs.is_empty() => {
                let mut r = get_kind(tparams, "$$internal$$typeaccess");
                r.push((
                    TypedValue::string("root_name"),
                    TypedValue::string(root_to_string(&root_id.1)),
                ));
                r.push((
                    TypedValue::string("access_list"),
                    type_constant_access_list(ids),
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
            let kind = get_kind(tparams, "$$internal$$fun");
            let single_hint = |name: &str, h| {
                hint_to_type_constant(opts, tparams, targ_map, h, false, false)
                    .map(|tc| (vec![(TypedValue::String(name.into()), tc)]))
            };
            let return_type = single_hint("return_type", &hf.return_ty)?;
            let variadic_type = hf
                .variadic_ty
                .as_ref()
                .map_or_else(|| Ok(vec![]), |h| single_hint("variadic_type", &h))?;
            let param_types = vec![(
                TypedValue::String("param_types".into()),
                hints_to_type_constant(opts, tparams, targ_map, &hf.param_tys)?,
            )];
            [kind, return_type, param_types, variadic_type].concat()
        }
        Htuple(hints) => {
            let kind = get_kind(tparams, "tuple");
            let elem_types = vec![(
                TypedValue::String("elem_types".into()),
                hints_to_type_constant(opts, tparams, targ_map, hints)?,
            )];
            [kind, elem_types].concat()
        }
        Hoption(h) => {
            let mut r = vec![(TypedValue::string("nullable"), TypedValue::Bool(true))];
            r.append(&mut hint_to_type_constant_list(opts, tparams, targ_map, h)?);
            r
        }
        Hsoft(h) => [
            vec![(TypedValue::String("soft".into()), TypedValue::Bool(true))],
            hint_to_type_constant_list(opts, tparams, targ_map, h)?,
        ]
        .concat(),
        Hlike(h) => [
            vec![(TypedValue::String("like".into()), TypedValue::Bool(true))],
            hint_to_type_constant_list(opts, tparams, targ_map, h)?,
        ]
        .concat(),
        // TODO(coeffects) actually handle emission of context constants
        Hintersection(_) => vec![(
            TypedValue::String("kind".into()),
            TypedValue::Int(get_kind_num(tparams, "HH\\mixed".into())),
        )],
        _ => return Err(unrecoverable("Hints not available on the original AST")),
    })
}

pub fn hint_to_type_constant(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hint: &Hint,
    is_typedef: bool,
    is_opaque: bool,
) -> Result<TypedValue> {
    let mut tconsts = hint_to_type_constant_list(opts, tparams, targ_map, &hint)?;
    if is_typedef {
        tconsts.extend_from_slice(get_typevars(tparams).as_slice());
    };
    if is_opaque {
        tconsts.extend_from_slice(&[(TypedValue::String("opaque".into()), TypedValue::Bool(true))])
    };
    Ok(TypedValue::Dict(tconsts))
}

fn hints_to_type_constant(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hints: &Vec<Hint>,
) -> Result<TypedValue> {
    let hints = hints
        .into_iter()
        .map(|h| hint_to_type_constant(opts, tparams, targ_map, h, false, false))
        .collect::<Result<Vec<_>>>();
    hints.map(TypedValue::Vec)
}
