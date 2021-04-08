// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use ast_scope_rust::{self as ast_scope, Lambda, Scope, ScopeItem};
use emit_attribute_rust as emit_attribute;
use emit_body_rust as emit_body;
use emit_fatal_rust::{emit_fatal_runtimeomitframe, raise_fatal_parse};
use emit_memoize_helpers_rust as emit_memoize_helpers;
use emit_native_opcode_rust as emit_native_opcode;
use env::emitter::Emitter;
use hhas_attribute_rust as hhas_attribute;
use hhas_coeffects::HhasCoeffects;
use hhas_method_rust::{HhasMethod, HhasMethodFlags};
use hhas_pos_rust::Span;
use hhbc_id_rust::method;
use hhbc_string_utils_rust as string_utils;
use instruction_sequence::{instr, Result};
use naming_special_names_rust::{classes, special_idents, user_attributes};
use ocamlrep::rc::RcOc;
use options::{HhvmFlags, Options};
use oxidized::{ast as T, ast_defs};

use itertools::Either;
use std::borrow::Cow;

pub fn from_asts<'a>(
    emitter: &mut Emitter,
    class: &'a T::Class_,
    methods: &'a [T::Method_],
) -> Result<Vec<HhasMethod<'a>>> {
    methods
        .iter()
        .map(|m| from_ast(emitter, class, m))
        .collect::<Result<Vec<_>>>()
}

