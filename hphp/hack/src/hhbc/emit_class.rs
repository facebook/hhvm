// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ast_constant_folder_rust as ast_constant_folder;
use closure_convert_rust as closure_convert;
use emit_attribute_rust as emit_attribute;
use emit_body_rust as emit_body;
use emit_expression_rust as emit_expression;
use emit_fatal_rust as emit_fatal;
use emit_memoize_method_rust as emit_memoize_method;
use emit_method_rust as emit_method;
use emit_property_rust as emit_property;
use emit_symbol_refs_rust as emit_symbol_refs;
use emit_type_constant_rust as emit_type_constant;
use emit_type_hint_rust as emit_type_hint;
use emit_xhp_rust as emit_xhp;
use env::{emitter::Emitter, Env};
use hhas_attribute_rust as hhas_attribute;
use hhas_class_rust::{HhasClass, HhasClassFlags, TraitReqKind};
use hhas_constant_rust::HhasConstant;
use hhas_method_rust::{HhasMethod, HhasMethodFlags};
use hhas_param_rust::HhasParam;
use hhas_pos_rust::Span;
use hhas_property_rust::HhasProperty;
use hhas_type_const::HhasTypeConstant;
use hhas_xhp_attribute_rust::HhasXhpAttribute;
use hhbc_id_rust::{self as hhbc_id, class, method, Id};
use hhbc_string_utils_rust as string_utils;
use instruction_sequence_rust::{Error::Unrecoverable, InstrSeq, Result};
use naming_special_names_rust as special_names;
use oxidized::{
    ast::{self as tast, Hint, ReifyKind, Visibility},
    namespace_env,
};
use runtime::TypedValue;
use rx_rust as rx;

use std::collections::BTreeMap;

fn add_symbol_refs(
    emitter: &mut Emitter,
    base: &Option<class::Type>,
    implements: &Vec<class::Type>,
    uses: &Vec<class::Type>,
    requirements: &Vec<(class::Type, TraitReqKind)>,
) {
    base.iter()
        .for_each(|x| emit_symbol_refs::State::add_class(emitter, x.clone()));
    implements
        .iter()
        .for_each(|x| emit_symbol_refs::State::add_class(emitter, x.clone()));
    uses.iter()
        .for_each(|x| emit_symbol_refs::State::add_class(emitter, x.clone()));
    requirements
        .iter()
        .for_each(|(x, _)| emit_symbol_refs::State::add_class(emitter, x.clone()));
}

fn make_86method<'a>(
    emitter: &mut Emitter,
    name: method::Type<'a>,
    params: Vec<HhasParam>,
    is_static: bool,
    visibility: Visibility,
    is_abstract: bool,
    span: Span,
    instrs: InstrSeq,
) -> HhasMethod<'a> {
    // TODO: move this. We just know that there are no iterators in 86methods
    emitter.iterator_mut().reset();

    let mut flags = HhasMethodFlags::empty();
    flags.set(HhasMethodFlags::NO_INJECTION, true);
    flags.set(HhasMethodFlags::IS_ABSTRACT, is_abstract);
    flags.set(HhasMethodFlags::IS_STATIC, is_static);

    let attributes = vec![];
    let rx_level = rx::Level::NonRx;

    let method_decl_vars = vec![];
    let method_return_type = None;
    let method_doc_comment = None;
    let method_is_memoize_wrapper = false;
    let method_is_memoize_wrapper_lsb = false;
    let method_env = None;

    let body = emit_body::make_body(
        emitter,
        instrs,
        method_decl_vars,
        method_is_memoize_wrapper,
        method_is_memoize_wrapper_lsb,
        vec![],
        params,
        method_return_type,
        method_doc_comment,
        method_env,
    );

    HhasMethod {
        body,
        attributes,
        name,
        flags,
        span,
        rx_level,
        visibility,
    }
}

