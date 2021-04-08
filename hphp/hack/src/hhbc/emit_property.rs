// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ast_constant_folder_rust as ast_constant_folder;
use emit_attribute_rust as emit_attribute;
use emit_expression_rust as emit_expression;
use emit_fatal_rust as emit_fatal;
use emit_pos_rust as emit_pos;
use emit_type_hint_rust as emit_type_hint;
use env::{emitter::Emitter, Env};
use hhas_property_rust::{HhasProperty, HhasPropertyFlags};
use hhas_type::{constraint, Info as TypeInfo};
use hhbc_ast_rust::InitpropOp;
use hhbc_id_rust::{prop, Id};
use hhbc_string_utils_rust as string_utils;
use instruction_sequence::{instr, InstrSeq, Result};
use naming_special_names_rust::{pseudo_consts, user_attributes as ua};
use oxidized::{aast_defs, ast as tast, ast_defs, doc_comment, namespace_env};
use runtime::TypedValue;

pub struct FromAstArgs<'a> {
    pub user_attributes: &'a [tast::UserAttribute],
    pub id: &'a tast::Sid,
    pub initial_value: &'a Option<tast::Expr>,
    pub typehint: Option<&'a aast_defs::Hint>,
    pub doc_comment: Option<doc_comment::DocComment>,
    pub visibility: aast_defs::Visibility,
    pub is_static: bool,
    pub is_abstract: bool,
    pub is_readonly: bool,
}

pub fn from_ast<'a>(
    emitter: &mut Emitter,
    class: &'a tast::Class_,
    namespace: &namespace_env::Env,
    tparams: &[&str],
    class_is_const: bool,
    args: FromAstArgs,
) -> Result<HhasProperty<'a>> {
    let ast_defs::Id(pos, cv_name) = args.id;
    // hhbc_ast.rs definitions require static (owned) strings
    let pid: String = prop::Type::from_ast_name(cv_name).into();
    let pid: prop::Type<'static> = pid.into();
    let attributes = emit_attribute::from_asts(emitter, namespace, args.user_attributes)?;

    let is_const =
        (!args.is_static && class_is_const) || attributes.iter().any(|a| a.name == ua::CONST);
    let is_lsb = attributes.iter().any(|a| a.name == ua::LSB);
    let is_late_init = attributes.iter().any(|a| a.name == ua::LATE_INIT);

    if !args.is_static && class.final_ && class.kind.is_cabstract() {
        return Err(emit_fatal::raise_fatal_parse(
            &pos,
            format!(
                "Class {} contains non-static property declaration and therefore cannot be declared 'abstract final'",
                string_utils::strip_global_ns(&class.name.1),
            ),
        ));
    };

    let type_info = match args.typehint.as_ref() {
        None => TypeInfo::make_empty(),
        Some(th) => emit_type_hint::hint_to_type_info(
            &emit_type_hint::Kind::Property,
            false,
            false,
            tparams,
            th,
        )?,
    };
    if !(valid_for_prop(&type_info.type_constraint)) {
        return Err(emit_fatal::raise_fatal_parse(
            &pos,
            format!(
                "Invalid property type hint for '{}::${}'",
                string_utils::strip_global_ns(&class.name.1),
                prop::Type::to_raw_string(&pid)
            ),
        ));
    };

    let env = Env::make_class_env(class);
    let (initial_value, initializer_instrs, mut hhas_property_flags) = match args.initial_value {
        None => {
            let initial_value = if is_late_init {
                Some(TypedValue::Uninit)
            } else {
                None
            };
            (initial_value, None, HhasPropertyFlags::HAS_SYSTEM_INITIAL)
        }
        Some(_) if is_late_init => {
            return Err(emit_fatal::raise_fatal_parse(
                &pos,
                format!(
                    "<<__LateInit>> property '{}::${}' cannot have an initial value",
                    string_utils::strip_global_ns(&class.name.1),
                    prop::Type::to_raw_string(&pid)
                ),
            ));
        }
        Some(e) => {
            let is_collection_map = match e.1.as_collection() {
                Some(c) if (c.0).1 == "Map" || (c.0).1 == "ImmMap" => true,
                _ => false,
            };
            let deep_init = !args.is_static
                && expr_requires_deep_init(e, emitter.options().emit_class_pointers() > 0);
            match ast_constant_folder::expr_to_typed_value(emitter, &env.namespace, e) {
                Ok(tv) if !(deep_init || is_collection_map) => {
                    (Some(tv), None, HhasPropertyFlags::empty())
                }
                _ => {
                    let label = emitter.label_gen_mut().next_regular();
                    let (prolog, epilog) = if args.is_static {
                        (
                            instr::empty(),
                            emit_pos::emit_pos_then(
                                &class.span,
                                instr::initprop(pid.clone(), InitpropOp::Static),
                            ),
                        )
                    } else if args.visibility.is_private() {
                        (
                            instr::empty(),
                            emit_pos::emit_pos_then(
                                &class.span,
                                instr::initprop(pid.clone(), InitpropOp::NonStatic),
                            ),
                        )
                    } else {
                        (
                            InstrSeq::gather(vec![
                                emit_pos::emit_pos(&class.span),
                                instr::checkprop(pid.clone()),
                                instr::jmpnz(label.clone()),
                            ]),
                            InstrSeq::gather(vec![
                                emit_pos::emit_pos(&class.span),
                                instr::initprop(pid.clone(), InitpropOp::NonStatic),
                                instr::label(label),
                            ]),
                        )
                    };
                    let mut flags = HhasPropertyFlags::empty();
                    flags.set(HhasPropertyFlags::IS_DEEP_INIT, deep_init);
                    (
                        Some(TypedValue::Uninit),
                        Some(InstrSeq::gather(vec![
                            prolog,
                            emit_expression::emit_expr(emitter, &env, e)?,
                            epilog,
                        ])),
                        flags,
                    )
                }
            }
        }
    };

    hhas_property_flags.set(HhasPropertyFlags::IS_ABSTRACT, args.is_abstract);
    hhas_property_flags.set(HhasPropertyFlags::IS_STATIC, args.is_static);
    hhas_property_flags.set(HhasPropertyFlags::IS_LSB, is_lsb);
    hhas_property_flags.set(HhasPropertyFlags::IS_CONST, is_const);
    hhas_property_flags.set(HhasPropertyFlags::IS_LATE_INIT, is_late_init);
    hhas_property_flags.set(HhasPropertyFlags::IS_READONLY, args.is_readonly);

    Ok(HhasProperty {
        name: pid,
        attributes,
        type_info,
        initial_value,
        initializer_instrs,
        flags: hhas_property_flags,
        visibility: args.visibility,
        doc_comment: args.doc_comment,
    })
}

