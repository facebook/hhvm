// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::collections::BTreeMap;

use emit_property::PropAndInit;
use env::Env;
use env::emitter::Emitter;
use error::Error;
use error::Result;
use ffi::Maybe;
use ffi::Maybe::*;
use hhbc::Class;
use hhbc::ClassName;
use hhbc::Coeffects;
use hhbc::Constant;
use hhbc::CtxConstant;
use hhbc::FCallArgs;
use hhbc::FCallArgsFlags;
use hhbc::FatalOp;
use hhbc::Local;
use hhbc::Method;
use hhbc::MethodFlags;
use hhbc::MethodName;
use hhbc::Param;
use hhbc::PropName;
use hhbc::Property;
use hhbc::ReadonlyOp;
use hhbc::Requirement;
use hhbc::Span;
use hhbc::SpecialClsRef;
use hhbc::TraitReqKind;
use hhbc::TypeConstant;
use hhbc::TypeInfo;
use hhbc::TypedValue;
use hhbc::Visibility;
use hhbc::string_id;
use hhbc_string_utils as string_utils;
use hhvm_types_ffi::ffi::Attr;
use hhvm_types_ffi::ffi::TypeConstraintFlags;
use instruction_sequence::InstrSeq;
use instruction_sequence::instr;
use itertools::Itertools;
use oxidized::ast;
use oxidized::ast::ClassReq;
use oxidized::ast::Hint;
use oxidized::ast::ReifyKind;
use oxidized::ast::RequireKind;

use super::TypeRefinementInHint;
use crate::emit_adata;
use crate::emit_attribute;
use crate::emit_body;
use crate::emit_constant;
use crate::emit_expression;
use crate::emit_memoize_method;
use crate::emit_method;
use crate::emit_property;
use crate::emit_type_constant;
use crate::emit_xhp;
use crate::xhp_attribute::XhpAttribute;

fn add_symbol_refs<'d>(
    emitter: &mut Emitter,
    base: Option<&ClassName>,
    implements: &[ClassName],
    uses: &[ClassName],
    requirements: &[hhbc::Requirement],
    enum_includes: &[ClassName],
) {
    base.iter().for_each(|&x| emitter.add_class_ref(*x));
    implements.iter().for_each(|x| emitter.add_class_ref(*x));
    uses.iter().for_each(|x| emitter.add_class_ref(*x));
    requirements
        .iter()
        .for_each(|r| emitter.add_class_ref(r.name));
    enum_includes.iter().for_each(|x| emitter.add_class_ref(*x));
}

fn make_86method<'d>(
    emitter: &mut Emitter,
    name: MethodName,
    params: Vec<Param>,
    is_static: bool,
    visibility: Visibility,
    is_abstract: bool,
    span: Span,
    coeffects: Coeffects,
    instrs: InstrSeq,
) -> Result<Method> {
    // TODO: move this. We just know that there are no iterators in 86methods
    emitter.iterator_mut().reset();

    let mut attrs = Attr::AttrNone;
    attrs.add(Attr::AttrNoInjection);
    attrs.set(Attr::AttrAbstract, is_abstract);
    attrs.set(Attr::AttrStatic, is_static);
    attrs.set(Attr::AttrBuiltin, emitter.systemlib());
    attrs.set(Attr::AttrPersistent, emitter.systemlib());
    attrs.add(Attr::from(visibility));

    let flags = MethodFlags::empty();
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
        vec![], // upper_bounds
        vec![], // tparam_info
        vec![], // attributes
        attrs,
        coeffects,
        params.into_iter().map(|p| (p, None)).collect::<Vec<_>>(),
        method_return_type,
        method_doc_comment,
        method_env,
        span,
    )?;

    Ok(Method {
        body,
        name,
        flags,
        visibility,
    })
}

