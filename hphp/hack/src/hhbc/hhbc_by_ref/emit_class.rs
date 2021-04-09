// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_by_ref_emit_attribute as emit_attribute;
use hhbc_by_ref_emit_body as emit_body;
use hhbc_by_ref_emit_expression as emit_expression;
use hhbc_by_ref_emit_fatal as emit_fatal;
use hhbc_by_ref_emit_memoize_method as emit_memoize_method;
use hhbc_by_ref_emit_method as emit_method;
use hhbc_by_ref_emit_pos as emit_pos;
use hhbc_by_ref_emit_property as emit_property;
use hhbc_by_ref_emit_symbol_refs as emit_symbol_refs;
use hhbc_by_ref_emit_type_constant as emit_type_constant;
use hhbc_by_ref_emit_type_hint as emit_type_hint;
use hhbc_by_ref_emit_xhp as emit_xhp;
use hhbc_by_ref_env::{emitter::Emitter, Env};
use hhbc_by_ref_hhas_attribute as hhas_attribute;
use hhbc_by_ref_hhas_class::{HhasClass, HhasClassFlags, TraitReqKind};
use hhbc_by_ref_hhas_coeffects::{HhasCoeffects, HhasCtxConstant};
use hhbc_by_ref_hhas_constant::{self as hhas_constant, HhasConstant};
use hhbc_by_ref_hhas_method::{HhasMethod, HhasMethodFlags};
use hhbc_by_ref_hhas_param::HhasParam;
use hhbc_by_ref_hhas_pos::Span;
use hhbc_by_ref_hhas_property::HhasProperty;
use hhbc_by_ref_hhas_type_const::HhasTypeConstant;
use hhbc_by_ref_hhas_xhp_attribute::HhasXhpAttribute;
use hhbc_by_ref_hhbc_ast::{FatalOp, FcallArgs, FcallFlags, ReadOnlyOp, SpecialClsRef};
use hhbc_by_ref_hhbc_id::r#const;
use hhbc_by_ref_hhbc_id::{self as hhbc_id, class, method, prop, Id};
use hhbc_by_ref_hhbc_string_utils as string_utils;
use hhbc_by_ref_instruction_sequence::{instr, InstrSeq, Result};
use hhbc_by_ref_label as label;
use hhbc_by_ref_runtime::TypedValue;
use naming_special_names_rust as special_names;
use oxidized::{
    ast::{self as tast, Hint, ReifyKind, Visibility},
    namespace_env,
    pos::Pos,
};

use std::collections::BTreeMap;

fn add_symbol_refs<'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    base: &Option<class::Type<'arena>>,
    implements: &[class::Type<'arena>],
    uses: &[&str],
    requirements: &[(class::Type<'arena>, TraitReqKind)],
) {
    base.iter()
        .for_each(|x| emit_symbol_refs::add_class(alloc, emitter, *x));
    implements
        .iter()
        .for_each(|x| emit_symbol_refs::add_class(alloc, emitter, *x));
    uses.iter().for_each(|x| {
        emit_symbol_refs::add_class(alloc, emitter, class::Type::from_ast_name(alloc, x))
    });
    requirements
        .iter()
        .for_each(|(x, _)| emit_symbol_refs::add_class(alloc, emitter, *x));
}

fn make_86method<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    name: method::Type<'arena>,
    params: Vec<HhasParam<'arena>>,
    is_static: bool,
    visibility: Visibility,
    is_abstract: bool,
    span: Span,
    instrs: InstrSeq<'arena>,
) -> Result<HhasMethod<'arena>> {
    // TODO: move this. We just know that there are no iterators in 86methods
    emitter.iterator_mut().reset();

    let mut flags = HhasMethodFlags::empty();
    flags.set(HhasMethodFlags::NO_INJECTION, true);
    flags.set(HhasMethodFlags::IS_ABSTRACT, is_abstract);
    flags.set(HhasMethodFlags::IS_STATIC, is_static);

    let attributes = vec![];
    let coeffects = HhasCoeffects::default();

    let method_decl_vars = vec![];
    let method_return_type = None;
    let method_doc_comment = None;
    let method_is_memoize_wrapper = false;
    let method_is_memoize_wrapper_lsb = false;
    let method_env = None;

    let body = emit_body::make_body(
        alloc,
        emitter,
        instrs,
        method_decl_vars,
        method_is_memoize_wrapper,
        method_is_memoize_wrapper_lsb,
        vec![],
        vec![],
        params,
        method_return_type,
        method_doc_comment,
        method_env,
    )?;

    Ok(HhasMethod {
        body,
        attributes,
        name,
        flags,
        span,
        coeffects,
        visibility,
    })
}