fn valid_for_prop(tc: &constraint::Type) -> bool {
    match &tc.name {
        None => true,
        Some(s) => {
            !(string_utils::is_self(&s)
                || string_utils::is_parent(&s)
                || s.eq_ignore_ascii_case("hh\\nothing")
                || s.eq_ignore_ascii_case("hh\\noreturn")
                || s.eq_ignore_ascii_case("callable"))
        }
    }
}

fn expr_requires_deep_init_(expr: &tast::Expr) -> bool {
    return expr_requires_deep_init(expr, false);
}

fn expr_requires_deep_init(expr: &tast::Expr, force_class_init: bool) -> bool {
    use ast_defs::Uop::*;
    use tast::Expr_;
    match &expr.1 {
        Expr_::Unop(e) if e.0 == Uplus || e.0 == Uminus => expr_requires_deep_init_(&e.1),
        Expr_::Binop(e) => expr_requires_deep_init_(&e.1) || expr_requires_deep_init_(&e.2),
        Expr_::Lvar(_)
        | Expr_::Null
        | Expr_::False
        | Expr_::True
        | Expr_::Int(_)
        | Expr_::Float(_)
        | Expr_::String(_) => false,
        Expr_::Collection(e) if (e.0).1 == "keyset" || (e.0).1 == "dict" || (e.0).1 == "vec" => {
            (e.2).iter().any(af_expr_requires_deep_init)
        }
        Expr_::Varray(e) => (e.1).iter().any(expr_requires_deep_init_),
        Expr_::Darray(e) => (e.1).iter().any(expr_pair_requires_deep_init),
        Expr_::Id(e) if e.1 == pseudo_consts::G__FILE__ || e.1 == pseudo_consts::G__DIR__ => false,
        Expr_::Shape(sfs) => sfs.iter().any(shape_field_requires_deep_init),
        Expr_::ClassConst(e) if (!force_class_init) => match e.0.as_ciexpr() {
            Some(ci_expr) => match (ci_expr.1).as_id() {
                Some(ast_defs::Id(_, s)) => {
                    class_const_requires_deep_init(&s.as_str(), &(e.1).1.as_str())
                }
                _ => true,
            },
            None => true,
        },
        _ => true,
    }
}

fn af_expr_requires_deep_init(af: &tast::Afield) -> bool {
    match af {
        tast::Afield::AFvalue(e) => expr_requires_deep_init_(e),
        tast::Afield::AFkvalue(e1, e2) => {
            expr_requires_deep_init_(e1) || expr_requires_deep_init_(e2)
        }
    }
}

fn expr_pair_requires_deep_init((e1, e2): &(tast::Expr, tast::Expr)) -> bool {
    expr_requires_deep_init_(e1) || expr_requires_deep_init_(e2)
}

fn shape_field_requires_deep_init((name, expr): &(ast_defs::ShapeFieldName, tast::Expr)) -> bool {
    match name {
        ast_defs::ShapeFieldName::SFlitInt(_) | ast_defs::ShapeFieldName::SFlitStr(_) => {
            expr_requires_deep_init_(expr)
        }
        ast_defs::ShapeFieldName::SFclassConst(ast_defs::Id(_, s), (_, p)) => {
            class_const_requires_deep_init(s, p)
        }
    }
}

fn class_const_requires_deep_init(s: &str, p: &str) -> bool {
    !string_utils::is_class(p)
        || string_utils::is_self(s)
        || string_utils::is_parent(s)
        || string_utils::is_static(s)
}