fn from_constant<'a>(
    emitter: &mut Emitter,
    env: &Env,
    id: &'a tast::Id,
    expr: &Option<tast::Expr>,
) -> Result<HhasConstant<'a>> {
    let (value, initializer_instrs) = match expr {
        None => (None, None),
        Some(init) => {
            match ast_constant_folder::expr_to_typed_value(emitter, &env.namespace, init) {
                Ok(v) => (Some(v), None),
                Err(_) => (
                    Some(TypedValue::Uninit),
                    Some(emit_expression::emit_expr(emitter, env, init)?),
                ),
            }
        }
    };
    Ok(HhasConstant {
        name: id.name().into(),
        value,
        initializer_instrs,
    })
}

fn from_extends(is_enum: bool, extends: &Vec<tast::Hint>) -> Option<hhbc_id::class::Type> {
    if is_enum {
        Some(hhbc_id::class::from_raw_string("HH\\BuiltinEnum"))
    } else {
        extends.first().map(|x| emit_type_hint::hint_to_class(x))
    }
}

fn from_implements(implements: &Vec<tast::Hint>) -> Vec<hhbc_id::class::Type> {
    implements
        .iter()
        .map(|x| emit_type_hint::hint_to_class(x))
        .collect()
}

fn from_type_constant<'a>(
    emitter: &mut Emitter,
    tc: &'a tast::ClassTypeconst,
) -> Result<HhasTypeConstant> {
    use tast::TypeconstAbstractKind::*;
    let name = tc.name.1.to_string();

    let initializer = match (&tc.abstract_, &tc.type_) {
        (TCAbstract(None), _) | (TCPartiallyAbstract, None) | (TCConcrete, None) => None,
        (TCAbstract(Some(init)), _)
        | (TCPartiallyAbstract, Some(init))
        | (TCConcrete, Some(init)) => {
            // TODO: Deal with the constraint
            // Type constants do not take type vars hence tparams:[]
            Some(emit_type_constant::hint_to_type_constant(
                emitter.options(),
                &vec![],
                &BTreeMap::new(),
                init,
                false,
                false,
            )?)
        }
    };

    Ok(HhasTypeConstant { name, initializer })
}

fn from_class_elt_classvars<'a>(
    emitter: &mut Emitter,
    namespace: &namespace_env::Env,
    ast_class: &'a tast::Class_,
    class_is_const: bool,
    tparams: &[&str],
) -> Result<Vec<HhasProperty<'a>>> {
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
                emitter,
                ast_class,
                namespace,
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
                },
            )
        })
        .collect::<Result<Vec<_>>>()
}

fn from_class_elt_constants<'a>(
    emitter: &mut Emitter,
    env: &Env,
    class_: &'a tast::Class_,
) -> Result<Vec<HhasConstant<'a>>> {
    class_
        .consts
        .iter()
        .map(|x| from_constant(emitter, env, &x.id, &x.expr))
        .collect()
}

fn from_class_elt_requirements<'a>(
    class_: &'a tast::Class_,
) -> Vec<(hhbc_id::class::Type, TraitReqKind)> {
    class_
        .reqs
        .iter()
        .map(|(h, is_extends)| {
            let kind = if *is_extends {
                TraitReqKind::MustExtend
            } else {
                TraitReqKind::MustImplement
            };
            (emit_type_hint::hint_to_class(h), kind)
        })
        .collect()
}

fn from_class_elt_typeconsts<'a>(
    emitter: &mut Emitter,
    class_: &'a tast::Class_,
) -> Result<Vec<HhasTypeConstant>> {
    class_
        .typeconsts
        .iter()
        .map(|x| from_type_constant(emitter, x))
        .collect()
}

fn from_enum_type(opt: Option<&tast::Enum_>) -> Result<Option<hhas_type::Info>> {
    use hhas_type::constraint::*;
    opt.map(|e| {
        let type_info_user_type = Some(emit_type_hint::fmt_hint(&[], true, &e.base)?);
        let type_info_type_constraint = Type::make(None, Flags::EXTENDED_HINT);
        Ok(hhas_type::Info::make(
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
            .map(|x| x.eq_ignore_ascii_case("hh"))
            .unwrap_or(false)
    };

    // global names are always reserved in any namespace.
    // hh_reserved names are checked either if
    // - class is in global namespace
    // - class is in HH namespace *)
    let is_special_class = class_name.contains("$");
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
                    name
                } else {
                    string_utils::strip_ns(class_name)
                }
            ),
        ))
    } else {
        Ok(())
    }
}