fn from_extends<'arena>(
    alloc: &'arena bumpalo::Bump,
    is_enum: bool,
    is_enum_class: bool,
    extends: &[tast::Hint],
) -> Option<hhbc_id::class::Type<'arena>> {
    if is_enum {
        // Do not use special_names:: as there's a prefix \ which breaks HHVM
        if is_enum_class {
            Some(hhbc_id::class::from_raw_string(
                alloc,
                "HH\\BuiltinEnumClass",
            ))
        } else {
            Some(hhbc_id::class::from_raw_string(alloc, "HH\\BuiltinEnum"))
        }
    } else {
        extends
            .first()
            .map(|x| emit_type_hint::hint_to_class(alloc, x))
    }
}

fn from_implements<'arena>(
    alloc: &'arena bumpalo::Bump,
    implements: &[tast::Hint],
) -> Vec<hhbc_id::class::Type<'arena>> {
    implements
        .iter()
        .map(|x| emit_type_hint::hint_to_class(alloc, x))
        .collect()
}

fn from_includes<'arena>(
    alloc: &'arena bumpalo::Bump,
    includes: &[tast::Hint],
) -> Vec<hhbc_id::class::Type<'arena>> {
    includes
        .iter()
        .map(|x| emit_type_hint::hint_to_class(alloc, x))
        .collect()
}

fn from_type_constant<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    tc: &'a tast::ClassTypeconstDef,
) -> Result<HhasTypeConstant<'arena>> {
    use tast::ClassTypeconst::*;
    let name = tc.name.1.to_string();

    let initializer = match &tc.kind {
        TCAbstract(tast::ClassAbstractTypeconst { default: None, .. }) => None,
        TCAbstract(tast::ClassAbstractTypeconst {
            default: Some(init),
            ..
        })
        | TCPartiallyAbstract(tast::ClassPartiallyAbstractTypeconst { type_: init, .. })
        | TCConcrete(tast::ClassConcreteTypeconst { c_tc_type: init }) => {
            // TODO: Deal with the constraint
            // Type constants do not take type vars hence tparams:[]
            Some(emit_type_constant::hint_to_type_constant(
                alloc,
                emitter.options(),
                &[],
                &BTreeMap::new(),
                init,
                false,
                false,
            )?)
        }
    };

    let is_abstract = match &tc.kind {
        TCConcrete(_) => false,
        _ => true,
    };

    Ok(HhasTypeConstant {
        name,
        initializer,
        is_abstract,
    })
}

fn from_ctx_constant(tc: &tast::ClassTypeconstDef) -> Result<HhasCtxConstant> {
    use tast::ClassTypeconst::*;
    let name = tc.name.1.to_string();
    let coeffects = match &tc.kind {
        TCAbstract(tast::ClassAbstractTypeconst { default: None, .. }) => vec![],
        TCPartiallyAbstract(_) => vec![], // does not parse
        TCAbstract(tast::ClassAbstractTypeconst {
            default: Some(hint),
            ..
        })
        | TCConcrete(tast::ClassConcreteTypeconst { c_tc_type: hint }) => {
            let result = HhasCoeffects::from_ctx_constant(hint);
            if result.is_empty() {
                vec![hhbc_by_ref_hhas_coeffects::Ctx::Pure]
            } else {
                result
            }
        }
    };
    let is_abstract = match &tc.kind {
        TCConcrete(_) => false,
        _ => true,
    };
    Ok(HhasCtxConstant {
        name,
        coeffects,
        is_abstract,
    })
}

