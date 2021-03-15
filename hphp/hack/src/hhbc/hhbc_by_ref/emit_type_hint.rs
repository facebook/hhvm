// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc_by_ref_emit_fatal as emit_fatal;
use hhbc_by_ref_hhas_type::{constraint, Info};
use hhbc_by_ref_hhbc_id::{class, Id as ClassId};
use hhbc_by_ref_hhbc_string_utils as string_utils;
use hhbc_by_ref_instruction_sequence::{Error::Unrecoverable, Result};
use naming_special_names_rust::{classes, typehints};
use oxidized::{
    aast_defs::{Hint, Hint_, Hint_::*, NastShapeInfo, ShapeFieldInfo, Tprim},
    ast_defs::{Id, ShapeFieldName},
    pos::Pos,
};
use std::borrow::Cow;

pub enum Kind {
    Property,
    Return,
    Param,
    TypeDef,
    UpperBound,
}

fn fmt_name_or_prim<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    name: &str,
) -> Cow<'arena, str> {
    if tparams.contains(&name) || string_utils::is_self(&name) || string_utils::is_parent(&name) {
        bumpalo::collections::String::from_str_in(name, alloc)
            .into_bump_str()
            .into()
    } else {
        let id: class::Type<'arena> = class::Type::from_ast_name(alloc, &name);
        if string_utils::is_xhp(&string_utils::strip_ns(&name)) {
            id.to_unmangled_str()
        } else {
            id.to_raw_string().to_owned().into()
        }
    }
}

pub fn prim_to_string(prim: &Tprim) -> Cow<'static, str> {
    use Tprim::*;
    match prim {
        Tnull => typehints::NULL.into(),
        Tvoid => typehints::VOID.into(),
        Tint => typehints::INT.into(),
        Tbool => typehints::BOOL.into(),
        Tfloat => typehints::FLOAT.into(),
        Tstring => typehints::STRING.into(),
        Tresource => typehints::RESOURCE.into(),
        Tnum => typehints::NUM.into(),
        Tarraykey => typehints::ARRAYKEY.into(),
        Tnoreturn => typehints::NORETURN.into(),
    }
}

pub fn fmt_hint<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    strip_tparams: bool,
    hint: &Hint,
) -> std::result::Result<String, hhbc_by_ref_instruction_sequence::Error> {
    let Hint(_, h) = hint;
    Ok(match h.as_ref() {
        Habstr(id, args) | Happly(Id(_, id), args) => {
            let name = fmt_name_or_prim(alloc, tparams, id);
            if args.is_empty() || strip_tparams {
                name.to_string()
            } else {
                format!("{}<{}>", name, fmt_hints(alloc, tparams, args)?)
            }
        }
        Hfun(hf) => {
            // TODO(mqian): Implement for inout parameters
            format!(
                "(function ({}): {})",
                fmt_hints(alloc, tparams, &hf.param_tys)?,
                fmt_hint(alloc, tparams, false, &hf.return_ty)?
            )
        }
        Haccess(Hint(_, hint), accesses) => {
            if let Happly(Id(_, id), _) = hint.as_ref() {
                format!(
                    "{}::{}",
                    fmt_name_or_prim(alloc, tparams, id),
                    accesses
                        .iter()
                        .map(|Id(_, s)| s.as_str())
                        .collect::<Vec<_>>()
                        .join("::")
                )
            } else {
                return Err(Unrecoverable(
                    "ast_to_nast error. Should be Haccess(Happly())".into(),
                ));
            }
        }
        Hoption(hint) => {
            let Hint(_, h) = hint;
            if let Hsoft(t) = h.as_ref() {
                // Follow HHVM order: soft -> option
                // Can we fix this eventually?
                format!("@?{}", fmt_hint(alloc, tparams, false, t)?)
            } else {
                format!("?{}", fmt_hint(alloc, tparams, false, hint)?)
            }
        }
        // No guarantee that this is in the correct order when using map instead of list
        //  TODO: Check whether shape fields need to retain order *)
        Hshape(NastShapeInfo { field_map, .. }) => {
            let fmt_field_name = |name: &ShapeFieldName| match name {
                ShapeFieldName::SFlitInt((_, s)) => s.to_owned(),
                ShapeFieldName::SFlitStr((_, s)) => format!("'{}'", s),
                ShapeFieldName::SFclassConst(Id(_, cid), (_, s)) => {
                    format!("{}::{}", fmt_name_or_prim(alloc, tparams, &cid), s)
                }
            };
            let fmt_field = |field: &ShapeFieldInfo| {
                let prefix = if field.optional { "?" } else { "" };
                Ok(format!(
                    "{}{}=>{}",
                    prefix,
                    fmt_field_name(&field.name),
                    fmt_hint(alloc, tparams, false, &field.hint)?
                ))
            };
            let shape_fields = field_map
                .iter()
                .map(fmt_field)
                .collect::<Result<Vec<_>>>()
                .map(|v| v.join(", "))?;
            string_utils::prefix_namespace("HH", &format!("shape({})", shape_fields).as_str())
        }
        Htuple(hints) => format!("({})", fmt_hints(alloc, tparams, hints)?),
        Hlike(t) => format!("~{}", fmt_hint(alloc, tparams, false, t)?),
        Hsoft(t) => format!("@{}", fmt_hint(alloc, tparams, false, t)?),
        Herr | Hany => {
            return Err(Unrecoverable(
                "This should be an error caught in naming".into(),
            ));
        }
        h => fmt_name_or_prim(alloc, tparams, &hint_to_string(&h)).into(),
    })
}