fn from_extends(
    is_enum: bool,
    is_enum_class: bool,
    is_abstract: bool,
    extends: &[ast::Hint],
) -> Option<ClassName> {
    if is_enum {
        // Do not use special_names:: as there's a prefix \ which breaks HHVM
        if is_enum_class {
            if is_abstract {
                Some(ClassName::new(string_id!("HH\\BuiltinAbstractEnumClass")))
            } else {
                Some(ClassName::new(string_id!("HH\\BuiltinEnumClass")))
            }
        } else {
            Some(ClassName::new(string_id!("HH\\BuiltinEnum")))
        }
    } else {
        extends.first().map(emit_type_hint::hint_to_class)
    }
}

fn from_implements(implements: &[ast::Hint]) -> Vec<ClassName> {
    implements
        .iter()
        .map(emit_type_hint::hint_to_class)
        .collect()
}

fn from_includes(includes: &[ast::Hint]) -> Vec<ClassName> {
    includes.iter().map(emit_type_hint::hint_to_class).collect()
}

fn from_type_constant<'a, 'd>(
    emitter: &mut Emitter,
    tc: &'a ast::ClassTypeconstDef,
) -> Result<TypeConstant> {
    use ast::ClassTypeconst;
    let name = tc.name.1.to_string();

    let initializer = match &tc.kind {
        ClassTypeconst::TCAbstract(ast::ClassAbstractTypeconst { default: None, .. }) => None,
        ClassTypeconst::TCAbstract(ast::ClassAbstractTypeconst {
            default: Some(init),
            ..
        })
        | ClassTypeconst::TCConcrete(ast::ClassConcreteTypeconst { c_tc_type: init }) => {
            // TODO: Deal with the constraint
            // Type constants do not take type vars hence tparams:[]
            Some(emit_type_constant::hint_to_type_constant(
                emitter.options(),
                &[],
                &BTreeMap::new(),
                init,
                TypeRefinementInHint::Disallowed,
            )?)
        }
    };

    let is_abstract = match &tc.kind {
        ClassTypeconst::TCConcrete(_) => false,
        _ => true,
    };

    Ok(TypeConstant {
        name: hhbc::intern(name),
        initializer: Maybe::from(initializer),
        is_abstract,
    })
}

fn from_ctx_constant(tc: &ast::ClassTypeconstDef) -> Result<CtxConstant> {
    use ast::ClassTypeconst;
    let ast::Id(_, name) = &tc.name;
    let (recognized, unrecognized) = match &tc.kind {
        ClassTypeconst::TCAbstract(ast::ClassAbstractTypeconst { default: None, .. }) => {
            (vec![], vec![])
        }
        ClassTypeconst::TCAbstract(ast::ClassAbstractTypeconst {
            default: Some(hint),
            ..
        })
        | ClassTypeconst::TCConcrete(ast::ClassConcreteTypeconst { c_tc_type: hint }) => {
            let (static_co, unenforced) = Coeffects::from_ctx_constant(hint);
            let r: Vec<_> = static_co
                .iter()
                .map(|ctx| hhbc::intern(ctx.to_string()))
                .collect();
            let u: Vec<_> = unenforced.iter().map(hhbc::intern).collect();
            (r, u)
        }
    };
    let is_abstract = match &tc.kind {
        ClassTypeconst::TCConcrete(_) => false,
        _ => true,
    };
    Ok(CtxConstant {
        name: hhbc::intern(name),
        recognized: recognized.into(),
        unrecognized: unrecognized.into(),
        is_abstract,
    })
}

