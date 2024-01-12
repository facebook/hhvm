// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::borrow::Cow;

use error::Error;
use error::Result;
use ffi::Maybe;
use ffi::Maybe::*;
use ffi::Str;
use hhbc::Constraint;
use hhbc::TypeInfo;
use hhbc_string_utils as string_utils;
use hhvm_types_ffi::ffi::TypeConstraintFlags;
use naming_special_names_rust::classes;
use naming_special_names_rust::typehints;
use oxidized::aast_defs::Hint;
use oxidized::aast_defs::Hint_;
use oxidized::aast_defs::Hint_::*;
use oxidized::aast_defs::NastShapeInfo;
use oxidized::aast_defs::ShapeFieldInfo;
use oxidized::aast_defs::Tprim;
use oxidized::ast_defs::Id;
use oxidized::ast_defs::ShapeFieldName;

#[derive(Eq, PartialEq)]
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
    if tparams.contains(&name) {
        (alloc.alloc_str(name) as &str).into()
    } else {
        let id: hhbc::ClassName<'arena> = hhbc::ClassName::from_ast_name_and_mangle(alloc, name);
        if string_utils::is_xhp(string_utils::strip_ns(name)) {
            id.unsafe_to_unmangled_str()
        } else {
            id.unsafe_as_str().into()
        }
    }
}

pub fn prim_to_string(prim: &Tprim) -> &'static str {
    match prim {
        Tprim::Tnull => typehints::NULL,
        Tprim::Tvoid => typehints::VOID,
        Tprim::Tint => typehints::INT,
        Tprim::Tbool => typehints::BOOL,
        Tprim::Tfloat => typehints::FLOAT,
        Tprim::Tstring => typehints::STRING,
        Tprim::Tresource => typehints::RESOURCE,
        Tprim::Tnum => typehints::NUM,
        Tprim::Tarraykey => typehints::ARRAYKEY,
        Tprim::Tnoreturn => typehints::NORETURN,
    }
}

pub fn fmt_hint<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    strip_tparams: bool,
    hint: &Hint,
) -> Result<String> {
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
        Hwildcard => "_".into(),
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
                return Err(Error::unrecoverable(
                    "ast_to_nast error. Should be Haccess(Happly())",
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
        Hrefinement(hint, _) => {
            // NOTE: refinements are already banned in type structures
            // and in other cases they should be invisible to the HHVM, so unpack hint
            fmt_hint(alloc, tparams, strip_tparams, hint)?
        }
        // No guarantee that this is in the correct order when using map instead of list
        //  TODO: Check whether shape fields need to retain order *)
        Hshape(NastShapeInfo { field_map, .. }) => {
            let fmt_field_name = |name: &ShapeFieldName| match name {
                ShapeFieldName::SFlitInt((_, s)) => s.to_owned(),
                ShapeFieldName::SFlitStr((_, s)) => format!("'{}'", s),
                ShapeFieldName::SFclassConst(Id(_, cid), (_, s)) => {
                    format!("{}::{}", fmt_name_or_prim(alloc, tparams, cid), s)
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
            string_utils::prefix_namespace("HH", &format!("shape({})", shape_fields))
        }
        Htuple(hints) => format!("({})", fmt_hints(alloc, tparams, hints)?),
        Hlike(t) => format!("~{}", fmt_hint(alloc, tparams, false, t)?),
        Hsoft(t) => format!("@{}", fmt_hint(alloc, tparams, false, t)?),
        h => fmt_name_or_prim(alloc, tparams, hint_to_string(h)).into(),
    })
}

fn hint_to_string<'a>(h: &'a Hint_) -> &'a str {
    match h {
        Hprim(p) => prim_to_string(p),
        Hmixed | Hunion(_) | Hintersection(_) => typehints::MIXED,
        Hnonnull => typehints::NONNULL,
        Hthis => typehints::THIS,
        Hdynamic => typehints::DYNAMIC,
        Hnothing => typehints::NOTHING,
        _ => panic!("shouldn't invoke this function"),
    }
}