fn from_class_elt_classvars<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    ast_class: &'a tast::Class_,
    class_is_const: bool,
    tparams: &[&str],
) -> Result<Vec<HhasProperty<'arena>>> {
    // TODO: we need to emit doc comments for each property,
    // not one per all properties on the same line
    // The doc comment is only for the first name in the list.
    // Currently this is organized in the ast_to_nast module
    ast_class
        .vars
        .iter()
        .map(|cv| {
            let hint = if cv.is_promoted_variadic {
                None
            } else {
                cv.type_.1.as_ref()
            };

            emit_property::from_ast(
                alloc,
                emitter,
                ast_class,
                tparams,
                class_is_const,
                emit_property::FromAstArgs {
                    user_attributes: &cv.user_attributes,
                    id: &cv.id,
                    initial_value: &cv.expr,
                    typehint: hint,
                    // Doc comments are weird. T40098274
                    doc_comment: cv.doc_comment.clone(),
                    visibility: cv.visibility, // This used to be cv_kinds
                    is_static: cv.is_static,
                    is_abstract: cv.abstract_,
                    is_readonly: cv.readonly,
                },
            )
        })
        .collect::<Result<Vec<_>>>()
}

fn from_class_elt_constants<'a, 'arena>(
    emitter: &mut Emitter<'arena>,
    env: &Env<'a, 'arena>,
    class_: &'a tast::Class_,
) -> Result<Vec<HhasConstant<'arena>>> {
    class_
        .consts
        .iter()
        .map(|x| hhas_constant::from_ast(emitter, env, &x.id, x.expr.as_ref()))
        .collect()
}

fn from_class_elt_requirements<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    class_: &'a tast::Class_,
) -> Vec<(hhbc_id::class::Type<'arena>, TraitReqKind)> {
    class_
        .reqs
        .iter()
        .map(|(h, is_extends)| {
            let kind = if *is_extends {
                TraitReqKind::MustExtend
            } else {
                TraitReqKind::MustImplement
            };
            (emit_type_hint::hint_to_class(alloc, h), kind)
        })
        .collect()
}

fn from_enum_type(opt: Option<&tast::Enum_>) -> Result<Option<hhbc_by_ref_hhas_type::Info>> {
    use hhbc_by_ref_hhas_type::constraint::*;
    opt.map(|e| {
        let alloc = bumpalo::Bump::new();
        let type_info_user_type = Some(emit_type_hint::fmt_hint(&alloc, &[], true, &e.base)?);
        let type_info_type_constraint = Type::make(None, Flags::EXTENDED_HINT);
        Ok(hhbc_by_ref_hhas_type::Info::make(
            type_info_user_type,
            type_info_type_constraint,
        ))
    })
    .transpose()
}

fn validate_class_name(ns: &namespace_env::Env, tast::Id(p, class_name): &tast::Id) -> Result<()> {
    let is_global_namespace = |ns: &namespace_env::Env| ns.name.is_none();
    let is_hh_namespace = |ns: &namespace_env::Env| {
        ns.name
            .as_ref()
            .map_or(false, |x| x.eq_ignore_ascii_case("hh"))
    };

    // global names are always reserved in any namespace.
    // hh_reserved names are checked either if
    // - class is in global namespace
    // - class is in HH namespace *)
    let is_special_class = class_name.contains('$');
    let check_hh_name = is_global_namespace(ns) || is_hh_namespace(ns);
    let name = string_utils::strip_ns(class_name);
    let lower_name = name.to_ascii_lowercase();
    let is_reserved_global_name = special_names::typehints::is_reserved_global_name(&lower_name);
    let name_is_reserved = !is_special_class
        && (is_reserved_global_name
            || (check_hh_name && special_names::typehints::is_reserved_hh_name(&lower_name)));
    if name_is_reserved {
        Err(emit_fatal::raise_fatal_parse(
            p,
            format!(
                "Cannot use '{}' as class name as it is reserved",
                if is_reserved_global_name {
                    &name
                } else {
                    string_utils::strip_global_ns(class_name)
                }
            ),
        ))
    } else {
        Ok(())
    }
}

