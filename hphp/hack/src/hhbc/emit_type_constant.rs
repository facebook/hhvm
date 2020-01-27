// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc_id_rust::{class, Id};
use hhbc_string_utils_rust as string_utils;
use instruction_sequence_rust::{Error::Unrecoverable, Result};
use naming_special_names_rust::classes;
use options::{HhvmFlags, Options};
use oxidized::{
    aast, aast_defs,
    aast_defs::{Hint, NastShapeInfo, ShapeFieldInfo},
    ast_defs,
    ast_defs::ShapeFieldName,
};
use runtime::TypedValue;
use std::collections::BTreeMap;

fn hack_arr_dv_arrs(opts: &Options) -> bool {
    opts.hhvm.flags.contains(HhvmFlags::HACK_ARR_DV_ARRS)
}

fn vec_or_varray(opts: &Options, l: Vec<TypedValue>) -> TypedValue {
    if hack_arr_dv_arrs(opts) {
        TypedValue::Vec((l, None))
    } else {
        TypedValue::VArray(l)
    }
}

fn dict_or_darray(opts: &Options, kv: Vec<(TypedValue, TypedValue)>) -> TypedValue {
    if hack_arr_dv_arrs(opts) {
        TypedValue::Dict((kv, None))
    } else {
        TypedValue::DArray(kv)
    }
}

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
        "array" => 12,
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
        "hh\\arraylike" => 27,
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
        "array"
        | "HH\\darray"
        | "HH\\varray"
        | "HH\\varray_or_darray"
        | "HH\\vec"
        | "HH\\dict"
        | "HH\\keyset"
        | "HH\\vec_or_dict"
        | "HH\\arraylike" => true,
        _ => false,
    }
}