#[allow(clippy::needless_lifetimes)]
fn hint_to_string<'a>(h: &'a Hint_) -> Cow<'a, str> {
    match h {
        Hprim(p) => prim_to_string(p),
        Hmixed | Hunion(_) | Hintersection(_) => typehints::MIXED.into(),
        Hnonnull => typehints::NONNULL.into(),
        Hdarray(_, _) => typehints::DARRAY.into(),
        Hvarray(_) => typehints::VARRAY.into(),
        HvarrayOrDarray(_, _) => typehints::VARRAY_OR_DARRAY.into(),
        Hthis => typehints::THIS.into(),
        Hdynamic => typehints::DYNAMIC.into(),
        Hnothing => typehints::NOTHING.into(),
        _ => panic!("shouldn't invoke this function"),
    }
}

fn fmt_hints<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    hints: &[Hint],
) -> std::result::Result<String, hhbc_by_ref_instruction_sequence::Error> {
    hints
        .iter()
        .map(|h| fmt_hint(alloc, tparams, false, &h))
        .collect::<Result<Vec<_>>>()
        .map(|v| v.join(", "))
}

fn can_be_nullable(hint: &Hint_) -> bool {
    match hint {
        Haccess(_, _) | Hfun(_) | Hdynamic | Hnonnull | Hmixed => false,
        Hoption(Hint(_, h)) => {
            if let Haccess(_, _) = **h {
                true
            } else {
                can_be_nullable(&**h)
            }
        }
        Happly(Id(_, id), _) => {
            id != "\\HH\\dynamic" && id != "\\HH\\nonnull" && id != "\\HH\\mixed"
        }
        Herr | Hany => panic!("This should be an error caught in naming"),
        _ => true,
    }
}

fn hint_to_type_constraint<'arena>(
    alloc: &'arena bumpalo::Bump,
    kind: &Kind,
    tparams: &[&str],
    skipawaitable: bool,
    h: &Hint,
) -> std::result::Result<constraint::Type, hhbc_by_ref_instruction_sequence::Error> {
    use constraint::{Flags, Type};
    let Hint(pos, hint) = h;
    Ok(match &**hint {
        Hdynamic | Hlike(_) | Hfun(_) | Hunion(_) | Hintersection(_) | Hmixed => Type::default(),
        Haccess(_, _) => Type::make_with_raw_str("", Flags::EXTENDED_HINT | Flags::TYPE_CONSTANT),
        Hshape(_) => Type::make_with_raw_str("HH\\darray", Flags::EXTENDED_HINT),
        Htuple(_) => Type::make_with_raw_str("HH\\varray", Flags::EXTENDED_HINT),
        Hsoft(t) => make_tc_with_flags_if_non_empty_flags(
            alloc,
            kind,
            tparams,
            skipawaitable,
            t,
            Flags::SOFT | Flags::EXTENDED_HINT,
        )?,
        Herr | Hany => {
            return Err(Unrecoverable(
                "This should be an error caught in naming".into(),
            ));
        }
        Hoption(t) => {
            if let Happly(Id(_, s), hs) = &*(t.1) {
                if skipawaitable && is_awaitable(&s) {
                    match &hs[..] {
                        [] => return Ok(Type::default()),
                        [h] => return hint_to_type_constraint(alloc, kind, tparams, false, h),
                        _ => {}
                    }
                }
            } else if let Hsoft(Hint(_, h)) = &*(t.1) {
                if let Happly(Id(_, s), hs) = &**h {
                    if skipawaitable && is_awaitable(&s) {
                        if let [h] = &hs[..] {
                            return make_tc_with_flags_if_non_empty_flags(
                                alloc,
                                kind,
                                tparams,
                                skipawaitable,
                                h,
                                Flags::SOFT | Flags::EXTENDED_HINT,
                            );
                        }
                    }
                }
            }
            make_tc_with_flags_if_non_empty_flags(
                alloc,
                kind,
                tparams,
                skipawaitable,
                t,
                Flags::NULLABLE | Flags::DISPLAY_NULLABLE | Flags::EXTENDED_HINT,
            )?
        }
        Happly(Id(_, s), hs) => {
            match &hs[..] {
                [] if s == "\\HH\\dynamic"
                    || s == "\\HH\\mixed"
                    || (skipawaitable && is_awaitable(s))
                    || (s.eq_ignore_ascii_case("\\hh\\void") && !is_typedef(&kind)) =>
                {
                    return Ok(Type::default());
                }
                [Hint(_, h)] if skipawaitable && is_awaitable(s) => {
                    return match &**h {
                        Hprim(Tprim::Tvoid) => Ok(Type::default()),
                        Happly(Id(_, id), hs) if id == "\\HH\\void" && hs.is_empty() => {
                            Ok(Type::default())
                        }
                        _ => hint_to_type_constraint(alloc, kind, tparams, false, &hs[0]),
                    };
                }
                _ => {}
            };
            type_application_helper(alloc, tparams, kind, pos, s)?
        }
        Habstr(s, _hs) => type_application_helper(alloc, tparams, kind, pos, s)?,
        h => type_application_helper(alloc, tparams, kind, pos, &hint_to_string(h))?,
    })
}

