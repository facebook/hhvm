// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::borrow::Cow;

use error::Error;
use error::Result;
use ffi::Maybe;
use ffi::Maybe::*;
use hhbc::string_id;
use hhbc::ClassName;
use hhbc::Constraint;
use hhbc::StringId;
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

fn fmt_name_or_prim<'n>(tparams: &[&str], name: &'n str) -> Cow<'n, str> {
    if tparams.contains(&name) {
        name.into()
    } else {
        let id = ClassName::from_ast_name_and_mangle(name);
        if string_utils::is_xhp(string_utils::strip_ns(name)) {
            id.unmangled().into()
        } else {
            id.as_str().into()
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
            let name = fmt_name_or_prim(tparams, id);
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
                    fmt_name_or_prim(tparams, id),
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
                    format!("{}::{}", fmt_name_or_prim(tparams, cid), s)
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
        h => fmt_name_or_prim(tparams, hint_to_string(h)).into(),
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

fn hint_to_type_constraint(
    alloc: &bumpalo::Bump,
    kind: &Kind,
    tparams: &[&str],
    skipawaitable: bool,
    h: &Hint,
) -> Result<Constraint> {
    let Hint(_, hint) = h;
    Ok(match &**hint {
        Hdynamic | Hfun(_) | Hunion(_) | Hintersection(_) | Hmixed | Hwildcard => {
            Constraint::default()
        }
        Haccess(_, _) => Constraint::intern(
            "",
            TypeConstraintFlags::ExtendedHint | TypeConstraintFlags::TypeConstant,
        ),
        Hshape(_) => Constraint::intern("HH\\darray", TypeConstraintFlags::ExtendedHint),
        Htuple(_) => Constraint::intern("HH\\varray", TypeConstraintFlags::ExtendedHint),
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
            type_application_helper(tparams, kind, s)?
        }
        Habstr(s, _hs) => type_application_helper(tparams, kind, s)?,
        Hrefinement(hint, _) => {
            // NOTE: refinements are already banned in type structures
            // and in other cases they should be invisible to the HHVM, so unpack hint
            hint_to_type_constraint(alloc, kind, tparams, skipawaitable, hint)?
        }
        h => type_application_helper(tparams, kind, hint_to_string(h))?,
    })
}

fn is_awaitable(s: &str) -> bool {
    s == classes::AWAITABLE
}

fn is_typedef(kind: &Kind) -> bool {
    Kind::TypeDef == *kind
}

fn make_tc_with_flags_if_non_empty_flags(
    alloc: &bumpalo::Bump,
    kind: &Kind,
    tparams: &[&str],
    skipawaitable: bool,
    hint: &Hint,
    flags: TypeConstraintFlags,
) -> Result<Constraint> {
    let tc = hint_to_type_constraint(alloc, kind, tparams, skipawaitable, hint)?;
    Ok(match (&tc.name, u16::from(&tc.flags)) {
        (Nothing, 0) => tc,
        _ => Constraint {
            name: tc.name,
            flags: flags | tc.flags,
        },
    })
}

// Used for nodes that do type application (i.e., Happly and Habstr)
fn type_application_helper(tparams: &[&str], kind: &Kind, name: &str) -> Result<Constraint> {
    if tparams.contains(&name) {
        let tc_name = match kind {
            Kind::Param | Kind::Return | Kind::Property => Just(hhbc::intern(name)),
            _ => Just(hhbc::StringId::EMPTY),
        };
        Ok(Constraint {
            name: tc_name,
            flags: TypeConstraintFlags::ExtendedHint | TypeConstraintFlags::TypeVar,
        })
    } else {
        let name = ClassName::mangle(name);
        Ok(Constraint {
            name: Just(name),
            flags: TypeConstraintFlags::NoFlags,
        })
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

fn make_type_info(
    alloc: &bumpalo::Bump,
    tparams: &[&str],
    h: &Hint,
    tc_name: Maybe<StringId>,
    tc_flags: TypeConstraintFlags,
) -> Result<TypeInfo> {
    let type_info_user_type = fmt_hint(alloc, tparams, false, h)?;
    let type_info_type_constraint = Constraint::new(tc_name, tc_flags);
    Ok(TypeInfo::new(
        Just(hhbc::intern(type_info_user_type)),
        type_info_type_constraint,
    ))
}

fn param_hint_to_type_info(
    alloc: &bumpalo::Bump,
    kind: &Kind,
    skipawaitable: bool,
    nullable: bool,
    tparams: &[&str],
    hint: &Hint,
) -> Result<TypeInfo> {
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

pub fn hint_to_type_info(
    alloc: &bumpalo::Bump,
    kind: &Kind,
    skipawaitable: bool,
    nullable: bool,
    tparams: &[&str],
    hint: &Hint,
) -> Result<TypeInfo> {
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
pub fn hint_to_type_info_union(
    alloc: &bumpalo::Bump,
    kind: &Kind,
    skipawaitable: bool,
    nullable: bool,
    tparams: &[&str],
    hint: &Hint,
) -> Result<Vec<TypeInfo>> {
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
    Ok(result)
}

pub fn hint_to_class(hint: &Hint) -> ClassName {
    let Hint(_, h) = hint;
    if let Happly(Id(_, name), _) = &**h {
        ClassName::from_ast_name_and_mangle(name)
    } else {
        ClassName::new(string_id!("__type_is_not_class__"))
    }
}

pub fn emit_type_constraint_for_native_function(
    tparams: &[&str],
    ret_opt: Option<&Hint>,
    ti: TypeInfo,
) -> TypeInfo {
    let (name, flags) = match (&ti.user_type, ret_opt) {
        (_, None) | (Nothing, _) => (Just("HH\\void"), TypeConstraintFlags::ExtendedHint),
        (Just(t), _) => match t.as_str() {
            "HH\\mixed" | "callable" => (Nothing, TypeConstraintFlags::default()),
            "HH\\void" => {
                let Hint(_, h) = ret_opt.as_ref().unwrap();
                (
                    Just("HH\\void"),
                    get_flags(tparams, TypeConstraintFlags::ExtendedHint, h),
                )
            }
            _ => return ti,
        },
    };
    let tc = Constraint::new(name.map(hhbc::intern), flags);
    TypeInfo::new(ti.user_type, tc)
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