fn shape_field_name(sf: ShapeFieldName) -> (String, bool) {
    use oxidized::ast_defs::{Id, ShapeFieldName::*};
    match sf {
        SFlitInt((_, s)) | SFlitStr((_, s)) => (s, false),
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
    sfi: ShapeFieldInfo,
) -> Result<(TypedValue, TypedValue)> {
    let (name, is_class_const) = shape_field_name(sfi.name);
    let mut inner_value = vec![(
        TypedValue::String("value".into()),
        hint_to_type_constant(opts, tparams, targ_map, sfi.hint)?,
    )];
    if is_class_const {
        inner_value.extend_from_slice(&[(
            TypedValue::String("is_cls_cns".into()),
            TypedValue::Bool(true),
        )]);
    };
    if sfi.optional {
        inner_value.extend_from_slice(&[(
            TypedValue::String("optional_shape_field".into()),
            TypedValue::Bool(true),
        )]);
    };
    Ok((TypedValue::String(name), dict_or_darray(opts, inner_value)))
}

fn shape_info_to_typed_value(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    si: NastShapeInfo,
) -> Result<TypedValue> {
    let info = si
        .field_map
        .into_iter()
        .map(|sfi| shape_field_to_pair(opts, tparams, targ_map, sfi))
        .collect::<Result<_>>()?;
    Ok(dict_or_darray(opts, info))
}

fn type_constant_access_list(opts: &Options, sl: Vec<aast::Sid>) -> TypedValue {
    vec_or_varray(
        opts,
        sl.into_iter()
            .map(|ast_defs::Id(_, s)| TypedValue::String(s))
            .collect(),
    )
}

fn resolve_classname(
    tparams: &[&str],
    mut s: String,
) -> (Vec<(TypedValue, TypedValue)>, Vec<(TypedValue, TypedValue)>) {
    let is_tparam = s == "_" || tparams.contains(&s.as_str());
    if !is_tparam {
        s = class::Type::from_ast_name(s.as_str()).into()
    };
    let mut classname = vec![];
    let kind = get_kind(tparams, &s.as_str());
    if !is_prim(&s) && !is_resolved_classname(&s) {
        let id = if is_tparam { "name" } else { "classname" }.to_string();
        classname.push((TypedValue::String(id), TypedValue::String(s)));
    };
    (classname, kind)
}

fn get_generic_types(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hints: Vec<Hint>,
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

fn root_to_string(s: String) -> String {
    if s == "this" {
        string_utils::prefix_namespace("HH", s.as_str())
    } else {
        class::Type::from_ast_name(s.as_str()).into()
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
    hint: Hint,
) -> Result<Vec<(TypedValue, TypedValue)>> {
    use aast_defs::Hint_::*;
    let Hint(_, h) = hint;
    Ok(match *h {
        Happly(s, hints) => {
            let ast_defs::Id(_, name) = s;
            if hints.is_empty() {
                let id = *targ_map
                    .get(&name.as_str())
                    .expect("Hints not available on the original AST");
                [
                    vec![(TypedValue::String("id".into()), TypedValue::Int(id))],
                    get_kind(tparams, "$$internal$$reifiedtype"),
                ]
                .concat()
            } else {
                let generic_types = if name.eq_ignore_ascii_case(classes::CLASS_NAME)
                    || name.eq_ignore_ascii_case(classes::TYPE_NAME)
                {
                    vec![]
                } else {
                    get_generic_types(opts, tparams, targ_map, hints)?
                };
                let (classname, kind) = resolve_classname(tparams, name);
                [generic_types, classname, kind].concat()
            }
        }
        Hshape(si) => [
            vec![(
                TypedValue::String("fields".into()),
                shape_info_to_typed_value(opts, tparams, targ_map, si)?,
            )],
            get_kind(tparams, "shape"),
            vec![(
                TypedValue::String("is_cls_cns".into()),
                TypedValue::Bool(true),
            )],
        ]
        .concat(),
        Haccess(_, _) => {
            return Err(Unrecoverable(
                "Structure not translated according to ast_to_nast".into(),
            ))
        }
        Hfun(hf) => {
            if hf.is_coroutine {
                return Err(Unrecoverable(
                    "Codegen for coroutine functions is not supported".into(),
                ));
            } else {
                let kind = get_kind(tparams, "$$internal$$fun");
                let single_hint = |name: &str, h| {
                    Ok(vec![(
                        TypedValue::String(name.into()),
                        hint_to_type_constant(opts, tparams, targ_map, h)?,
                    )])
                };
                let return_type = single_hint("return_type", hf.return_ty)?;
                let variadic_type = hf
                    .variadic_ty
                    .map_or_else(|| Ok(vec![]), |h| single_hint("variadic_type", h))?;
                let param_types = vec![(
                    TypedValue::String("param_types".into()),
                    hints_to_type_constant(opts, tparams, targ_map, hf.param_tys)?,
                )];
                [variadic_type, param_types, return_type, kind].concat()
            }
        }
        Htuple(hints) => {
            let kind = get_kind(tparams, "tuple");
            let elem_types = vec![(
                TypedValue::String("elem_types".into()),
                hints_to_type_constant(opts, tparams, targ_map, hints)?,
            )];
            [elem_types, kind].concat()
        }
        Hoption(h) => [
            hint_to_type_constant_list(opts, tparams, targ_map, h)?,
            vec![(
                TypedValue::String("nullable".into()),
                TypedValue::Bool(true),
            )],
        ]
        .concat(),
        Hsoft(h) => [
            hint_to_type_constant_list(opts, tparams, targ_map, h)?,
            vec![(TypedValue::String("soft".into()), TypedValue::Bool(true))],
        ]
        .concat(),
        Hlike(h) => [
            hint_to_type_constant_list(opts, tparams, targ_map, h)?,
            vec![(TypedValue::String("like".into()), TypedValue::Bool(true))],
        ]
        .concat(),
        HpuAccess(_, _) => {
            return Err(Unrecoverable(
                "TODO(T36532263) hint_to_type_constant_list".into(),
            ))
        }
        _ => {
            return Err(Unrecoverable(
                "Hints not available on the original AST".into(),
            ))
        }
    })
}

pub fn hint_to_type_constant(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hint: Hint,
) -> Result<TypedValue> {
    Ok(dict_or_darray(
        opts,
        hint_to_type_constant_list(opts, tparams, targ_map, hint)?,
    ))
}

fn hints_to_type_constant(
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hints: Vec<Hint>,
) -> Result<TypedValue> {
    Ok(vec_or_varray(
        opts,
        hints
            .into_iter()
            .map(|h| hint_to_type_constant(opts, tparams, targ_map, h))
            .collect::<Result<_>>()?,
    ))
}