fn emit_reified_init_body<'a>(
    _env: &Env,
    _num_reified: usize,
    _ast_class: &'a tast::Class_,
) -> InstrSeq {
    // TODO(hrust)
    InstrSeq::default()
}

fn emit_reified_init_method<'a>(
    emitter: &mut Emitter,
    env: &Env,
    ast_class: &'a tast::Class_,
) -> Option<HhasMethod<'a>> {
    use hhas_type::{constraint::*, Info};

    let num_reified = ast_class
        .tparams
        .list
        .iter()
        .filter(|x| x.reified != ReifyKind::Erased)
        .count();
    let maybe_has_reified_parents = match ast_class.extends.first().as_ref() {
        Some(Hint(_, h)) if h.as_happly().map(|(_, l)| !l.is_empty()).unwrap_or(false) => true,
        _ => false, // Hack classes can only extend a single parent
    };
    if num_reified == 0 && !maybe_has_reified_parents {
        None
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

        let instrs = emit_reified_init_body(env, num_reified, ast_class);
        Some(make_86method(
            emitter,
            string_utils::reified::INIT_METH_NAME.into(),
            params,
            false, // is_static
            Visibility::Protected,
            false, // is_abstract
            ast_class.span.clone().into(),
            instrs,
        ))
    }
}

fn make_init_method<'a, F>(
    emitter: &mut Emitter,
    properties: &[HhasProperty],
    filter: F,
    name: &'static str,
    span: Span,
) -> Option<HhasMethod<'a>>
where
    F: Fn(&HhasProperty) -> bool,
{
    if properties
        .iter()
        .any(|p: &HhasProperty| p.initializer_instrs.is_some() && filter(p))
    {
        // TODO(hrust)
        let instrs = InstrSeq::default();
        Some(make_86method(
            emitter,
            name.into(),
            vec![],
            true, // is_static
            Visibility::Private,
            false, // is_abstract
            span,
            instrs,
        ))
    } else {
        None
    }
}