fn is_awaitable(s: &str) -> bool {
    s == classes::AWAITABLE
}

fn is_typedef(kind: &Kind) -> bool {
    if let Kind::TypeDef = kind {
        true
    } else {
        false
    }
}

fn make_tc_with_flags_if_non_empty_flags<'arena>(
    alloc: &'arena bumpalo::Bump,
    kind: &Kind,
    tparams: &[&str],
    skipawaitable: bool,
    hint: &Hint,
    flags: constraint::Flags,
) -> std::result::Result<constraint::Type, hhbc_by_ref_instruction_sequence::Error> {
    let tc = hint_to_type_constraint(alloc, kind, tparams, skipawaitable, hint)?;
    Ok(match (&tc.name, &tc.flags.bits()) {
        (None, 0) => tc,
        _ => constraint::Type::make(tc.name, flags | tc.flags),
    })
}

// Used for nodes that do type application (i.e., Happly and Habstr)
fn type_application_helper<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    kind: &Kind,
    pos: &Pos,
    name: &str,
) -> std::result::Result<constraint::Type, hhbc_by_ref_instruction_sequence::Error> {
    use constraint::{Flags, Type};
    if tparams.contains(&name) {
        let tc_name = match kind {
            Kind::Param | Kind::Return | Kind::Property => name,
            _ => "",
        };
        Ok(Type::make(
            Some(tc_name.into()),
            Flags::EXTENDED_HINT | Flags::TYPE_VAR,
        ))
    } else if string_utils::is_self(&name) || string_utils::is_parent(name) {
        if is_typedef(&kind) {
            Err(emit_fatal::raise_fatal_runtime(
                pos,
                format!("Cannot access {} when no class scope is active", name),
            ))
        } else {
            Ok(Type::make(Some(name.to_owned()), Flags::empty()))
        }
    } else {
        let name: String = class::Type::from_ast_name(alloc, name).into();
        Ok(Type::make(Some(name), Flags::empty()))
    }
}

fn add_nullable(nullable: bool, flags: constraint::Flags) -> constraint::Flags {
    if nullable {
        constraint::Flags::NULLABLE | constraint::Flags::DISPLAY_NULLABLE | flags
    } else {
        flags
    }
}

fn try_add_nullable(nullable: bool, hint: &Hint, flags: constraint::Flags) -> constraint::Flags {
    let Hint(_, h) = hint;
    add_nullable(nullable && can_be_nullable(&**h), flags)
}

fn make_type_info<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    h: &Hint,
    tc_name: Option<String>,
    tc_flags: constraint::Flags,
) -> std::result::Result<Info, hhbc_by_ref_instruction_sequence::Error> {
    let type_info_user_type = Some(fmt_hint(alloc, tparams, false, h)?);
    let type_info_type_constraint = constraint::Type::make(tc_name, tc_flags);
    Ok(Info::make(type_info_user_type, type_info_type_constraint))
}