fn from_class_elt_classvars<'a, 'd>(
    emitter: &mut Emitter,
    ast_class: &'a ast::Class_,
    class_is_const: bool,
    tparams: &[&str],
    is_closure: bool,
) -> Result<Vec<PropAndInit>> {
    // TODO: we need to emit doc comments for each property,
    // not one per all properties on the same line
    // The doc comment is only for the first name in the list.
    // Currently this is organized in the ast_to_nast module
    ast_class
        .vars
        .iter()
        .map(|cv| {
            emit_property::from_ast(
                emitter,
                ast_class,
                tparams,
                class_is_const,
                is_closure,
                emit_property::FromAstArgs {
                    user_attributes: &cv.user_attributes,
                    id: &cv.id,
                    initial_value: &cv.expr,
                    typehint: cv.type_.1.as_ref(),
                    // Doc comments are weird. T40098274
                    doc_comment: cv.doc_comment.clone(),
                    visibility: cv.visibility, // This used to be cv_kinds
                    is_static: cv.is_static,
                    is_abstract: cv.abstract_,
                    is_readonly: cv.readonly,
                },
            )
        })
        .collect()
}

fn from_class_elt_constants<'a, 'd>(
    emitter: &mut Emitter,
    env: &Env<'a>,
    class_: &'a ast::Class_,
) -> Result<Vec<(Constant, Option<InstrSeq>)>> {
    use oxidized::aast::ClassConstKind;
    class_
        .consts
        .iter()
        .map(|x| {
            // start unnamed local numbers at 1 for constants to not clobber $constVars
            emitter.local_gen_mut().reset(Local::new(1));
            let (is_abstract, init_opt) = match &x.kind {
                ClassConstKind::CCAbstract(default) => (true, default.as_ref()),
                ClassConstKind::CCConcrete(expr) => (false, Some(expr)),
            };
            emit_constant::from_ast(emitter, env, &x.id, is_abstract, init_opt)
        })
        .collect()
}

fn from_class_elt_requirements<'a>(class_: &'a ast::Class_) -> Vec<Requirement> {
    class_
        .reqs
        .iter()
        .filter_map(|ClassReq(h, req_kind)| {
            let name = emit_type_hint::hint_to_class(h);
            let f = |kind| Some(Requirement { name, kind });
            match *req_kind {
                RequireKind::RequireExtends => f(TraitReqKind::MustExtend),
                RequireKind::RequireImplements => f(TraitReqKind::MustImplement),
                RequireKind::RequireClass => f(TraitReqKind::MustBeClass),
                RequireKind::RequireThisAs => f(TraitReqKind::MustBeAs),
            }
        })
        .collect()
}

fn from_enum_type(opt: Option<&ast::Enum_>) -> Result<Option<TypeInfo>> {
    use hhbc::Constraint;
    opt.map(|e| {
        let type_info_user_type = Just(hhbc::intern(emit_type_hint::fmt_hint(&[], true, &e.base)?));
        let type_info_type_constraint = Constraint::new(Nothing, TypeConstraintFlags::NoFlags);
        Ok(TypeInfo::new(
            type_info_user_type,
            type_info_type_constraint,
        ))
    })
    .transpose()
}

fn emit_reified_extends_params<'a, 'd>(
    e: &mut Emitter,
    env: &Env<'a>,
    ast_class: &'a ast::Class_,
) -> Result<InstrSeq> {
    match &ast_class.extends[..] {
        [h, ..] => match h.1.as_happly() {
            Some((_, l)) if !l.is_empty() => {
                return Ok(InstrSeq::gather(vec![
                    emit_expression::emit_reified_targs(e, env, &ast_class.span, l.iter())?,
                    instr::record_reified_generic(),
                ]));
            }
            _ => {}
        },
        _ => {}
    }
    let tv = TypedValue::vec(Default::default());
    emit_adata::typed_value_into_instr(e, tv)
}

pub(crate) static REIFIED_INIT_METH_NAME: hhbc::Lazy<MethodName> =
    hhbc::Lazy::new(|| MethodName::intern(string_utils::reified::INIT_METH_NAME));
pub(crate) static REIFIED_PROP_NAME: hhbc::Lazy<PropName> =
    hhbc::Lazy::new(|| PropName::intern(string_utils::reified::PROP_NAME));

