// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use error::Error;
use error::Result;
use ffi::Pair;
use hhbc::TypedValue;
use hhbc_string_utils as string_utils;
use naming_special_names_rust::classes;
use options::Options;
use oxidized::aast;
use oxidized::aast_defs;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::NastShapeInfo;
use oxidized::aast_defs::ShapeFieldInfo;
use oxidized::ast_defs;
use oxidized::ast_defs::ShapeFieldName;

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

fn shape_field_name<'arena>(alloc: &'arena bumpalo::Bump, sf: &ShapeFieldName) -> (String, bool) {
    use oxidized::ast_defs::Id;
    match sf {
        ShapeFieldName::SFlitInt((_, s)) => (s.to_string(), false),
        ShapeFieldName::SFlitStr((_, s)) => {
            (
                // FIXME: This is not safe--string literals are binary strings.
                // There's no guarantee that they're valid UTF-8.
                unsafe { String::from_utf8_unchecked(s.clone().into()) },
                false,
            )
        }
        ShapeFieldName::SFclassConst(Id(_, cname), (_, s)) => {
            let id = hhbc::ClassName::from_ast_name_and_mangle(alloc, cname);
            (format!("{}::{}", id.unsafe_as_str(), s), true)
        }
    }
}

fn shape_field_to_pair<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    sfi: &ShapeFieldInfo,
) -> Result<Pair<TypedValue<'arena>, TypedValue<'arena>>> {
    let (name, is_class_const) = shape_field_name(alloc, &sfi.name);
    let mut r = bumpalo::vec![in alloc;];
    if is_class_const {
        r.push(Pair(
            TypedValue::string("is_cls_cns"),
            TypedValue::Bool(true),
        ));
    };
    if sfi.optional {
        r.push(Pair(
            TypedValue::string("optional_shape_field"),
            TypedValue::Bool(true),
        ));
    };
    r.push(Pair(
        TypedValue::string("value"),
        hint_to_type_constant(alloc, opts, tparams, targ_map, &sfi.hint, false, false)?,
    ));
    Ok(Pair(
        TypedValue::alloc_string(name, alloc),
        TypedValue::dict(r.into_bump_slice()),
    ))
}

fn shape_info_to_typed_value<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    si: &NastShapeInfo,
) -> Result<TypedValue<'arena>> {
    let info = si
        .field_map
        .iter()
        .map(|sfi| shape_field_to_pair(alloc, opts, tparams, targ_map, sfi))
        .collect::<Result<Vec<_>>>()?;
    Ok(TypedValue::dict(
        alloc.alloc_slice_fill_iter(info.into_iter()),
    ))
}

fn shape_allows_unknown_fields<'arena>(
    si: &NastShapeInfo,
) -> Option<Pair<TypedValue<'arena>, TypedValue<'arena>>> {
    if si.allows_unknown_fields {
        Some(Pair(
            TypedValue::string("allows_unknown_fields"),
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
    let sl_ = alloc.alloc_slice_fill_iter(
        sl.iter()
            .map(|ast_defs::Id(_, s)| TypedValue::alloc_string(s, alloc)),
    );
    TypedValue::vec(sl_)
}

fn resolve_classname<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    mut s: String,
) -> (Option<Pair<TypedValue<'arena>, TypedValue<'arena>>>, String) {
    let is_tparam = s == "_" || tparams.contains(&s.as_str());
    if !is_tparam {
        s = hhbc::ClassName::from_ast_name_and_mangle(alloc, s.as_str()).unsafe_into_string()
    };
    if is_prim(&s) || is_resolved_classname(&s) {
        (None, s)
    } else {
        let id = if is_tparam { "name" } else { "classname" };
        (
            Some(Pair(
                TypedValue::string(id),
                TypedValue::alloc_string(s.as_str(), alloc),
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
) -> Result<bumpalo::collections::vec::Vec<'arena, Pair<TypedValue<'arena>, TypedValue<'arena>>>> {
    Ok(if hints.is_empty() {
        bumpalo::vec![in alloc;]
    } else {
        bumpalo::vec![in alloc; Pair(
            TypedValue::string("generic_types"),
            hints_to_type_constant(alloc, opts, tparams, targ_map, hints)?,
        )]
    })
}

fn get_kind<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    s: &str,
) -> bumpalo::collections::vec::Vec<'arena, Pair<TypedValue<'arena>, TypedValue<'arena>>> {
    bumpalo::vec![in alloc; Pair(
        TypedValue::string("kind"),
        TypedValue::Int(get_kind_num(tparams, s)),
    )]
}

fn root_to_string<'arena>(alloc: &'arena bumpalo::Bump, s: &str) -> String {
    if s == "this" {
        string_utils::prefix_namespace("HH", s)
    } else {
        hhbc::ClassName::from_ast_name_and_mangle(alloc, s).unsafe_into_string()
    }
}

fn get_typevars<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
) -> bumpalo::collections::Vec<'arena, Pair<TypedValue<'arena>, TypedValue<'arena>>> {
    if tparams.is_empty() {
        bumpalo::vec![in alloc;]
    } else {
        bumpalo::vec![in alloc; (
            TypedValue::string("typevars"),
            TypedValue::alloc_string(tparams.join(","), alloc),
        ).into()]
    }
}