fn emit_reified_extends_params<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &Env<'a, 'arena>,
    ast_class: &'a tast::Class_,
) -> Result<InstrSeq<'arena>> {
    let alloc = env.arena;
    match &ast_class.extends[..] {
        [h, ..] => match h.1.as_happly() {
            Some((_, l)) if !l.is_empty() => {
                return Ok(InstrSeq::gather(
                    alloc,
                    vec![
                        emit_expression::emit_reified_targs(
                            e,
                            env,
                            &ast_class.span,
                            &l.iter().collect::<Vec<_>>(),
                        )?,
                        instr::record_reified_generic(alloc),
                    ],
                ));
            }
            _ => {}
        },
        _ => {}
    }
    let tv = TypedValue::Vec(&[]);
    Ok(instr::typedvalue(alloc, tv))
}

fn emit_reified_init_body<'a, 'arena>(
    e: &mut Emitter<'arena>,
    env: &Env<'a, 'arena>,
    num_reified: usize,
    ast_class: &'a tast::Class_,
) -> Result<InstrSeq<'arena>> {
    use string_utils::reified::{INIT_METH_NAME, INIT_METH_PARAM_NAME, PROP_NAME};

    let alloc = env.arena;
    let check_length = InstrSeq::gather(
        alloc,
        vec![
            instr::cgetl(alloc, hhbc_by_ref_local::Type::Named(INIT_METH_PARAM_NAME)),
            instr::check_reified_generic_mismatch(alloc),
        ],
    );
    let set_prop = if num_reified == 0 {
        instr::empty(alloc)
    } else {
        InstrSeq::gather(
            alloc,
            vec![
                check_length,
                instr::checkthis(alloc),
                instr::cgetl(alloc, hhbc_by_ref_local::Type::Named(INIT_METH_PARAM_NAME)),
                instr::baseh(alloc),
                instr::setm_pt(
                    alloc,
                    0,
                    prop::from_raw_string(alloc, PROP_NAME),
                    ReadOnlyOp::Any,
                ),
                instr::popc(alloc),
            ],
        )
    };
    let return_instr = InstrSeq::gather(alloc, vec![instr::null(alloc), instr::retc(alloc)]);
    Ok(if ast_class.extends.is_empty() {
        InstrSeq::gather(alloc, vec![set_prop, return_instr])
    } else {
        let generic_arr = emit_reified_extends_params(e, env, ast_class)?;
        let call_parent = InstrSeq::gather(
            alloc,
            vec![
                instr::nulluninit(alloc),
                instr::nulluninit(alloc),
                generic_arr,
                instr::fcallclsmethodsd(
                    alloc,
                    FcallArgs::new(alloc, FcallFlags::default(), 1, &[], None, 1, None),
                    SpecialClsRef::Parent,
                    method::from_raw_string(alloc, INIT_METH_NAME),
                ),
                instr::popc(alloc),
            ],
        );
        InstrSeq::gather(alloc, vec![set_prop, call_parent, return_instr])
    })
}