pub fn emit_class<'a>(
    emitter: &mut Emitter,
    ast_class: &'a tast::Class_,
    hoisted: closure_convert::HoistKind,
) -> Result<HhasClass<'a>> {
    let namespace = &ast_class.namespace;
    validate_class_name(namespace, &ast_class.name)?;
    let mut env = Env::make_class_env(ast_class);
    // TODO: communicate this without looking at the name
    let is_closure = ast_class.name.1.starts_with("Closure$");

    let mut attributes = emit_attribute::from_asts(emitter, namespace, &ast_class.user_attributes)?;
    if !is_closure {
        attributes
            .extend(emit_attribute::add_reified_attribute(&ast_class.tparams.list).into_iter());
        attributes.extend(
            emit_attribute::add_reified_parent_attribute(&env, &ast_class.extends)?.into_iter(),
        )
    }

    let is_const = hhas_attribute::has_const(&attributes);
    // In the future, we intend to set class_no_dynamic_props independently from
    // class_is_const, but for now class_is_const is the only thing that turns
    // it on.
    let no_dynamic_props = is_const;
    let name = class::Type::from_ast_name(&ast_class.name.1);
    let is_trait = ast_class.kind == tast::ClassKind::Ctrait;
    let is_interface = ast_class.kind == tast::ClassKind::Cinterface;

    let uses = ast_class
        .uses
        .iter()
        .filter_map(|x| match x.1.as_ref() {
            tast::Hint_::Happly(tast::Id(p, name), _) => {
                if is_interface {
                    Some(Err(emit_fatal_rust::raise_fatal_parse(
                        p,
                        "Interfaces cannot use traits",
                    )))
                } else {
                    Some(Ok(name.into()))
                }
            }
            _ => None,
        })
        .collect::<Result<_>>()?;

    let elaborate_namespace_id = |x: &'a tast::Id| hhbc_id::class::Type::from_ast_name(x.name());
    let use_aliases = ast_class
        .use_as_alias
        .iter()
        .map(|tast::UseAsAlias(ido1, id, ido2, vis)| {
            let id1 = ido1.as_ref().map(|x| elaborate_namespace_id(x));
            let id2 = ido2.as_ref().map(|x| (&x.1).into());
            (id1, (&id.1).into(), id2, vis)
        })
        .collect();

    let use_precedences = ast_class
        .insteadof_alias
        .iter()
        .map(|tast::InsteadofAlias(id1, id2, ids)| {
            let id1 = elaborate_namespace_id(&id1);
            let id2 = &id2.1;
            let ids = ids.iter().map(|x| elaborate_namespace_id(x)).collect();
            (id1, id2.into(), ids)
        })
        .collect();
    let string_of_trait = |trait_: &'a tast::Hint| {
        use tast::Hint_::*;
        match trait_.1.as_ref() {
            // TODO: Currently, names are not elaborated.
            // Names should be elaborated if this feature is to be supported
            // T56629465
            Happly(tast::Id(_, trait_), _) => Ok(trait_.into()),
            // Happly converted from naming
            Hprim(p) => Ok(emit_type_hint::prim_to_string(p).into()),
            Hany | Herr => Err(Unrecoverable(
                "I'm convinced that this should be an error caught in naming".into(),
            )),
            Hmixed => Ok(special_names::typehints::MIXED.into()),
            Hnonnull => Ok(special_names::typehints::NONNULL.into()),
            Habstr(s) => Ok(s.into()),
            Harray(_, _) => Ok(special_names::typehints::ARRAY.into()),
            Hdarray(_, _) => Ok(special_names::typehints::DARRAY.into()),
            Hvarray(_) => Ok(special_names::typehints::VARRAY.into()),
            HvarrayOrDarray(_, _) => Ok(special_names::typehints::VARRAY_OR_DARRAY.into()),
            Hthis => Ok(special_names::typehints::THIS.into()),
            Hdynamic => Ok(special_names::typehints::DYNAMIC.into()),
            _ => Err(Unrecoverable("TODO Fail gracefully here".into())),
        }
    };
    let method_trait_resolutions: Vec<(_, class::Type)> = ast_class
        .method_redeclarations
        .iter()
        .map(|mtr| Ok((mtr, string_of_trait(&mtr.trait_)?)))
        .collect::<Result<_>>()?;

    let enum_type = if ast_class.kind == tast::ClassKind::Cenum {
        from_enum_type(ast_class.enum_.as_ref())?
    } else {
        None
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
    let is_final = ast_class.final_ || is_trait || enum_type.is_some();
    let is_sealed = hhas_attribute::has_sealed(&attributes);

    let tparams: Vec<&str> = ast_class
        .tparams
        .list
        .iter()
        .map(|x| x.name.1.as_ref())
        .collect();

    let base = if is_interface {
        None
    } else {
        from_extends(enum_type.is_some(), &ast_class.extends)
    };

    if base
        .as_ref()
        .map(|cls| cls.to_raw_string().eq_ignore_ascii_case("closure") && !is_closure)
        .unwrap_or(false)
    {
        Err(emit_fatal::raise_fatal_runtime(
            &ast_class.name.0,
            "Class cannot extend Closure".to_string(),
        ))?;
    }
    let implements = if is_interface {
        &ast_class.extends
    } else {
        &ast_class.implements
    };
    let implements = from_implements(implements);
    let span: Span = ast_class.span.clone().into();
    let mut additional_methods: Vec<HhasMethod> = vec![];
    if let Some(cats) = xhp_categories {
        additional_methods.extend(emit_xhp::from_category_declaration(ast_class, &cats)?)
    }
    if let Some(children) = xhp_children {
        additional_methods.extend(emit_xhp::from_children_declaration(ast_class, &children)?)
    }
    let no_xhp_attributes = xhp_attributes.is_empty() && ast_class.xhp_attr_uses.is_empty();
    if !no_xhp_attributes {
        additional_methods.extend(emit_xhp::from_attribute_declaration(
            ast_class,
            &xhp_attributes,
            &ast_class.xhp_attr_uses,
        )?)
    }
    emitter.label_gen_mut().reset();
    let mut properties =
        from_class_elt_classvars(emitter, namespace, &ast_class, is_const, &tparams)?;
    let constants = from_class_elt_constants(emitter, &env, ast_class)?;

    let requirements = from_class_elt_requirements(ast_class);

    let pinit_filter = |p: &HhasProperty| !p.is_static();
    let sinit_filter = |p: &HhasProperty| p.is_static() && !p.is_lsb();
    let linit_filter = |p: &HhasProperty| p.is_static() && p.is_lsb();

    let pinit_method =
        make_init_method(emitter, &properties, &pinit_filter, "86pinit", span.clone());
    let sinit_method =
        make_init_method(emitter, &properties, &sinit_filter, "86sinit", span.clone());
    let linit_method =
        make_init_method(emitter, &properties, &linit_filter, "86linit", span.clone());

    let initialized_constants: Vec<_> = constants
        .iter()
        .filter_map(|p| {
            p.initializer_instrs
                .as_ref()
                .map(|instrs| Some((&p.name, instrs)))
        })
        .collect();
    let cinit_method = if initialized_constants.is_empty() {
        None
    } else {
        let instrs = InstrSeq::default(); // TODO(hrust)
        let params = vec![HhasParam {
            name: "$constName".to_string(),
            is_variadic: false,
            is_inout: false,
            user_attributes: vec![],
            type_info: None,
            default_value: None,
        }];

        Some(make_86method(
            emitter,
            "86cinit".into(),
            params,
            true, /* is_static */
            Visibility::Private,
            is_interface, /* is_abstract */
            span.clone(),
            instrs,
        ))
    };

    let should_emit_reified_init =
        !(emitter.context().systemlib() || is_closure || is_interface || is_trait);
    let reified_init_method = if should_emit_reified_init {
        emit_reified_init_method(emitter, &env, ast_class)
    } else {
        None
    };
    let needs_no_reifiedinit = reified_init_method.is_some() && ast_class.extends.is_empty();
    additional_methods.extend(reified_init_method.into_iter());
    additional_methods.extend(pinit_method.into_iter());
    additional_methods.extend(sinit_method.into_iter());
    additional_methods.extend(linit_method.into_iter());
    additional_methods.extend(cinit_method.into_iter());

    let mut methods = emit_method::from_asts(emitter, ast_class, &ast_class.methods)?;
    methods.extend(additional_methods.into_iter());
    let type_constants = from_class_elt_typeconsts(emitter, ast_class)?;
    let upper_bounds = if emitter.options().enforce_generic_ub() {
        emit_body::emit_generics_upper_bounds(&ast_class.tparams.list, false)
    } else {
        vec![]
    };

    if !no_xhp_attributes {
        properties.extend(emit_xhp::properties_for_cache(
            namespace, ast_class, is_const,
        )?);
    }
    let info = emit_memoize_method::make_info(ast_class, name.clone(), &ast_class.methods)?;
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

    add_symbol_refs(emitter, &base, &implements, &uses, &requirements);
    Ok(HhasClass {
        attributes,
        base,
        implements,
        name,
        span,
        flags,
        doc_comment,
        uses,
        use_aliases,
        use_precedences,
        method_trait_resolutions,
        methods,
        enum_type,
        hoisted,
        upper_bounds,
        properties,
        requirements,
        type_constants,
        constants,
    })
}

pub fn emit_classes_from_program<'a>(
    emitter: &mut Emitter,
    tast: &'a tast::Program,
) -> Result<Vec<HhasClass<'a>>> {
    tast.iter()
        .filter_map(|x| {
            if let tast::Def::Class(cd) = x {
                Some(emit_class(
                    emitter,
                    cd,
                    // TODO(hrust): pass the real hoist kind
                    closure_convert::HoistKind::TopLevel,
                ))
            } else {
                None
            }
        })
        .collect()
}