fn fmt_hints<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    hints: &[Hint],
) -> Result<String> {
    hints
        .iter()
        .map(|h| fmt_hint(alloc, tparams, false, h))
        .collect::<Result<Vec<_>>>()
        .map(|v| v.join(", "))
}

fn can_be_nullable(hint: &Hint_) -> bool {
    match hint {
        Haccess(_, _) | Hfun(_) | Hdynamic | Hnonnull | Hmixed | Hwildcard => false,
        Hoption(Hint(_, h)) => {
            if let Haccess(_, _) = **h {
                true
            } else {
                can_be_nullable(h)
            }
        }
        Happly(Id(_, id), _) => {
            id != "\\HH\\dynamic" && id != "\\HH\\nonnull" && id != "\\HH\\mixed"
        }
        _ => true,
    }
}

fn hint_to_type_constraint<'arena>(
    alloc: &'arena bumpalo::Bump,
    kind: &Kind,
    tparams: &[&str],
    skipawaitable: bool,
    h: &Hint,
) -> Result<Constraint<'arena>> {
    let Hint(_, hint) = h;
    Ok(match &**hint {
        Hdynamic | Hfun(_) | Hunion(_) | Hintersection(_) | Hmixed | Hwildcard => {
            Constraint::default()
        }
        Haccess(_, _) => Constraint::make(
            Just("".into()),
            TypeConstraintFlags::ExtendedHint | TypeConstraintFlags::TypeConstant,
        ),
        Hshape(_) => Constraint::make(Just("HH\\darray".into()), TypeConstraintFlags::ExtendedHint),
        Htuple(_) => Constraint::make(Just("HH\\varray".into()), TypeConstraintFlags::ExtendedHint),
        Hsoft(t) => make_tc_with_flags_if_non_empty_flags(
            alloc,
            kind,
            tparams,
            skipawaitable,
            t,
            TypeConstraintFlags::Soft | TypeConstraintFlags::ExtendedHint,
        )?,
        Hlike(h) => hint_to_type_constraint(alloc, kind, tparams, skipawaitable, h)?,
        Hoption(t) => {
            if let Happly(Id(_, s), hs) = &*(t.1) {
                if skipawaitable && is_awaitable(s) {
                    match &hs[..] {
                        [] => return Ok(Constraint::default()),
                        [h] => return hint_to_type_constraint(alloc, kind, tparams, false, h),
                        _ => {}
                    }
                }
            } else if let Hsoft(Hint(_, h)) = &*(t.1) {
                if let Happly(Id(_, s), hs) = &**h {
                    if skipawaitable && is_awaitable(s) {
                        if let [h] = &hs[..] {
                            return make_tc_with_flags_if_non_empty_flags(
                                alloc,
                                kind,
                                tparams,
                                skipawaitable,
                                h,
                                TypeConstraintFlags::Soft | TypeConstraintFlags::ExtendedHint,
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
                TypeConstraintFlags::Nullable
                    | TypeConstraintFlags::DisplayNullable
                    | TypeConstraintFlags::ExtendedHint,
            )?
        }
        Happly(Id(_, s), hs) => {
            match &hs[..] {
                [] if s == "\\HH\\dynamic"
                    || s == "\\HH\\mixed"
                    || (skipawaitable && is_awaitable(s))
                    || (s.eq_ignore_ascii_case("\\HH\\void") && !is_typedef(kind)) =>
                {
                    return Ok(Constraint::default());
                }
                [Hint(_, h)] if skipawaitable && is_awaitable(s) => {
                    return match &**h {
                        Hprim(Tprim::Tvoid) => Ok(Constraint::default()),
                        Happly(Id(_, id), hs) if id == "\\HH\\void" && hs.is_empty() => {
                            Ok(Constraint::default())
                        }
                        _ => hint_to_type_constraint(alloc, kind, tparams, false, &hs[0]),
                    };
                }
                [h] if s == typehints::POISON_MARKER || s == typehints::HH_FUNCTIONREF => {
                    return hint_to_type_constraint(alloc, kind, tparams, false, h);
                }
                _ => {}
            };
            type_application_helper(alloc, tparams, kind, s)?
        }
        Habstr(s, _hs) => type_application_helper(alloc, tparams, kind, s)?,
        Hrefinement(hint, _) => {
            // NOTE: refinements are already banned in type structures
            // and in other cases they should be invisible to the HHVM, so unpack hint
            hint_to_type_constraint(alloc, kind, tparams, skipawaitable, hint)?
        }
        h => type_application_helper(alloc, tparams, kind, hint_to_string(h))?,
    })
}

fn is_awaitable(s: &str) -> bool {
    s == classes::AWAITABLE
}

fn is_typedef(kind: &Kind) -> bool {
    Kind::TypeDef == *kind
}

fn make_tc_with_flags_if_non_empty_flags<'arena>(
    alloc: &'arena bumpalo::Bump,
    kind: &Kind,
    tparams: &[&str],
    skipawaitable: bool,
    hint: &Hint,
    flags: TypeConstraintFlags,
) -> Result<Constraint<'arena>> {
    let tc = hint_to_type_constraint(alloc, kind, tparams, skipawaitable, hint)?;
    Ok(match (&tc.name, u16::from(&tc.flags)) {
        (Nothing, 0) => tc,
        _ => Constraint::make(tc.name, flags | tc.flags),
    })
}

// Used for nodes that do type application (i.e., Happly and Habstr)
fn type_application_helper<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    kind: &Kind,
    name: &str,
) -> Result<Constraint<'arena>> {
    if tparams.contains(&name) {
        let tc_name = match kind {
            Kind::Param | Kind::Return | Kind::Property => Just(Str::new_str(alloc, name)),
            _ => Just("".into()),
        };
        Ok(Constraint::make(
            tc_name,
            TypeConstraintFlags::ExtendedHint | TypeConstraintFlags::TypeVar,
        ))
    } else {
        let name: String =
            hhbc::ClassName::from_ast_name_and_mangle(alloc, name).unsafe_into_string();
        Ok(Constraint::make(
            Just(Str::new_str(alloc, &name)),
            TypeConstraintFlags::NoFlags,
        ))
    }
}