fn emit_reified_init_method<'a, 'arena>(
    emitter: &mut Emitter<'arena>,
    env: &Env<'a, 'arena>,
    ast_class: &'a tast::Class_,
) -> Result<Option<HhasMethod<'arena>>> {
    use hhbc_by_ref_hhas_type::{constraint::*, Info};

    let alloc = env.arena;
    let num_reified = ast_class
        .tparams
        .iter()
        .filter(|x| x.reified != ReifyKind::Erased)
        .count();
    let maybe_has_reified_parents = match ast_class.extends.first().as_ref() {
        Some(Hint(_, h)) if h.as_happly().map_or(false, |(_, l)| !l.is_empty()) => true,
        _ => false, // Hack classes can only extend a single parent
    };
    if num_reified == 0 && !maybe_has_reified_parents {
        Ok(None)
    } else {
        let tc = Type::make(Some("HH\\varray".into()), Flags::empty());
        let params = vec![HhasParam {
            name: string_utils::reified::INIT_METH_PARAM_NAME.to_string(),
            is_variadic: false,
            is_inout: false,
            user_attributes: vec![],
            type_info: Some(Info::make(Some("HH\\varray".into()), tc)),
            default_value: None,
        }];

        let body_instrs = emit_reified_init_body(emitter, env, num_reified, ast_class)?;
        let instrs = emit_pos::emit_pos_then(alloc, &ast_class.span, body_instrs);
        Ok(Some(make_86method(
            alloc,
            emitter,
            (alloc, string_utils::reified::INIT_METH_NAME).into(),
            params,
            false, // is_static
            Visibility::Protected,
            false, // is_abstract
            Span::from_pos(&ast_class.span),
            instrs,
        )?))
    }
}