fn emit_reified_init_body<'a, 'd>(
    e: &mut Emitter,
    env: &Env<'a>,
    num_reified: usize,
    ast_class: &'a ast::Class_,
    init_meth_param_local: Local,
) -> Result<InstrSeq> {
    let check_length = InstrSeq::gather(vec![
        instr::c_get_l(init_meth_param_local),
        instr::check_cls_reified_generic_mismatch(),
    ]);
    let set_prop = if num_reified == 0 {
        instr::empty()
    } else {
        InstrSeq::gather(vec![
            check_length,
            instr::check_this(),
            instr::c_get_l(init_meth_param_local),
            instr::base_h(),
            instr::set_m_pt(0, *REIFIED_PROP_NAME, ReadonlyOp::Any),
            instr::pop_c(),
        ])
    };
    let return_instr = InstrSeq::gather(vec![instr::null(), instr::ret_c()]);
    Ok(if ast_class.extends.is_empty() {
        InstrSeq::gather(vec![set_prop, return_instr])
    } else {
        let generic_arr = emit_reified_extends_params(e, env, ast_class)?;
        let call_parent = InstrSeq::gather(vec![
            instr::null_uninit(),
            instr::null_uninit(),
            generic_arr,
            instr::f_call_cls_method_sd(
                FCallArgs::new(FCallArgsFlags::default(), 1, 1, vec![], vec![], None, None),
                SpecialClsRef::ParentCls,
                *REIFIED_INIT_METH_NAME,
            ),
            instr::pop_c(),
        ]);
        InstrSeq::gather(vec![set_prop, call_parent, return_instr])
    })
}

fn emit_reified_init_method<'a, 'd>(
    emitter: &mut Emitter,
    env: &Env<'a>,
    ast_class: &'a ast::Class_,
) -> Result<Option<Method>> {
    use hhbc::Constraint;

    let num_reified = ast_class
        .tparams
        .iter()
        .filter(|x| x.reified != ReifyKind::Erased)
        .count();
    let maybe_has_reified_parents = match ast_class.extends.first().as_ref() {
        Some(Hint(_, h)) if h.as_happly().is_some_and(|(_, l)| !l.is_empty()) => true,
        _ => false, // Hack classes can only extend a single parent
    };
    if num_reified == 0 && !maybe_has_reified_parents {
        Ok(None)
    } else {
        let tc = Constraint::intern("HH\\varray", TypeConstraintFlags::NoFlags);
        let param_local = Local::new(0);
        let params = vec![Param {
            name: hhbc::intern(string_utils::reified::INIT_METH_PARAM_NAME),
            is_variadic: false,
            is_splat: false,
            is_inout: false,
            is_readonly: false,
            is_optional: false,
            user_attributes: Default::default(),
            type_info: Just(TypeInfo::new(Just(hhbc::intern("HH\\varray")), tc)),
        }];

        let instrs = emit_reified_init_body(emitter, env, num_reified, ast_class, param_local)?;
        let instrs = emit_pos::emit_pos_then(&ast_class.span, instrs);
        Ok(Some(make_86method(
            emitter,
            *REIFIED_INIT_METH_NAME,
            params,
            false, // is_static
            Visibility::Public,
            false, // is_abstract
            Span::from_pos(&ast_class.span),
            Coeffects::pure(),
            instrs,
        )?))
    }
}

fn make_init_method<'d>(
    emitter: &mut Emitter,
    properties: &mut [PropAndInit],
    filter: impl Fn(&Property) -> bool,
    name: &'static str,
    span: Span,
) -> Result<Option<Method>> {
    let mut has_inits = false;
    let instrs = InstrSeq::gather(
        properties
            .iter_mut()
            .filter_map(|p| match p.init {
                Some(_) if filter(&p.prop) => {
                    has_inits = true;
                    p.init.take()
                }
                _ => None,
            })
            .collect(),
    );
    if has_inits {
        let instrs = InstrSeq::gather(vec![instrs, instr::null(), instr::ret_c()]);
        Ok(Some(make_86method(
            emitter,
            MethodName::intern(name),
            vec![],
            true, // is_static
            Visibility::Private,
            false, // is_abstract
            span,
            Coeffects::pure(),
            instrs,
        )?))
    } else {
        Ok(None)
    }
}