fn add_nullable(nullable: bool, flags: TypeConstraintFlags) -> TypeConstraintFlags {
    if nullable {
        TypeConstraintFlags::Nullable | TypeConstraintFlags::DisplayNullable | flags
    } else {
        flags
    }
}

fn try_add_nullable(
    nullable: bool,
    hint: &Hint,
    flags: TypeConstraintFlags,
) -> TypeConstraintFlags {
    let Hint(_, h) = hint;
    add_nullable(nullable && can_be_nullable(h), flags)
}

fn make_type_info<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    h: &Hint,
    tc_name: Maybe<Str<'arena>>,
    tc_flags: TypeConstraintFlags,
) -> Result<TypeInfo<'arena>> {
    let type_info_user_type = fmt_hint(alloc, tparams, false, h)?;
    let type_info_type_constraint = Constraint::make(tc_name, tc_flags);
    Ok(TypeInfo::make(
        Just(Str::new_str(alloc, &type_info_user_type)),
        type_info_type_constraint,
    ))
}

fn param_hint_to_type_info<'arena>(
    alloc: &'arena bumpalo::Bump,
    kind: &Kind,
    skipawaitable: bool,
    nullable: bool,
    tparams: &[&str],
    hint: &Hint,
) -> Result<TypeInfo<'arena>> {
    let Hint(_, h) = hint;
    let is_simple_hint = match h.as_ref() {
        Hsoft(_) | Hoption(_) | Haccess(_, _) | Hfun(_) | Hdynamic | Hnonnull | Hmixed => false,
        Happly(Id(_, s), hs) => {
            hs.is_empty()
                && s != "\\HH\\dynamic"
                && s != "\\HH\\nonnull"
                && s != "\\HH\\mixed"
                && !tparams.contains(&s.as_str())
        }
        Habstr(s, hs) => hs.is_empty() && !tparams.contains(&s.as_str()),
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
                TypeConstraintFlags::NoFlags
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
) -> Result<TypeInfo<'arena>> {
    if let Kind::Param = kind {
        return param_hint_to_type_info(alloc, kind, skipawaitable, nullable, tparams, hint);
    };
    let tc = hint_to_type_constraint(alloc, kind, tparams, skipawaitable, hint)?;
    let flags = match kind {
        Kind::Return | Kind::Property if tc.name.is_just() => {
            TypeConstraintFlags::ExtendedHint | tc.flags
        }
        Kind::UpperBound => TypeConstraintFlags::UpperBound | tc.flags,
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

// Used from emit_typedef for potential case types
pub fn hint_to_type_info_union<'arena>(
    alloc: &'arena bumpalo::Bump,
    kind: &Kind,
    skipawaitable: bool,
    nullable: bool,
    tparams: &[&str],
    hint: &Hint,
) -> Result<ffi::Slice<'arena, TypeInfo<'arena>>> {
    let Hint(_, h) = hint;
    let mut result = vec![];
    match &**h {
        Hunion(hints) => {
            for hint in hints {
                result.push(hint_to_type_info(
                    alloc,
                    kind,
                    skipawaitable,
                    nullable,
                    tparams,
                    hint,
                )?)
            }
        }
        _ => result.push(hint_to_type_info(
            alloc,
            kind,
            skipawaitable,
            nullable,
            tparams,
            hint,
        )?),
    }
    Ok(ffi::Slice::from_vec(alloc, result))
}