pub fn from_ast<'a>(
    emitter: &mut Emitter,
    class: &'a T::Class_,
    method_: impl Into<Cow<'a, T::Method_>>,
) -> Result<HhasMethod<'a>> {
    let method_: Cow<'a, T::Method_> = method_.into();
    let method = method_.as_ref();
    let is_memoize = method
        .user_attributes
        .iter()
        .any(|ua| user_attributes::is_memoized(&ua.name.1));
    let class_name = string_utils::mangle(string_utils::strip_global_ns(&class.name.1).into());
    let is_closure_body = &method.name.1 == "__invoke" && (class.name.1).starts_with("Closure$");
    let mut attributes =
        emit_attribute::from_asts(emitter, &class.namespace, &method.user_attributes)?;
    if !is_closure_body {
        attributes.extend(emit_attribute::add_reified_attribute(&method.tparams[..]));
    };
    let call_context = if is_closure_body {
        match &method.user_attributes[..] {
            [T::UserAttribute {
                name: ast_defs::Id(_, ref s),
                params: _,
            }] if s.eq_ignore_ascii_case("__DynamicMethCallerForce") => {
                Some("__SystemLib\\DynamicContextOverrideUnsafe".to_string())
            }
            _ => None,
        }
    } else {
        None
    };

    let is_native = attributes
        .iter()
        .any(|attr| attr.is(user_attributes::is_native));
    let is_native_opcode_impl = hhas_attribute::is_native_opcode_impl(&attributes);
    let is_abstract = class.kind.is_cinterface() || method.abstract_;
    let is_async = method.fun_kind.is_fasync() || method.fun_kind.is_fasync_generator();
    let is_no_injection = hhas_attribute::is_no_injection(&attributes);

    if !(method.static_ || is_closure_body) {
        for p in method.params.iter() {
            if p.name == special_idents::THIS {
                return Err(raise_fatal_parse(&p.pos, "Cannot re-assign $this"));
            }
        }
    };
    if class.kind.is_cinterface() && !method.body.ast.is_empty() {
        return Err(raise_fatal_parse(
            &method.name.0,
            format!(
                "Interface method {}::{} cannot contain body",
                class_name, &method.name.1
            ),
        ));
    };
    if !method.static_ && class.final_ && class.kind.is_cabstract() {
        return Err(raise_fatal_parse(
            &method.name.0,
            format!(
                "Class {} contains non-static method {} and therefore cannot be declared 'abstract final'",
                class_name, &method.name.1
            ),
        ));
    };

    let visibility = if is_native_opcode_impl {
        T::Visibility::Public
    } else if is_memoize {
        T::Visibility::Private
    } else {
        method.visibility
    };
    let deprecation_info = if is_memoize {
        None
    } else {
        hhas_attribute::deprecation_info(attributes.iter())
    };
    let default_dropthrough = if method.abstract_ {
        Some(emit_fatal_runtimeomitframe(
            &&method.name.0,
            format!(
                "Cannot call abstract method {}::{}()",
                class_name, &method.name.1
            ),
        ))
    } else {
        None
    };
    let mut scope = Scope {
        items: vec![
            ScopeItem::Class(ast_scope::Class::new_ref(class)),
            ScopeItem::Method(match &method_ {
                Cow::Borrowed(m) => ast_scope::Method::new_ref(&m),
                Cow::Owned(m) => ast_scope::Method::new_rc(&m),
            }),
        ],
    };
    if is_closure_body {
        scope.items.push(ScopeItem::Lambda(Lambda {
            is_async,
            coeffects: HhasCoeffects::default(),
        }))
    };
    let namespace = RcOc::clone(
        emitter
            .emit_global_state()
            .closure_namespaces
            .get(&class_name)
            .unwrap_or(&class.namespace),
    );
    let mut coeffects = HhasCoeffects::from_ast(&method.ctxs, &method.params);
    if method.ctxs == None && is_closure_body {
        let parent_coeffects = emitter
            .emit_global_state()
            .get_lambda_coeffects_of_scope(&class.name.1, &method.name.1);
        coeffects = parent_coeffects.inherit_to_child_closure()
    }
    if is_native_opcode_impl
        && (class.name.1.as_str() == classes::GENERATOR
            || class.name.1.as_str() == classes::ASYNC_GENERATOR)
    {
        match method.name.1.as_str() {
            "send" | "raise" | "throw" | "next" | "rewind" => {
                coeffects = coeffects.with_gen_coeffect()
            }
            _ => {}
        }
    }
    let (ast_body_block, is_rx_body, rx_disabled) = if !coeffects.is_any_rx() {
        (&method.body.ast, false, false)
    } else {
        match hhas_coeffects::halves_of_is_enabled_body(&method.body) {
            None => (&method.body.ast, true, false),
            Some((enabled_body, disabled_body)) => {
                if emitter
                    .options()
                    .hhvm
                    .flags
                    .contains(options::HhvmFlags::RX_IS_ENABLED)
                {
                    (enabled_body, true, false)
                } else {
                    (disabled_body, false, true)
                }
            }
        }
    };
    let (body, is_generator, is_pair_generator) = if is_native_opcode_impl {
        (
            emit_native_opcode::emit_body(
                emitter,
                &scope,
                namespace.as_ref(),
                &class.user_attributes,
                &method.name,
                &method.params,
                method.ret.1.as_ref(),
            )?,
            false,
            false,
        )
    } else {
        let class_tparam_names = class
            .tparams
            .iter()
            .map(|tp| (tp.name).1.as_str())
            .collect::<Vec<_>>();
        let mut flags = emit_body::Flags::empty();
        flags.set(
            emit_body::Flags::SKIP_AWAITABLE,
            method.fun_kind.is_fasync(),
        );
        flags.set(emit_body::Flags::MEMOIZE, is_memoize);
        flags.set(emit_body::Flags::CLOSURE_BODY, is_closure_body);
        flags.set(emit_body::Flags::NATIVE, is_native);
        flags.set(emit_body::Flags::RX_BODY, is_rx_body);
        flags.set(emit_body::Flags::ASYNC, is_async);
        flags.set(
            emit_body::Flags::HAS_COEFFECTS_LOCAL,
            coeffects.has_coeffects_local(),
        );
        emit_body::emit_body(
            emitter,
            namespace,
            Either::Right(ast_body_block),
            instr::null(),
            scope,
            emit_body::Args {
                immediate_tparams: &method.tparams,
                class_tparam_names: class_tparam_names.as_slice(),
                ast_params: &method.params,
                ret: method.ret.1.as_ref(),
                pos: &method.span,
                deprecation_info: &deprecation_info,
                doc_comment: method.doc_comment.clone(),
                default_dropthrough,
                call_context,
                flags,
            },
        )?
    };
    let name = {
        let mut name: method::Type = string_utils::strip_global_ns(&method.name.1)
            .to_string()
            .into();
        if is_memoize {
            name.add_suffix(emit_memoize_helpers::MEMOIZE_SUFFIX);
        };
        name
    };
    let is_interceptable = is_method_interceptable(emitter.options());
    let span = if is_native_opcode_impl {
        Span(0, 0)
    } else {
        Span::from_pos(&method.span)
    };
    let mut flags = HhasMethodFlags::empty();
    flags.set(HhasMethodFlags::IS_STATIC, method.static_);
    flags.set(HhasMethodFlags::IS_FINAL, method.final_);
    flags.set(HhasMethodFlags::IS_ABSTRACT, is_abstract);
    flags.set(HhasMethodFlags::IS_ASYNC, is_async);
    flags.set(HhasMethodFlags::IS_GENERATOR, is_generator);
    flags.set(HhasMethodFlags::IS_PAIR_GENERATOR, is_pair_generator);
    flags.set(HhasMethodFlags::IS_CLOSURE_BODY, is_closure_body);
    flags.set(HhasMethodFlags::IS_INTERCEPTABLE, is_interceptable);
    flags.set(HhasMethodFlags::IS_MEMOIZE_IMPL, is_memoize);
    flags.set(HhasMethodFlags::RX_DISABLED, rx_disabled);
    flags.set(HhasMethodFlags::NO_INJECTION, is_no_injection);
    Ok(HhasMethod {
        attributes,
        visibility,
        name,
        body,
        span,
        coeffects,
        flags,
    })
}

fn is_method_interceptable(opts: &Options) -> bool {
    opts.hhvm
        .flags
        .contains(HhvmFlags::JIT_ENABLE_RENAME_FUNCTION)
}