pub fn emit_class<'a, 'd>(emitter: &mut Emitter, ast_class: &'a ast::Class_) -> Result<Class> {
    let mut env = Env::make_class_env(ast_class);
    // TODO: communicate this without looking at the name
    let is_closure = ast_class.name.1.starts_with("Closure$");

    let mut attributes = emit_attribute::from_asts(emitter, &ast_class.user_attributes)?;
    if !is_closure {
        attributes.extend(emit_attribute::add_reified_attribute(&ast_class.tparams));
        attributes.extend(emit_attribute::add_reified_parent_attribute(
            &env,
            &ast_class.extends,
        ))
    }

    let is_const = hhbc::has_const(attributes.as_ref());
    // In the future, we intend to set class_no_dynamic_props independently from
    // class_is_const, but for now class_is_const is the only thing that turns
    // it on.
    let no_dynamic_props = is_const;
    let name = ClassName::from_ast_name_and_mangle(&ast_class.name.1);
    let is_trait = ast_class.kind == ast::ClassishKind::Ctrait;
    let is_interface = ast_class.kind == ast::ClassishKind::Cinterface;

    let uses: Vec<ClassName> = ast_class
        .uses
        .iter()
        .filter_map(|Hint(pos, hint)| match hint.as_ref() {
            ast::Hint_::Happly(ast::Id(_, name), _) => {
                if is_interface {
                    Some(Err(Error::fatal_parse(pos, "Interfaces cannot use traits")))
                } else {
                    Some(Ok(ClassName::from_ast_name_and_mangle(name.as_str())))
                }
            }
            _ => None,
        })
        .collect::<Result<Vec<_>>>()?
        .into_iter()
        .unique()
        .collect();

    let enum_type = if ast_class.kind.is_cenum() || ast_class.kind.is_cenum_class() {
        from_enum_type(ast_class.enum_.as_ref())?
    } else {
        None
    };
    let is_enum_class = ast_class.kind.is_cenum_class();
    let xhp_attributes: Vec<_> = ast_class
        .xhp_attrs
        .iter()
        .map(
            |ast::XhpAttr(type_, class_var, tag, maybe_enum)| XhpAttribute {
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

    let is_abstract = match ast_class.kind {
        ast::ClassishKind::Cclass(k) => k.is_abstract(),
        ast::ClassishKind::CenumClass(k) => k.is_abstract(),
        _ => false,
    };
    let is_final = ast_class.final_ || is_trait;
    let is_sealed = hhbc::has_sealed(attributes.as_ref());

    let tparams: Vec<&str> = ast_class
        .tparams
        .iter()
        .map(|x| x.name.1.as_ref())
        .collect();

    let base = if is_interface {
        None
    } else {
        from_extends(
            enum_type.is_some(),
            is_enum_class,
            is_abstract,
            &ast_class.extends,
        )
    };

    let base_is_closure = || {
        base.as_ref()
            .is_some_and(|cls| cls.as_str().eq_ignore_ascii_case("Closure"))
    };
    if !is_closure && base_is_closure() {
        return Err(Error::fatal_runtime(
            &ast_class.name.0,
            "Class cannot extend Closure",
        ));
    }
    let implements = if is_interface {
        &ast_class.extends
    } else {
        &ast_class.implements
    };
    let implements = from_implements(implements);
    let enum_includes = if ast_class.kind.is_cenum() || ast_class.kind.is_cenum_class() {
        match &ast_class.enum_ {
            None => vec![],
            Some(enum_) => from_includes(&enum_.includes),
        }
    } else {
        vec![]
    };
    let span = Span::from_pos(&ast_class.span);
    let mut additional_methods: Vec<Method> = vec![];
    if let Some(cats) = xhp_categories {
        additional_methods.push(emit_xhp::from_category_declaration(
            emitter, ast_class, &cats,
        )?)
    }
    if let Some(children) = xhp_children {
        additional_methods.push(emit_xhp::from_children_declaration(
            emitter, ast_class, &children,
        )?)
    }
    let no_xhp_attributes = xhp_attributes.is_empty() && ast_class.xhp_attr_uses.is_empty();
    if !no_xhp_attributes {
        additional_methods.push(emit_xhp::from_attribute_declaration(
            emitter,
            ast_class,
            &xhp_attributes,
            &ast_class.xhp_attr_uses,
        )?)
    }
    emitter.label_gen_mut().reset();
    let mut properties =
        from_class_elt_classvars(emitter, ast_class, is_const, &tparams, is_closure)?;
    let mut constants = from_class_elt_constants(emitter, &env, ast_class)?;

    let requirements = from_class_elt_requirements(ast_class);

    let pinit_method = make_init_method(
        emitter,
        &mut properties,
        |p| !p.flags.is_static(),
        "86pinit",
        span,
    )?;
    let sinit_method = make_init_method(
        emitter,
        &mut properties,
        |p| p.flags.is_static() && !p.flags.is_lsb(),
        "86sinit",
        span,
    )?;
    let linit_method = make_init_method(
        emitter,
        &mut properties,
        |p| p.flags.is_static() && p.flags.is_lsb(),
        "86linit",
        span,
    )?;

    let initialized_constants: Vec<_> = constants
        .iter_mut()
        .filter_map(|&mut (Constant { ref name, .. }, ref mut instrs)| {
            instrs
                .take()
                .map(|instrs| (name, emitter.label_gen_mut().next_regular(), instrs))
        })
        .collect();
    let cinit_method = if initialized_constants.is_empty() {
        None
    } else {
        let param_local = Local::new(0);
        let params = vec![Param {
            name: hhbc::intern("$constName"),
            is_variadic: false,
            is_splat: false,
            is_inout: false,
            is_readonly: false,
            is_optional: false,
            user_attributes: Default::default(),
            type_info: Nothing, // string?
        }];
        let default_label = emitter.label_gen_mut().next_regular();
        let mut cases = Vec::with_capacity(initialized_constants.len() + 1);
        for (name, label, _) in &initialized_constants {
            cases.push((hhbc::intern_bytes(name.as_bytes()), *label))
        }
        cases.push((hhbc::intern_bytes("default".as_bytes()), default_label));
        let pos = &ast_class.span;
        let instrs = InstrSeq::gather(vec![
            emit_pos::emit_pos(pos),
            instr::c_get_l(param_local),
            instr::s_switch(cases),
            InstrSeq::gather(
                initialized_constants
                    .into_iter()
                    .map(|(_, label, init_instrs)| {
                        // one case for each constant
                        InstrSeq::gather(vec![
                            instr::label(label),
                            init_instrs,
                            emit_pos::emit_pos(pos),
                            instr::ret_c(),
                        ])
                    })
                    .collect(),
            ),
            // default case for constant-not-found
            instr::label(default_label),
            emit_pos::emit_pos(pos),
            instr::string("Could not find initializer for "),
            instr::c_get_l(param_local),
            instr::string(" in 86cinit"),
            instr::concat_n(3),
            instr::fatal(FatalOp::Runtime),
        ]);

        Some(make_86method(
            emitter,
            MethodName::new(string_id!("86cinit")),
            params,
            true, /* is_static */
            Visibility::Private,
            is_interface, /* is_abstract */
            span,
            Coeffects::default(),
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
    additional_methods.extend(reified_init_method);
    additional_methods.extend(pinit_method);
    additional_methods.extend(sinit_method);
    additional_methods.extend(linit_method);
    additional_methods.extend(cinit_method);

    let mut methods = emit_method::from_asts(emitter, ast_class, &ast_class.methods)?;
    methods.extend(additional_methods);
    let (ctxconsts, tconsts): (Vec<_>, Vec<_>) =
        ast_class.typeconsts.iter().partition(|x| x.is_ctx);
    let type_constants = tconsts
        .iter()
        .map(|x| from_type_constant(emitter, x))
        .collect::<Result<Vec<_>>>()?;
    let ctx_constants = ctxconsts
        .iter()
        .map(|x| from_ctx_constant(x))
        .collect::<Result<Vec<CtxConstant>>>()?;
    let upper_bounds = emit_body::emit_generics_upper_bounds(&ast_class.tparams, &[], false);

    if !no_xhp_attributes {
        properties.push(emit_xhp::properties_for_cache(
            emitter, ast_class, is_const, is_closure,
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
    let is_closure = methods.iter().any(|x| x.is_closure_body());
    let is_systemlib = emitter.systemlib();

    let mut flags = Attr::AttrNone;
    flags.set(Attr::AttrAbstract, is_abstract);
    flags.set(Attr::AttrBuiltin, is_systemlib);
    flags.set(Attr::AttrFinal, is_final);
    flags.set(Attr::AttrForbidDynamicProps, no_dynamic_props);
    flags.set(Attr::AttrInterface, is_interface);
    flags.set(Attr::AttrIsConst, is_const);
    flags.set(Attr::AttrNoOverride, is_closure);
    flags.set(Attr::AttrNoReifiedInit, needs_no_reifiedinit);
    flags.set(Attr::AttrPersistent, is_systemlib);
    flags.set(Attr::AttrSealed, is_sealed);
    flags.set(Attr::AttrTrait, is_trait);
    flags.set(Attr::AttrEnumClass, hhbc::has_enum_class(&attributes));
    flags.set(Attr::AttrIsFoldable, hhbc::has_foldable(&attributes));
    flags.set(
        Attr::AttrDynamicallyConstructible,
        hhbc::has_dynamically_constructible(&attributes),
    );
    flags.set(
        Attr::AttrDynamicallyReferenced,
        hhbc::has_dynamically_referenced(&attributes),
    );
    flags.set(
        Attr::AttrEnum,
        enum_type.is_some() && !hhbc::has_enum_class(&attributes),
    );
    flags.set(Attr::AttrInternal, ast_class.internal);
    flags.set(Attr::AttrIsClosureClass, is_closure);

    add_symbol_refs(
        emitter,
        base.as_ref(),
        &implements,
        &uses,
        &requirements,
        &enum_includes,
    );
    Ok(Class {
        attributes: attributes.into(),
        base: Maybe::from(base),
        implements: implements.into(),
        enum_includes: enum_includes.into(),
        name,
        span,
        flags,
        doc_comment: doc_comment
            .map(|(_, comment)| comment.into_bytes().into())
            .into(),
        uses: uses.into(),
        methods: methods.into(),
        enum_type: Maybe::from(enum_type),
        upper_bounds: upper_bounds.into(),
        tparams: tparams.into_iter().map(ClassName::intern).collect(),
        properties: Vec::from_iter(properties.into_iter().map(|p| p.prop)).into(),
        requirements: requirements.into(),
        type_constants: type_constants.into(),
        ctx_constants: ctx_constants.into(),
        constants: Vec::from_iter(constants.into_iter().map(|(c, _)| c)).into(),
    })
}

pub fn emit_classes_from_program<'a, 'd>(
    emitter: &mut Emitter,
    ast: &'a [ast::Def],
) -> Result<Vec<Class>> {
    ast.iter()
        .filter_map(|class| {
            if let ast::Def::Class(cd) = class {
                Some(emit_class(emitter, cd))
            } else {
                None
            }
        })
        .collect()
}