pub fn hint_to_class<'arena>(alloc: &'arena bumpalo::Bump, hint: &Hint) -> hhbc::ClassName<'arena> {
    let Hint(_, h) = hint;
    if let Happly(Id(_, name), _) = &**h {
        hhbc::ClassName::from_ast_name_and_mangle(alloc, name)
    } else {
        hhbc::ClassName::from_raw_string(alloc, "__type_is_not_class__")
    }
}

pub fn emit_type_constraint_for_native_function<'arena>(
    alloc: &'arena bumpalo::Bump,
    tparams: &[&str],
    ret_opt: Option<&Hint>,
    ti: TypeInfo<'arena>,
) -> TypeInfo<'arena> {
    let (name, flags) = match (&ti.user_type, ret_opt) {
        (_, None) | (Nothing, _) => (
            Just(String::from("HH\\void")),
            TypeConstraintFlags::ExtendedHint,
        ),
        (Just(t), _) => match t.unsafe_as_str() {
            "HH\\mixed" | "callable" => (Nothing, TypeConstraintFlags::default()),
            "HH\\void" => {
                let Hint(_, h) = ret_opt.as_ref().unwrap();
                (
                    Just("HH\\void".to_string()),
                    get_flags(tparams, TypeConstraintFlags::ExtendedHint, h),
                )
            }
            _ => return ti,
        },
    };
    let tc = Constraint::make(name.map(|n| Str::new_str(alloc, &n)), flags);
    TypeInfo::make(ti.user_type, tc)
}

fn get_flags(tparams: &[&str], flags: TypeConstraintFlags, hint: &Hint_) -> TypeConstraintFlags {
    match hint {
        Hoption(x) => {
            let Hint(_, h) = x;
            TypeConstraintFlags::Nullable
                | TypeConstraintFlags::DisplayNullable
                | get_flags(tparams, flags, h)
        }
        Hsoft(x) => {
            let Hint(_, h) = x;
            TypeConstraintFlags::Soft | get_flags(tparams, flags, h)
        }
        Haccess(_, _) => TypeConstraintFlags::TypeConstant | flags,
        Happly(Id(_, s), _) if tparams.contains(&s.as_str()) => {
            TypeConstraintFlags::TypeVar | flags
        }
        _ => flags,
    }
}