fn hint_to_type_constant_list<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    Hint(_, hint): &Hint,
) -> Result<bumpalo::collections::Vec<'arena, Pair<TypedValue<'arena>, TypedValue<'arena>>>> {
    use aast_defs::Hint_;
    Ok(match hint.as_ref() {
        Hint_::Happly(s, hints) => {
            let ast_defs::Id(_, name) = s;
            if hints.is_empty() {
                if let Some(id) = targ_map.get(name.as_str()) {
                    let mut r = get_kind(alloc, tparams, "$$internal$$reifiedtype");
                    r.push(Pair(TypedValue::string("id"), TypedValue::Int(*id)));
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
        Hint_::Hshape(si) => {
            let mut r = bumpalo::vec![in alloc;];
            if let Some(v) = shape_allows_unknown_fields(si) {
                r.push(v);
            }
            r.append(&mut get_kind(alloc, tparams, "shape"));
            r.push(Pair(
                TypedValue::string("fields"),
                shape_info_to_typed_value(alloc, opts, tparams, targ_map, si)?,
            ));
            r
        }
        Hint_::Haccess(Hint(_, h), ids) => match h.as_happly() {
            Some((root_id, hs)) if hs.is_empty() => {
                let mut r = get_kind(alloc, tparams, "$$internal$$typeaccess");
                r.push(Pair(
                    TypedValue::string("root_name"),
                    TypedValue::alloc_string(root_to_string(alloc, &root_id.1).as_str(), alloc),
                ));
                r.push(Pair(
                    TypedValue::string("access_list"),
                    type_constant_access_list(alloc, ids),
                ));
                r
            }
            _ => {
                return Err(Error::unrecoverable(
                    "Structure not translated according to ast_to_nast",
                ));
            }
        },
        Hint_::Hfun(hf) => {
            let mut kind = get_kind(alloc, tparams, "$$internal$$fun");
            let single_hint = |name: &str, h| {
                hint_to_type_constant(alloc, opts, tparams, targ_map, h, false, false).map(
                    |tc| (bumpalo::vec![in alloc; Pair(TypedValue::alloc_string(name, alloc), tc)]),
                )
            };
            let mut return_type = single_hint("return_type", &hf.return_ty)?;
            let mut variadic_type = hf.variadic_ty.as_ref().map_or_else(
                || Ok(bumpalo::vec![in alloc;]),
                |h| single_hint("variadic_type", h),
            )?;
            let mut param_types = bumpalo::vec![in alloc; Pair(
                TypedValue::string("param_types"),
                hints_to_type_constant(alloc, opts, tparams, targ_map, &hf.param_tys)?,
            )];
            param_types.append(&mut variadic_type);
            return_type.append(&mut param_types);
            kind.append(&mut return_type);
            kind
        }
        Hint_::Htuple(hints) => {
            let mut kind = get_kind(alloc, tparams, "tuple");
            let mut elem_types = bumpalo::vec![in alloc; Pair(
                TypedValue::string("elem_types"),
                hints_to_type_constant(alloc, opts, tparams, targ_map, hints)?,
            )];
            kind.append(&mut elem_types);
            kind
        }
        Hint_::Hoption(h) => {
            let mut r = bumpalo::vec![in alloc; Pair(TypedValue::string("nullable"), TypedValue::Bool(true))];
            r.append(&mut hint_to_type_constant_list(
                alloc, opts, tparams, targ_map, h,
            )?);
            r
        }
        Hint_::Hsoft(h) => {
            let mut r =
                bumpalo::vec![in alloc; Pair(TypedValue::string("soft"), TypedValue::Bool(true))];
            r.append(&mut hint_to_type_constant_list(
                alloc, opts, tparams, targ_map, h,
            )?);
            r
        }
        Hint_::Hlike(h) => {
            let mut r =
                bumpalo::vec![in alloc; Pair(TypedValue::string("like"), TypedValue::Bool(true))];
            r.append(&mut hint_to_type_constant_list(
                alloc, opts, tparams, targ_map, h,
            )?);
            r
        }
        // TODO(coeffects) actually handle emission of context constants
        Hint_::Hintersection(_) => bumpalo::vec![in alloc; (
            TypedValue::string("kind"),
            TypedValue::Int(get_kind_num(tparams, "HH\\mixed")),
        ).into()],
        _ => {
            return Err(Error::unrecoverable(
                "Hints not available on the original AST",
            ));
        }
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
) -> Result<TypedValue<'arena>> {
    let mut tconsts = hint_to_type_constant_list(alloc, opts, tparams, targ_map, hint)?;
    if is_typedef {
        tconsts.append(&mut get_typevars(alloc, tparams));
    };
    if is_opaque {
        tconsts.push((TypedValue::string("opaque"), TypedValue::Bool(true)).into())
    };
    Ok(TypedValue::dict(tconsts.into_bump_slice()))
}

fn hints_to_type_constant<'arena>(
    alloc: &'arena bumpalo::Bump,
    opts: &Options,
    tparams: &[&str],
    targ_map: &BTreeMap<&str, i64>,
    hints: &[Hint],
) -> Result<TypedValue<'arena>> {
    hints
        .iter()
        .map(|h| hint_to_type_constant(alloc, opts, tparams, targ_map, h, false, false))
        .collect::<Result<Vec<_>>>()
        .map(|hs| TypedValue::vec(alloc.alloc_slice_fill_iter(hs.into_iter())))
}
