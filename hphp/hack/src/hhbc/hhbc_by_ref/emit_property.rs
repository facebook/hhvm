// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use hhbc_by_ref_ast_constant_folder as ast_constant_folder;
use hhbc_by_ref_emit_attribute as emit_attribute;
use hhbc_by_ref_emit_expression as emit_expression;
use hhbc_by_ref_emit_fatal as emit_fatal;
use hhbc_by_ref_emit_pos as emit_pos;
use hhbc_by_ref_emit_type_hint as emit_type_hint;
use hhbc_by_ref_env::{emitter::Emitter, Env};
use hhbc_by_ref_hhas_property::{HhasProperty, HhasPropertyFlags};
use hhbc_by_ref_hhas_type::{constraint, Info as TypeInfo};
use hhbc_by_ref_hhbc_ast::InitpropOp;
use hhbc_by_ref_hhbc_id::{prop, Id};
use hhbc_by_ref_hhbc_string_utils as string_utils;
use hhbc_by_ref_instruction_sequence::{instr, InstrSeq, Result};
use hhbc_by_ref_runtime::TypedValue;
use naming_special_names_rust::{pseudo_consts, user_attributes as ua};
use oxidized::{aast_defs, ast as tast, ast_defs, doc_comment};

pub struct FromAstArgs<'ast> {
    pub user_attributes: &'ast [tast::UserAttribute],
    pub id: &'ast tast::Sid,
    pub initial_value: &'ast Option<tast::Expr>,
    pub typehint: Option<&'ast aast_defs::Hint>,
    pub doc_comment: Option<doc_comment::DocComment>,
    pub visibility: aast_defs::Visibility,
    pub is_static: bool,
    pub is_abstract: bool,
    pub is_readonly: bool,
}

pub fn from_ast<'ast, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    class: &'ast tast::Class_,
    tparams: &[&str],
    class_is_const: bool,
    args: FromAstArgs,
) -> Result<HhasProperty<'arena>> {
    let ast_defs::Id(pos, cv_name) = args.id;
    let pid: prop::Type<'arena> = prop::Type::from_ast_name(alloc, cv_name);
    let attributes = emit_attribute::from_asts(alloc, emitter, args.user_attributes)?;

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
            alloc,
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

    let env = Env::make_class_env(alloc, class);
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
            match ast_constant_folder::expr_to_typed_value(alloc, emitter, e) {
                Ok(tv) if !(deep_init || is_collection_map) => {
                    (Some(tv), None, HhasPropertyFlags::empty())
                }
                _ => {
                    let label = emitter.label_gen_mut().next_regular(alloc);
                    let (prolog, epilog) = if args.is_static {
                        (
                            instr::empty(alloc),
                            emit_pos::emit_pos_then(
                                alloc,
                                &class.span,
                                instr::initprop(alloc, pid, InitpropOp::Static),
                            ),
                        )
                    } else if args.visibility.is_private() {
                        (
                            instr::empty(alloc),
                            emit_pos::emit_pos_then(
                                alloc,
                                &class.span,
                                instr::initprop(alloc, pid, InitpropOp::NonStatic),
                            ),
                        )
                    } else {
                        (
                            InstrSeq::gather(
                                alloc,
                                vec![
                                    emit_pos::emit_pos(alloc, &class.span),
                                    instr::checkprop(alloc, pid),
                                    instr::jmpnz(alloc, label.clone()),
                                ],
                            ),
                            InstrSeq::gather(
                                alloc,
                                vec![
                                    emit_pos::emit_pos(alloc, &class.span),
                                    instr::initprop(alloc, pid, InitpropOp::NonStatic),
                                    instr::label(alloc, label),
                                ],
                            ),
                        )
                    };
                    let mut flags = HhasPropertyFlags::empty();
                    flags.set(HhasPropertyFlags::IS_DEEP_INIT, deep_init);
                    (
                        Some(TypedValue::Uninit),
                        Some(InstrSeq::gather(
                            alloc,
                            vec![
                                prolog,
                                emit_expression::emit_expr(emitter, &env, e)?,
                                epilog,
                            ],
                        )),
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
    expr_requires_deep_init(expr, false)
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
