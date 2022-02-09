// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use env::{emitter::Emitter, Env};
use ffi::Maybe::*;
use hhas_property::HhasProperty;
use hhas_type::{constraint, HhasTypeInfo};
use hhbc_ast::InitPropOp;
use hhbc_id::prop;
use hhbc_string_utils as string_utils;
use hhvm_types_ffi::ffi::Attr;
use instruction_sequence::{instr, InstrSeq, Result};
use naming_special_names_rust::{pseudo_consts, user_attributes as ua};
use oxidized::{aast_defs, ast, ast_defs, doc_comment};
use runtime::TypedValue;

pub struct FromAstArgs<'ast> {
    pub user_attributes: &'ast [ast::UserAttribute],
    pub id: &'ast ast::Sid,
    pub initial_value: &'ast Option<ast::Expr>,
    pub typehint: Option<&'ast aast_defs::Hint>,
    pub doc_comment: Option<doc_comment::DocComment>,
    pub visibility: aast_defs::Visibility,
    pub is_static: bool,
    pub is_abstract: bool,
    pub is_readonly: bool,
}

pub fn from_ast<'ast, 'arena, 'decl>(
    emitter: &mut Emitter<'arena, 'decl>,
    class: &'ast ast::Class_,
    tparams: &[&str],
    class_is_const: bool,
    class_is_closure: bool,
    args: FromAstArgs<'_>,
) -> Result<HhasProperty<'arena>> {
    let alloc = emitter.alloc;
    let ast_defs::Id(pos, cv_name) = args.id;
    let pid: prop::PropType<'arena> = prop::PropType::from_ast_name(alloc, cv_name);
    let attributes = emit_attribute::from_asts(emitter, args.user_attributes)?;

    let is_const = (!args.is_static && class_is_const)
        || attributes
            .iter()
            .any(|a| a.name.unsafe_as_str() == ua::CONST);
    let is_lsb = attributes.iter().any(|a| a.name.unsafe_as_str() == ua::LSB);
    let is_late_init = attributes
        .iter()
        .any(|a| a.name.unsafe_as_str() == ua::LATE_INIT);

    let is_cabstract = match class.kind {
        ast_defs::ClassishKind::Cclass(k) => k.is_abstract(),
        _ => false,
    };
    if !args.is_static && class.final_ && is_cabstract {
        return Err(emit_fatal::raise_fatal_parse(
            pos,
            format!(
                "Class {} contains non-static property declaration and therefore cannot be declared 'abstract final'",
                string_utils::strip_global_ns(&class.name.1),
            ),
        ));
    };

    let type_info = match args.typehint.as_ref() {
        None => HhasTypeInfo::make_empty(alloc),
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
            pos,
            format!(
                "Invalid property type hint for '{}::${}'",
                string_utils::strip_global_ns(&class.name.1),
                pid.unsafe_as_str()
            ),
        ));
    };

    let env = Env::make_class_env(alloc, class);
    let (initial_value, initializer_instrs, mut hhas_property_flags) = match args.initial_value {
        None => {
            let initial_value = if is_late_init || class_is_closure {
                Some(TypedValue::Uninit)
            } else {
                None
            };
            (initial_value, None, Attr::AttrSystemInitialValue)
        }
        Some(_) if is_late_init => {
            return Err(emit_fatal::raise_fatal_parse(
                pos,
                format!(
                    "<<__LateInit>> property '{}::${}' cannot have an initial value",
                    string_utils::strip_global_ns(&class.name.1),
                    pid.unsafe_as_str()
                ),
            ));
        }
        Some(e) => {
            let is_collection_map = match e.2.as_collection() {
                Some(c) if (c.0).1 == "Map" || (c.0).1 == "ImmMap" => true,
                _ => false,
            };
            let deep_init = !args.is_static
                && expr_requires_deep_init(e, emitter.options().emit_class_pointers() > 0);
            match ast_constant_folder::expr_to_typed_value(emitter, e) {
                Ok(tv) if !(deep_init || is_collection_map) => (Some(tv), None, Attr::AttrNone),
                _ => {
                    let label = emitter.label_gen_mut().next_regular();
                    let (prolog, epilog) = if args.is_static {
                        (
                            instr::empty(alloc),
                            emit_pos::emit_pos_then(
                                alloc,
                                &class.span,
                                instr::initprop(alloc, pid, InitPropOp::Static),
                            ),
                        )
                    } else if args.visibility.is_private() {
                        (
                            instr::empty(alloc),
                            emit_pos::emit_pos_then(
                                alloc,
                                &class.span,
                                instr::initprop(alloc, pid, InitPropOp::NonStatic),
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
                                    instr::initprop(alloc, pid, InitPropOp::NonStatic),
                                    instr::label(alloc, label),
                                ],
                            ),
                        )
                    };
                    let mut flags = Attr::AttrNone;
                    flags.set(Attr::AttrDeepInit, deep_init);
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

    hhas_property_flags.set(Attr::AttrAbstract, args.is_abstract);
    hhas_property_flags.set(Attr::AttrStatic, args.is_static);
    hhas_property_flags.set(Attr::AttrLSB, is_lsb);
    hhas_property_flags.set(Attr::AttrIsConst, is_const);
    hhas_property_flags.set(Attr::AttrLateInit, is_late_init);
    hhas_property_flags.set(Attr::AttrIsReadonly, args.is_readonly);
    hhas_property_flags.add(Attr::from(args.visibility));

    Ok(HhasProperty {
        name: pid,
        attributes: alloc.alloc_slice_fill_iter(attributes.into_iter()).into(),
        type_info,
        initial_value: initial_value.into(),
        initializer_instrs: initializer_instrs.into(),
        flags: hhas_property_flags,
        visibility: hhbc_ast::Visibility::from(args.visibility),
        doc_comment: args
            .doc_comment
            .map(|pstr| ffi::Str::from(alloc.alloc_str(&pstr.0.1)))
            .into(),
    })
}

fn valid_for_prop(tc: &constraint::Constraint<'_>) -> bool {
    match &tc.name {
        Nothing => true,
        Just(s) => {
            !(s.unsafe_as_str().eq_ignore_ascii_case("hh\\nothing")
                || s.unsafe_as_str().eq_ignore_ascii_case("hh\\noreturn")
                || s.unsafe_as_str().eq_ignore_ascii_case("callable"))
        }
    }
}

fn expr_requires_deep_init_(expr: &ast::Expr) -> bool {
    expr_requires_deep_init(expr, false)
}

fn expr_requires_deep_init(expr: &ast::Expr, force_class_init: bool) -> bool {
    use ast::Expr_;
    use ast_defs::Uop::*;
    match &expr.2 {
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
            Some(ci_expr) => match (ci_expr.2).as_id() {
                Some(ast_defs::Id(_, s)) => {
                    class_const_requires_deep_init(s.as_str(), (e.1).1.as_str())
                }
                _ => true,
            },
            None => true,
        },
        _ => true,
    }
}

fn af_expr_requires_deep_init(af: &ast::Afield) -> bool {
    match af {
        ast::Afield::AFvalue(e) => expr_requires_deep_init_(e),
        ast::Afield::AFkvalue(e1, e2) => {
            expr_requires_deep_init_(e1) || expr_requires_deep_init_(e2)
        }
    }
}

fn expr_pair_requires_deep_init((e1, e2): &(ast::Expr, ast::Expr)) -> bool {
    expr_requires_deep_init_(e1) || expr_requires_deep_init_(e2)
}

fn shape_field_requires_deep_init((name, expr): &(ast_defs::ShapeFieldName, ast::Expr)) -> bool {
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