fn param_hint_to_type_info<'arena>(
    alloc: &'arena bumpalo::Bump,
    kind: &Kind,
    skipawaitable: bool,
    nullable: bool,
    tparams: &[&str],
    hint: &Hint,
) -> std::result::Result<Info, hhbc_by_ref_instruction_sequence::Error> {
    let Hint(_, h) = hint;
    let is_simple_hint = match h.as_ref() {
        Hsoft(_)
        | Hoption(_)
        | Haccess(_, _)
        | Hfun(_)
        | Hdynamic
        | Hnonnull
        | Hmixed
        | Hdarray(_, _) => false,
        Happly(Id(_, s), hs) => {
            if !hs.is_empty() {
                false
            } else {
                if s == "\\HH\\dynamic" || s == "\\HH\\nonnull" || s == "\\HH\\mixed" {
                    false
                } else {
                    !tparams.contains(&s.as_str())
                }
            }
        }
        Habstr(s, hs) => {
            if !hs.is_empty() {
                false
            } else {
                !tparams.contains(&s.as_str())
            }
        }
        Herr | Hany => {
            return Err(Unrecoverable(
                "Expected error on Tany in naming: param_hint_to_type_info".into(),
            ));
        }
        _ => true,
    };
    let tc = hint_to_type_constraint(alloc, kind, tparams, skipawaitable, hint)?;
    make_type_info(
        alloc,
        tparams,
        hint,
        tc.name,
        try_add_nullable(
            nullable,
            hint,
            if is_simple_hint {
                constraint::Flags::empty()
            } else {
                tc.flags
            },
        ),
    )
}

pub fn hint_to_type_info<'arena>(
    alloc: &'arena bumpalo::Bump,
    kind: &Kind,
    skipawaitable: bool,
    nullable: bool,
    tparams: &[&str],
    hint: &Hint,
) -> std::result::Result<Info, hhbc_by_ref_instruction_sequence::Error> {
    if let Kind::Param = kind {
        return param_hint_to_type_info(alloc, kind, skipawaitable, nullable, tparams, hint);
    };
    let tc = hint_to_type_constraint(alloc, kind, tparams, skipawaitable, hint)?;
    let flags = match kind {
        Kind::Return | Kind::Property if tc.name.is_some() => {
            constraint::Flags::EXTENDED_HINT | tc.flags
        }
        Kind::UpperBound => constraint::Flags::UPPERBOUND | tc.flags,
        _ => tc.flags,
    };
    make_type_info(
        alloc,
        tparams,
        hint,
        tc.name,
        if is_typedef(kind) {
            add_nullable(nullable, flags)
        } else {
            try_add_nullable(nullable, hint, flags)
        },
    )
}

pub fn hint_to_class<'arena>(alloc: &'arena bumpalo::Bump, hint: &Hint) -> class::Type<'arena> {
    let Hint(_, h) = hint;
    if let Happly(Id(_, name), _) = &**h {
        class::Type::from_ast_name(alloc, &name)
    } else {
        class::from_raw_string(alloc, "__type_is_not_class__")
    }
}

pub fn emit_type_constraint_for_native_function(
    tparams: &[&str],
    ret_opt: Option<&Hint>,
    ti: Info,
) -> Info {
    use constraint::Flags;
    let (name, flags) = match (&ti.user_type, ret_opt) {
        (_, None) | (None, _) => (Some(String::from("HH\\void")), Flags::EXTENDED_HINT),
        (Some(t), _) => {
            if t == "HH\\mixed" || t == "callable" {
                (None, Flags::default())
            } else {
                let Hint(_, h) = ret_opt.as_ref().unwrap();
                let name = Some(
                    string_utils::strip_type_list(
                        t.trim_start_matches('?')
                            .trim_start_matches('@')
                            .trim_start_matches('?'),
                    )
                    .to_string(),
                );
                (name, get_flags(tparams, Flags::EXTENDED_HINT, &h))
            }
        }
    };
    let tc = constraint::Type::make(name, flags);
    Info::make(ti.user_type, tc)
}

fn get_flags(tparams: &[&str], flags: constraint::Flags, hint: &Hint_) -> constraint::Flags {
    use constraint::Flags;
    match hint {
        Hoption(x) => {
            let Hint(_, h) = x;
            Flags::NULLABLE | Flags::DISPLAY_NULLABLE | get_flags(tparams, flags, &**h)
        }
        Hsoft(x) => {
            let Hint(_, h) = x;
            Flags::SOFT | get_flags(tparams, flags, &**h)
        }
        Haccess(_, _) => Flags::TYPE_CONSTANT | flags,
        Happly(Id(_, s), _) if tparams.contains(&s.as_str()) => Flags::TYPE_VAR | flags,
        _ => flags,
    }
}