fn make_init_method<'a, 'arena, F>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    properties: &[HhasProperty<'arena>],
    filter: F,
    name: &'static str,
    span: Span,
) -> Result<Option<HhasMethod<'arena>>>
where
    F: Fn(&HhasProperty<'arena>) -> bool,
{
    if properties
        .iter()
        .any(|p: &HhasProperty| p.initializer_instrs.is_some() && filter(p))
    {
        let instrs = InstrSeq::gather(
            alloc,
            properties
                .iter()
                .filter_map(|p| {
                    if filter(p) {
                        // TODO(hrust) this clone can be avoided by wrapping initializer_instrs by Rc
                        // and also support Rc in InstrSeq
                        p.initializer_instrs
                            .as_ref()
                            .map(|i| InstrSeq::clone(alloc, i))
                    } else {
                        None
                    }
                })
                .collect(),
        );
        let instrs = InstrSeq::gather(alloc, vec![instrs, instr::null(alloc), instr::retc(alloc)]);
        Ok(Some(make_86method(
            alloc,
            emitter,
            (alloc, name).into(),
            vec![],
            true, // is_static
            Visibility::Private,
            false, // is_abstract
            span,
            instrs,
        )?))
    } else {
        Ok(None)
    }
}

pub fn emit_class<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    ast_class: &'a tast::Class_,
) -> Result<HhasClass<'a, 'arena>> {
    let namespace = &ast_class.namespace;
    validate_class_name(namespace, &ast_class.name)?;
    let mut env = Env::make_class_env(alloc, ast_class);
    // TODO: communicate this without looking at the name
    let is_closure = ast_class.name.1.starts_with("Closure$");

    let mut attributes = emit_attribute::from_asts(alloc, emitter, &ast_class.user_attributes)?;
    if !is_closure {
        attributes.extend(emit_attribute::add_reified_attribute(&ast_class.tparams).into_iter());
        attributes.extend(
            emit_attribute::add_reified_parent_attribute(&env, &ast_class.extends).into_iter(),
        )
    }

    let is_const = hhas_attribute::has_const(&attributes);
    // In the future, we intend to set class_no_dynamic_props independently from
    // class_is_const, but for now class_is_const is the only thing that turns
    // it on.
    let no_dynamic_props = is_const;
    let name = class::Type::from_ast_name(alloc, &ast_class.name.1);
    let is_trait = ast_class.kind == tast::ClassKind::Ctrait;
    let is_interface = ast_class.kind == tast::ClassKind::Cinterface;

    let uses = ast_class
        .uses
        .iter()
        .filter_map(|x| match x.1.as_ref() {
            tast::Hint_::Happly(tast::Id(_, name), _) => {
                if is_interface {
                    Some(Err(emit_fatal::raise_fatal_parse(
                        &x.0,
                        "Interfaces cannot use traits",
                    )))
                } else {
                    Some(Ok(name.as_str()))
                }
            }
            _ => None,
        })
        .collect::<Result<Vec<_>>>()?;

    let elaborate_namespace_id =
        |x: &'a tast::Id| hhbc_id::class::Type::from_ast_name(alloc, x.name());
    let use_aliases = ast_class
        .use_as_alias
        .iter()
        .map(|tast::UseAsAlias(ido1, id, ido2, vis)| {
            let id1 = ido1.as_ref().map(|x| elaborate_namespace_id(x));
            let id2 = ido2.as_ref().map(|x| (alloc, x.1.as_ref()).into());
            (id1, (alloc, id.1.as_ref()).into(), id2, vis)
        })
        .collect();

    let use_precedences = ast_class
        .insteadof_alias
        .iter()
        .map(|tast::InsteadofAlias(id1, id2, ids)| {
            let id1 = elaborate_namespace_id(id1);
            let id2: hhbc_by_ref_hhbc_id::class::Type<'arena> = (alloc, id2.1.as_ref()).into();
            let ids = ids.iter().map(|x| elaborate_namespace_id(x)).collect();
            (id1, id2, ids)
        })
        .collect();

    let enum_type = if ast_class.kind == tast::ClassKind::Cenum {
        from_enum_type(ast_class.enum_.as_ref())?
    } else {
        None
    };
    let is_enum_class = if ast_class.kind == tast::ClassKind::Cenum {
        match &ast_class.enum_ {
            Some(info) => info.enum_class,
            None => false,
        }
    } else {
        false
    };
    let xhp_attributes: Vec<_> = ast_class
        .xhp_attrs
        .iter()
        .map(
            |tast::XhpAttr(type_, class_var, tag, maybe_enum)| HhasXhpAttribute {
                type_: type_.1.as_ref(),
                class_var,
                tag: *tag,
                maybe_enum: maybe_enum.as_ref(),
            },
        )
        .collect();

    let xhp_children = ast_class.xhp_children.first().map(|(p, sl)| (p, vec![sl]));
    let xhp_categories: Option<(_, Vec<_>)> = ast_class
        .xhp_category
        .as_ref()
        .map(|(p, c)| (p, c.iter().map(|x| &x.1).collect()));

    let is_abstract = ast_class.kind == tast::ClassKind::Cabstract;
    let is_final = ast_class.final_ || is_trait;
    let is_sealed = hhas_attribute::has_sealed(&attributes);

    let tparams: Vec<&str> = ast_class
        .tparams
        .iter()
        .map(|x| x.name.1.as_ref())
        .collect();

    let base = if is_interface {
        None
    } else {
        from_extends(
            alloc,
            enum_type.is_some(),
            is_enum_class,
            &ast_class.extends,
        )
    };

    if !is_closure
        && base.as_ref().map_or(false, |cls| {
            cls.to_raw_string().eq_ignore_ascii_case("closure")
        })
    {
        return Err(emit_fatal::raise_fatal_runtime(
            &ast_class.name.0,
            "Class cannot extend Closure",
        ));
    }
    let implements = if is_interface {
        &ast_class.extends
    } else {
        &ast_class.implements
    };
    let implements = from_implements(alloc, implements);
    let enum_includes = if ast_class.kind == tast::ClassKind::Cenum {
        match &ast_class.enum_ {
            None => vec![],
            Some(enum_) => from_includes(alloc, &enum_.includes),
        }
    } else {
        vec![]
    };
    let span = Span::from_pos(&ast_class.span);
    let mut additional_methods: Vec<HhasMethod<'arena>> = vec![];
    if let Some(cats) = xhp_categories {
        additional_methods.push(emit_xhp::from_category_declaration(
            alloc, emitter, ast_class, &cats,
        )?)
    }
    if let Some(children) = xhp_children {
        additional_methods.push(emit_xhp::from_children_declaration(
            alloc, emitter, ast_class, &children,
        )?)
    }
    let no_xhp_attributes = xhp_attributes.is_empty() && ast_class.xhp_attr_uses.is_empty();
    if !no_xhp_attributes {
        additional_methods.push(emit_xhp::from_attribute_declaration(
            alloc,
            emitter,
            ast_class,
            &xhp_attributes,
            &ast_class.xhp_attr_uses,
        )?)
    }
    emitter.label_gen_mut().reset();
    let mut properties = from_class_elt_classvars(alloc, emitter, &ast_class, is_const, &tparams)?;
    let constants = from_class_elt_constants(emitter, &env, ast_class)?;

    let requirements = from_class_elt_requirements(alloc, ast_class);

    let pinit_filter = |p: &HhasProperty| !p.is_static();
    let sinit_filter = |p: &HhasProperty| p.is_static() && !p.is_lsb();
    let linit_filter = |p: &HhasProperty| p.is_static() && p.is_lsb();

    let pinit_method =
        make_init_method(alloc, emitter, &properties, &pinit_filter, "86pinit", span)?;
    let sinit_method =
        make_init_method(alloc, emitter, &properties, &sinit_filter, "86sinit", span)?;
    let linit_method =
        make_init_method(alloc, emitter, &properties, &linit_filter, "86linit", span)?;

    let initialized_constants: Vec<_> = constants
        .iter()
        .filter_map(
            |
                HhasConstant {
                    ref name,
                    ref initializer_instrs,
                    ..
                },
            | {
                initializer_instrs
                    .as_ref()
                    .map(|instrs| (name, emitter.label_gen_mut().next_regular(alloc), instrs))
            },
        )
        .collect();
    let cinit_method = if initialized_constants.is_empty() {
        None
    } else {
        fn make_cinit_instrs<'arena>(
            alloc: &'arena bumpalo::Bump,
            e: &mut Emitter<'arena>,
            default_label: label::Label<'arena>,
            pos: &Pos,
            consts: &[(
                &r#const::Type<'arena>,
                label::Label<'arena>,
                &InstrSeq<'arena>,
            )],
        ) -> InstrSeq<'arena> {
            match consts {
                [] => InstrSeq::gather(
                    alloc,
                    vec![
                        instr::label(alloc, default_label),
                        emit_pos::emit_pos(alloc, pos),
                        instr::string(alloc, "Could not find initializer for "),
                        instr::cgetl(alloc, hhbc_by_ref_local::Type::Named("$constName")),
                        instr::string(alloc, " in 86cinit"),
                        instr::concatn(alloc, 3),
                        instr::fatal(alloc, FatalOp::Runtime),
                    ],
                ),
                [(_, label, instrs), cs @ ..] => InstrSeq::gather(
                    alloc,
                    vec![
                        instr::label(alloc, *label),
                        InstrSeq::clone(alloc, *instrs),
                        emit_pos::emit_pos(alloc, pos),
                        instr::retc(alloc),
                        make_cinit_instrs(alloc, e, default_label, pos, cs),
                    ],
                ),
            }
        }
        let default_label = emitter.label_gen_mut().next_regular(alloc);

        let body_instrs = {
            let mut cases =
                bumpalo::collections::Vec::with_capacity_in(initialized_constants.len() + 1, alloc);
            for (name, label, _) in &initialized_constants {
                let n: &str = alloc.alloc_str((*name).to_raw_string());
                cases.push((n, *label))
            }
            cases.push((alloc.alloc_str("default"), default_label));
            InstrSeq::gather(
                alloc,
                vec![
                    instr::cgetl(alloc, hhbc_by_ref_local::Type::Named("$constName")),
                    instr::sswitch(alloc, cases),
                    make_cinit_instrs(
                        alloc,
                        emitter,
                        default_label,
                        &ast_class.span,
                        &initialized_constants[..],
                    ),
                ],
            )
        };
        let instrs = emit_pos::emit_pos_then(alloc, &ast_class.span, body_instrs);
        let params = vec![HhasParam {
            name: "$constName".to_string(),
            is_variadic: false,
            is_inout: false,
            user_attributes: vec![],
            type_info: None,
            default_value: None,
        }];

        Some(make_86method(
            alloc,
            emitter,
            (alloc, "86cinit").into(),
            params,
            true, /* is_static */
            Visibility::Private,
            is_interface, /* is_abstract */
            span,
            instrs,
        )?)
    };

    let should_emit_reified_init = !(emitter.systemlib() || is_closure || is_interface || is_trait);
    let reified_init_method = if should_emit_reified_init {
        emit_reified_init_method(emitter, &env, ast_class)?
    } else {
        None
    };
    let needs_no_reifiedinit = reified_init_method.is_some() && ast_class.extends.is_empty();
    additional_methods.extend(reified_init_method.into_iter());
    additional_methods.extend(pinit_method.into_iter());
    additional_methods.extend(sinit_method.into_iter());
    additional_methods.extend(linit_method.into_iter());
    additional_methods.extend(cinit_method.into_iter());

    let mut methods = emit_method::from_asts(alloc, emitter, ast_class, &ast_class.methods)?;
    methods.extend(additional_methods.into_iter());
    let (ctxconsts, tconsts): (Vec<_>, Vec<_>) =
        ast_class.typeconsts.iter().partition(|x| x.is_ctx);
    let type_constants = tconsts
        .iter()
        .map(|x| from_type_constant(alloc, emitter, x))
        .collect::<Result<Vec<HhasTypeConstant>>>()?;
    let ctx_constants = ctxconsts
        .iter()
        .map(|x| from_ctx_constant(x))
        .collect::<Result<Vec<HhasCtxConstant>>>()?;
    let upper_bounds = emit_body::emit_generics_upper_bounds(alloc, &ast_class.tparams, &[], false);

    if !no_xhp_attributes {
        properties.extend(emit_xhp::properties_for_cache(
            alloc, emitter, ast_class, is_const,
        )?);
    }
    let info = emit_memoize_method::make_info(ast_class, name, &ast_class.methods)?;
    methods.extend(emit_memoize_method::emit_wrapper_methods(
        emitter,
        &mut env,
        &info,
        ast_class,
        &ast_class.methods,
    )?);
    let doc_comment = ast_class.doc_comment.clone();
    let is_xhp = ast_class.is_xhp || ast_class.has_xhp_keyword;

    let mut flags = HhasClassFlags::empty();
    flags.set(HhasClassFlags::IS_FINAL, is_final);
    flags.set(HhasClassFlags::IS_SEALED, is_sealed);
    flags.set(HhasClassFlags::IS_ABSTRACT, is_abstract);
    flags.set(HhasClassFlags::IS_INTERFACE, is_interface);
    flags.set(HhasClassFlags::IS_TRAIT, is_trait);
    flags.set(HhasClassFlags::IS_XHP, is_xhp);
    flags.set(HhasClassFlags::IS_CONST, is_const);
    flags.set(HhasClassFlags::NO_DYNAMIC_PROPS, no_dynamic_props);
    flags.set(HhasClassFlags::NEEDS_NO_REIFIEDINIT, needs_no_reifiedinit);

    add_symbol_refs(
        alloc,
        emitter,
        &base,
        &implements,
        uses.as_ref(),
        &requirements,
    );
    Ok(HhasClass {
        attributes,
        base,
        implements,
        enum_includes,
        name,
        span,
        flags,
        doc_comment,
        uses,
        use_aliases,
        use_precedences,
        methods,
        enum_type,
        upper_bounds,
        properties,
        requirements,
        type_constants,
        ctx_constants,
        constants,
    })
}

pub fn emit_classes_from_program<'a, 'arena>(
    alloc: &'arena bumpalo::Bump,
    emitter: &mut Emitter<'arena>,
    tast: &'a [tast::Def],
) -> Result<Vec<HhasClass<'a, 'arena>>> {
    tast.iter()
        .filter_map(|class| {
            if let tast::Def::Class(cd) = class {
                Some(emit_class(alloc, emitter, cd))
            } else {
                None
            }
        })
        .collect()
}
