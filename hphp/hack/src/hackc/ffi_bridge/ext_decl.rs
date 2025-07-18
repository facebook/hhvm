// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_string_utils::mangle_xhp_id;
use hhbc_string_utils::strip_global_ns;
use oxidized_by_ref::ast_defs::Id;
use oxidized_by_ref::direct_decl_parser::ParsedFile;
use oxidized_by_ref::file_info::Mode;
use oxidized_by_ref::shallow_decl_defs::AbstractTypeconst;
use oxidized_by_ref::shallow_decl_defs::ClassConstKind;
use oxidized_by_ref::shallow_decl_defs::ConcreteTypeconst;
use oxidized_by_ref::shallow_decl_defs::DeclConstraintRequirement;
use oxidized_by_ref::shallow_decl_defs::ShallowClass;
use oxidized_by_ref::shallow_decl_defs::ShallowClassConst;
use oxidized_by_ref::shallow_decl_defs::ShallowMethod;
use oxidized_by_ref::shallow_decl_defs::ShallowProp;
use oxidized_by_ref::shallow_decl_defs::ShallowTypeconst;
use oxidized_by_ref::shallow_decl_defs::Typeconst;
use oxidized_by_ref::typing_defs::TypedefTypeAssignment;
use oxidized_by_ref::typing_defs_core::FunParams;
use oxidized_by_ref::typing_defs_core::ShapeType;
use oxidized_by_ref::typing_defs_core::Tparam;
use oxidized_by_ref::typing_defs_core::TshapeFieldName;
use oxidized_by_ref::typing_defs_core::TupleType;
use oxidized_by_ref::typing_defs_core::Ty;
use oxidized_by_ref::typing_defs_core::Ty_;
use oxidized_by_ref::typing_defs_core::UserAttribute;
use oxidized_by_ref::typing_defs_core::UserAttributeParam;
use oxidized_by_ref::typing_reason;
use ty::reason::BReason;

use crate::ffi::ExtDeclAttribute;
use crate::ffi::ExtDeclClass;
use crate::ffi::ExtDeclClassConst;
use crate::ffi::ExtDeclClassTypeConst;
use crate::ffi::ExtDeclEnumType;
use crate::ffi::ExtDeclFile;
use crate::ffi::ExtDeclFileConst;
use crate::ffi::ExtDeclFileFunc;
use crate::ffi::ExtDeclMethod;
use crate::ffi::ExtDeclMethodParam;
use crate::ffi::ExtDeclModule;
use crate::ffi::ExtDeclProp;
use crate::ffi::ExtDeclSignature;
use crate::ffi::ExtDeclTparam;
use crate::ffi::ExtDeclTypeConstraint;
use crate::ffi::ExtDeclTypeDef;
use crate::ffi::ExtDeclTypeStructure;
use crate::ffi::ExtDeclTypeStructureSubType;

fn find_class<'a>(parsed_file: &ParsedFile<'a>, symbol: &str) -> Option<&'a ShallowClass<'a>> {
    let input_symbol_formatted = symbol.starts_with('\\');
    parsed_file
        .decls
        .classes()
        .find(|&(mut sym, _)| {
            if !input_symbol_formatted {
                sym = &sym[1..];
            }
            sym == symbol
        })
        .map(|(_, shallow_class)| shallow_class)
}

pub fn get_file_attributes(parsed_file: &ParsedFile<'_>, name: &str) -> Vec<ExtDeclAttribute> {
    get_attributes(parsed_file.file_attributes, name)
}

pub fn get_file_consts(parsed_file: &ParsedFile<'_>, name: &str) -> Vec<ExtDeclFileConst> {
    parsed_file
        .decls
        .consts()
        .filter(|(cname, _)| name.is_empty() || name == strip_global_ns(cname))
        .map(|(cname, decl)| ExtDeclFileConst {
            name: fmt_type(cname),
            type_: extract_type_name(decl.type_),
            value: str_or_empty(decl.value),
        })
        .collect()
}

pub fn get_file_funcs(parsed_file: &ParsedFile<'_>, name: &str) -> Vec<ExtDeclFileFunc> {
    parsed_file
        .decls
        .funs()
        .filter(|(cname, _)| name.is_empty() || name == strip_global_ns(cname))
        .map(|(cname, decl)| ExtDeclFileFunc {
            name: fmt_type(cname),
            type_: extract_type_name(decl.type_),
            module: id_or_empty(decl.module),
            internal: decl.internal,
            support_dynamic_type: decl.support_dynamic_type,
            php_std_lib: decl.php_std_lib,
            no_auto_dynamic: decl.no_auto_dynamic,
            no_auto_likes: decl.no_auto_likes,
            signature: get_signature(decl.type_.1),
        })
        .collect()
}

pub fn get_file_modules(parsed_file: &ParsedFile<'_>, name: &str) -> Vec<ExtDeclModule> {
    parsed_file
        .decls
        .modules()
        .filter(|(cname, _)| name.is_empty() || cname == &name)
        .map(|(cname, _)| ExtDeclModule {
            name: fmt_type(cname),
        })
        .collect()
}

pub fn get_file_module_membership(parsed_file: &ParsedFile<'_>) -> String {
    if let Some(module_membership) = parsed_file.module_membership {
        module_membership.to_string()
    } else {
        "".to_string()
    }
}

pub fn get_file_typedefs(parsed_file: &ParsedFile<'_>, name: &str) -> Vec<ExtDeclTypeDef> {
    parsed_file
        .decls
        .typedefs()
        .filter(|(cname, _)| name.is_empty() || name == strip_global_ns(cname))
        .map(|(cname, decl)| {
            let (visibility, type_) = match decl.type_assignment {
                TypedefTypeAssignment::SimpleTypeDef((vis, hint)) => {
                    (enum_typedef_visibility(*vis), extract_type_name(hint))
                }
                TypedefTypeAssignment::CaseType((variant, variants)) => {
                    let mut hints;
                    let ty_ = if variants.is_empty() {
                        variant.0.1
                    } else {
                        hints = variants.iter().map(|v| v.0).collect::<Vec<_>>();
                        hints.insert(0, variant.0);
                        Ty_::Tunion(&hints)
                    };
                    (
                        String::from("case_type"),
                        extract_type_name(&Ty(
                            &typing_reason::Reason::NoReason, // the position is ignored when printing the type
                            ty_,
                        )),
                    )
                }
            };
            ExtDeclTypeDef {
                name: fmt_type(cname),
                module: id_or_empty(decl.module),
                visibility,
                tparams: get_typed_params(decl.tparams),
                as_constraint: extract_type_name_opt(decl.as_constraint),
                super_constraint: extract_type_name_opt(decl.super_constraint),
                type_,
                is_ctx: decl.is_ctx,
                attributes: get_attributes(decl.attributes, ""),
                internal: decl.internal,
                docs_url: str_or_empty(decl.docs_url),
            }
        })
        .collect()
}

pub fn get_type_structure(parsed_file: &ParsedFile<'_>, name: &str) -> Vec<ExtDeclTypeStructure> {
    parsed_file
        .decls
        .typedefs()
        .filter(|(cname, _)| name.is_empty() || name == strip_global_ns(cname))
        .map(|(_cname, decl)| match decl.type_assignment {
            TypedefTypeAssignment::SimpleTypeDef((_vis, hint)) => build_type_structure(hint),
            TypedefTypeAssignment::CaseType((variant, variants)) => {
                let mut hints = variants.iter().map(|v| v.0).collect::<Vec<_>>();
                hints.insert(0, variant.0);
                build_type_structure(&Ty(
                    // the reason is not included in the type structure
                    &typing_reason::Reason::NoReason,
                    Ty_::Tunion(&hints),
                ))
            }
        })
        .collect()
}

pub fn get_file(parsed_file: &ParsedFile<'_>) -> ExtDeclFile {
    ExtDeclFile {
        disable_xhp_element_mangling: parsed_file.disable_xhp_element_mangling,
        has_first_pass_parse_errors: parsed_file.has_first_pass_parse_errors,
        is_strict: parsed_file.mode.is_some_and(|x| x == Mode::Mstrict),
        typedefs: get_file_typedefs(parsed_file, ""),
        functions: get_file_funcs(parsed_file, ""),
        constants: get_file_consts(parsed_file, ""),
        file_attributes: get_file_attributes(parsed_file, ""),
        modules: get_file_modules(parsed_file, ""),
        classes: get_classes(parsed_file),
    }
}

pub fn get_shape_keys(parsed_file: &ParsedFile<'_>, name: &str) -> Vec<String> {
    // don't let empty strings pass in, else we fetch all type structures
    if name.is_empty() {
        return vec![];
    }
    let type_structure = get_type_structure(parsed_file, name);
    // if empty, then no matching type for the name
    let Some(type_) = type_structure.first() else {
        return vec![];
    };
    // can't get keys unless it's a shape
    match type_.kind.as_str() {
        "shape" => type_.subtypes.iter().map(|st| st.name.clone()).collect(),
        _ => vec![],
    }
}

//pub fn get_method(parsed_file: &ParsedFile<'_>, name: &str) -> Option<ExtDeclClass>
pub fn get_class_methods(
    parsed_file: &ParsedFile<'_>,
    kls: &str,
    name: &str,
) -> Vec<ExtDeclMethod> {
    let class_opt = find_class(parsed_file, kls);
    match class_opt {
        Some(cls) => get_methods(cls.methods, name),
        None => Vec::new(),
    }
}

pub fn get_class_smethods(
    parsed_file: &ParsedFile<'_>,
    kls: &str,
    name: &str,
) -> Vec<ExtDeclMethod> {
    let class_opt = find_class(parsed_file, kls);
    match class_opt {
        Some(cls) => get_methods(cls.static_methods, name),
        None => Vec::new(),
    }
}

pub fn get_class_consts(
    parsed_file: &ParsedFile<'_>,
    kls: &str,
    name: &str,
) -> Vec<ExtDeclClassConst> {
    let class_opt = find_class(parsed_file, kls);
    match class_opt {
        Some(cls) => get_consts(cls.consts, name),
        None => Vec::new(),
    }
}

pub fn get_class_typeconsts(
    parsed_file: &ParsedFile<'_>,
    kls: &str,
    name: &str,
) -> Vec<ExtDeclClassTypeConst> {
    let class_opt = find_class(parsed_file, kls);
    match class_opt {
        Some(cls) => get_typeconsts(cls.typeconsts, name),
        None => Vec::new(),
    }
}

pub fn get_class_props(parsed_file: &ParsedFile<'_>, kls: &str, name: &str) -> Vec<ExtDeclProp> {
    let class_opt = find_class(parsed_file, kls);
    match class_opt {
        Some(cls) => get_props(cls.props, name),
        None => Vec::new(),
    }
}

pub fn get_class_sprops(parsed_file: &ParsedFile<'_>, kls: &str, name: &str) -> Vec<ExtDeclProp> {
    let class_opt = find_class(parsed_file, kls);
    match class_opt {
        Some(cls) => get_props(cls.sprops, name),
        None => Vec::new(),
    }
}

pub fn get_class_attributes(
    parsed_file: &ParsedFile<'_>,
    kls: &str,
    name: &str,
) -> Vec<ExtDeclAttribute> {
    let class_opt = find_class(parsed_file, kls);
    match class_opt {
        Some(cls) => get_attributes(cls.user_attributes, name),
        None => Vec::new(),
    }
}

pub fn get_classes(parsed_file: &ParsedFile<'_>) -> Vec<ExtDeclClass> {
    parsed_file
        .decls
        .classes()
        .map(|(_kls, decl)| get_class_impl(decl))
        .collect()
}

pub fn get_class(parsed_file: &ParsedFile<'_>, name: &str) -> Option<ExtDeclClass> {
    let class_opt = find_class(parsed_file, name);
    Some(get_class_impl(class_opt?))
}

// ========== Complex fields ==========

fn get_class_impl(class: &ShallowClass<'_>) -> ExtDeclClass {
    ExtDeclClass {
        kind: enum_class_kind(class.kind), // ClassishKind (cls, iface, trait, enum)
        name: fmt_type(class.name.1),

        // Optional strings
        module: id_or_empty(class.module),
        docs_url: str_or_empty(class.docs_url),
        enum_type: get_enum_type(class.enum_type),

        // Flags
        final_: class.final_,
        abstract_: class.abstract_,
        is_xhp: class.is_xhp,
        internal: class.internal,
        has_xhp_keyword: class.has_xhp_keyword,
        xhp_marked_empty: class.xhp_marked_empty,
        support_dynamic_type: class.support_dynamic_type,
        is_strict: class.mode == oxidized_by_ref::file_info::Mode::Mstrict,

        // Special Params
        tparams: get_typed_params(class.tparams),

        // Implementation
        extends: extract_type_name_vec(class.extends),
        uses: extract_type_name_vec(class.uses),
        implements: extract_type_name_vec(class.implements),
        require_extends: extract_type_name_vec(class.req_extends),
        require_implements: extract_type_name_vec(class.req_implements),
        require_class: extract_type_name_cr_vec(class.req_constraints),
        xhp_attr_uses: extract_type_name_vec(class.xhp_attr_uses),

        // Nested Types
        user_attributes: get_attributes(class.user_attributes, ""),
        methods: get_methods(class.methods, ""),
        static_methods: get_methods(class.static_methods, ""),
        constructor: get_method_opt(class.constructor),
        consts: get_consts(class.consts, ""),
        typeconsts: get_typeconsts(class.typeconsts, ""),
        props: get_props(class.props, ""),
        sprops: get_props(class.sprops, ""),
        /*
            Not supported yet
            pub xhp_enum_values: s_map::SMap<'a, &'a [ast_defs::XhpEnumValue<'a>]>,
        */
    }
}
fn get_method_params(params: &FunParams<'_>) -> Vec<ExtDeclMethodParam> {
    params
        .iter()
        .map(|p| ExtDeclMethodParam {
            name: str_or_empty(p.name),
            type_: extract_type_name(p.type_),
            accept_disposable: p.flags.accepts_disposable(),
            is_inout: p.flags.is_inout(),
            has_default: p.def_value.is_some(),
            is_optional: p.flags.is_optional(),
            is_readonly: p.flags.is_readonly(),
            def_value: str_or_empty(p.def_value),
        })
        .collect()
}

fn get_props(props: &[&ShallowProp<'_>], name: &str) -> Vec<ExtDeclProp> {
    props
        .iter()
        .filter(|prop| name.is_empty() || prop.name.1 == name)
        .map(|prop| ExtDeclProp {
            name: prop.name.1.to_string(),
            visibility: enum_visibility(prop.visibility),
            type_: extract_type_name(prop.type_),
            is_abstract: prop.flags.is_abstract(),
            is_const: prop.flags.is_const(),
            is_lateinit: prop.flags.is_lateinit(),
            is_readonly: prop.flags.is_readonly(),
            needs_init: prop.flags.needs_init(),
            is_php_std_lib: prop.flags.is_php_std_lib(),
            is_lsb: prop.flags.is_lsb(),
            is_safe_global_variable: prop.flags.is_safe_global_variable(),
            no_auto_likes: prop.flags.no_auto_likes(),
            // prop.xhp_attr - not supported yet
        })
        .collect()
}

fn get_method_opt(method: Option<&ShallowMethod<'_>>) -> Vec<ExtDeclMethod> {
    match method {
        Some(method) => vec![get_method(method)],
        None => vec![],
    }
}

fn get_method(meth: &ShallowMethod<'_>) -> ExtDeclMethod {
    ExtDeclMethod {
        name: fmt_type(meth.name.1),
        type_: extract_type_name(meth.type_),
        visibility: enum_visibility(meth.visibility),
        attributes: get_attributes(meth.attributes, ""),
        signature: get_signature(meth.type_.1),
        is_abstract: meth.flags.is_abstract(),
        is_final: meth.flags.is_final(),
        is_dynamicallycallable: meth.flags.is_dynamicallycallable(),
        is_override: meth.flags.is_override(),
        is_php_std_lib: meth.flags.is_php_std_lib(),
        supports_dynamic_type: meth.flags.supports_dynamic_type(),
    }
}

fn get_methods(methods: &[&ShallowMethod<'_>], name: &str) -> Vec<ExtDeclMethod> {
    methods
        .iter()
        .filter(|meth| name.is_empty() || meth.name.1 == name)
        .map(|meth| get_method(meth))
        .collect()
}

fn get_typeconsts(typeconsts: &[&ShallowTypeconst<'_>], name: &str) -> Vec<ExtDeclClassTypeConst> {
    typeconsts
        .iter()
        .filter(|tc| name.is_empty() || tc.name.1 == name)
        .map(|tc| {
            let kind = match &tc.kind {
                Typeconst::TCAbstract(AbstractTypeconst { default, .. }) => match default {
                    None => String::new(),
                    Some(d) => extract_type_name(d),
                },
                Typeconst::TCConcrete(ConcreteTypeconst { tc_type, .. }) => {
                    extract_type_name(tc_type)
                }
            };

            ExtDeclClassTypeConst {
                name: fmt_type(tc.name.1),
                is_ctx: tc.is_ctx,
                enforceable: tc.enforceable.1,
                reifiable: tc.reifiable.is_some(),
                kind,
            }
        })
        .collect()
}

fn get_consts(consts: &[&ShallowClassConst<'_>], name: &str) -> Vec<ExtDeclClassConst> {
    consts
        .iter()
        .filter(|c| name.is_empty() || c.name.1 == name)
        .map(|c| ExtDeclClassConst {
            name: fmt_type(c.name.1),
            type_: extract_type_name(c.type_),
            is_abstract: c.abstract_ != ClassConstKind::CCConcrete,
            value: str_or_empty(c.value),
        })
        .collect()
}

fn get_enum_type(
    enum_type: Option<&oxidized_by_ref::shallow_decl_defs::EnumType<'_>>,
) -> Vec<ExtDeclEnumType> {
    match enum_type {
        Some(en) => vec![ExtDeclEnumType {
            base: extract_type_name(en.base),
            constraint: extract_type_name_opt(en.constraint),
            includes: extract_type_name_vec(en.includes),
        }],
        None => vec![],
    }
}

fn get_where_contraints(
    arr: &[&oxidized_by_ref::shallow_decl_defs::WhereConstraint<'_>],
) -> Vec<ExtDeclTypeConstraint> {
    arr.iter()
        .map(|w| ExtDeclTypeConstraint {
            type_: extract_type_name(w.0),
            kind: enum_constraint_kind(w.1),
        })
        .collect()
}

fn get_typed_params(arr: &[&Tparam<'_>]) -> Vec<ExtDeclTparam> {
    arr.iter()
        .map(|t| ExtDeclTparam {
            variance: enum_variance(t.variance),
            name: fmt_type(t.name.1),
            constraints: (t.constraints.iter())
                .map(|c| ExtDeclTypeConstraint {
                    kind: enum_constraint_kind(c.0),
                    type_: extract_type_name(c.1),
                })
                .collect(),
            reified: enum_reify_kind(t.reified),
            user_attributes: get_attributes(t.user_attributes, ""),
        })
        .collect()
}

fn get_attributes(arr: &[&UserAttribute<'_>], name: &str) -> Vec<ExtDeclAttribute> {
    let input_symbol_formatted = name.starts_with('\\');

    arr.iter()
        .filter(|t| {
            name.is_empty()
                || t.name.1 == name
                || (!input_symbol_formatted && &t.name.1[1..] == name)
        })
        .map(|t| ExtDeclAttribute {
            name: fmt_type(t.name.1),
            args: (t.params.iter())
                .map(|arg| match arg {
                    UserAttributeParam::Classname(cn) => fmt_type(cn),
                    UserAttributeParam::String(s) => s.to_string(),
                    UserAttributeParam::Int(i) => i.to_string(),
                    _ => String::from("__ext_decl_unknown"),
                })
                .collect(),
            raw_val: str_or_empty(t.raw_val),
        })
        .collect()
}

fn get_signature(ty: Ty_<'_>) -> Vec<ExtDeclSignature> {
    match ty {
        oxidized_by_ref::typing_defs::Ty_::Tfun(ft) => {
            let implicit_params = match ft.implicit_params.capability {
                oxidized_by_ref::typing_defs_core::Capability::CapDefaults(_) => String::new(),
                oxidized_by_ref::typing_defs_core::Capability::CapTy(x) => extract_type_name(x),
            };

            vec![ExtDeclSignature {
                tparams: get_typed_params(ft.tparams),
                where_constraints: get_where_contraints(ft.where_constraints),
                return_type: extract_type_name(ft.ret),
                params: get_method_params(ft.params),
                implicit_params,
                cross_package: str_or_empty(ft.cross_package),
                return_disposable: ft.flags.return_disposable(),
                is_coroutine: ft.flags.is_coroutine(),
                is_async: ft.flags.is_async(),
                is_generator: ft.flags.is_generator(),
                instantiated_targs: ft.flags.instantiated_targs(),
                is_function_pointer: ft.flags.is_function_pointer(),
                returns_readonly: ft.flags.returns_readonly(),
                readonly_this: ft.flags.readonly_this(),
                support_dynamic_type: ft.flags.support_dynamic_type(),
                is_memoized: ft.flags.is_memoized(),
                variadic: ft.flags.variadic(),
            }]
        }
        _ => Vec::new(),
    }
}

fn build_unnamed_type_structure_subtype(ty: &&Ty<'_>) -> ExtDeclTypeStructureSubType {
    ExtDeclTypeStructureSubType {
        name: String::new(),
        optional: false,
        type_: build_type_structure(ty),
    }
}

fn build_type_structure(outer_ty: &Ty<'_>) -> ExtDeclTypeStructure {
    match outer_ty.1 {
        Ty_::Taccess(_)
        | Ty_::Tthis
        | Ty_::Tany(_)
        | Ty_::Tclass(_)
        | Ty_::Tdynamic
        | Ty_::Tgeneric(_)
        | Ty_::Tmixed
        | Ty_::Twildcard
        | Ty_::Tnonnull
        | Ty_::Tprim(_) => ExtDeclTypeStructure {
            type_: extract_type_name(outer_ty),
            kind: String::from("primitive"),
            nullable: false,
            subtypes: vec![],
        },
        Ty_::Tfun(_fun_type) => ExtDeclTypeStructure {
            type_: extract_type_name(outer_ty),
            kind: String::from("function"),
            nullable: false,
            subtypes: vec![],
        },

        // Pseudo Recursive
        Ty_::Toption(ty) => {
            let mut inner = build_type_structure(ty);
            inner.nullable = true;
            inner
        }

        Ty_::Tapply(&(_id, targs)) => ExtDeclTypeStructure {
            type_: extract_type_name(outer_ty),
            kind: String::from("apply"),
            nullable: false,
            subtypes: targs
                .iter()
                .map(build_unnamed_type_structure_subtype)
                .collect(),
        },
        Ty_::Tlike(ty) => ExtDeclTypeStructure {
            type_: extract_type_name(outer_ty),
            kind: String::from("like"),
            nullable: false,
            subtypes: vec![build_unnamed_type_structure_subtype(&ty)],
        },
        Ty_::Tshape(&ShapeType {
            origin: _,
            unknown_value: _kind,
            fields,
        }) => {
            let shape_fields = fields
                .iter()
                .map(
                    |(shape_field, shape_field_type)| ExtDeclTypeStructureSubType {
                        name: match shape_field.0 {
                            TshapeFieldName::TSFregexGroup(field_name) => field_name.1.to_string(),
                            TshapeFieldName::TSFlitStr(field_name) => field_name.1.to_string(),
                            TshapeFieldName::TSFclassConst(((_pos, classish_name), field_name)) => {
                                let cname = fmt_type(classish_name);
                                format!("{}::{}", cname, field_name.1)
                            }
                        },
                        type_: build_type_structure(shape_field_type.ty),
                        optional: shape_field_type.optional,
                    },
                )
                .collect();

            ExtDeclTypeStructure {
                type_: extract_type_name(outer_ty),
                kind: String::from("shape"),
                nullable: false,
                subtypes: shape_fields,
            }
        }
        Ty_::TvecOrDict(&(tk, tv)) => ExtDeclTypeStructure {
            type_: extract_type_name(outer_ty),
            kind: String::from("refinement"),
            nullable: false,
            subtypes: vec![
                ExtDeclTypeStructureSubType {
                    name: String::from("keys"),
                    optional: false,
                    type_: build_type_structure(tk),
                },
                ExtDeclTypeStructureSubType {
                    name: String::from("values"),
                    optional: false,
                    type_: build_type_structure(tv),
                },
            ],
        },
        Ty_::Ttuple(&TupleType { required, extra: _ }) => ExtDeclTypeStructure {
            type_: extract_type_name(outer_ty),
            kind: String::from("tuple"),
            nullable: false,
            subtypes: required
                .iter()
                .map(build_unnamed_type_structure_subtype)
                .collect(),
        },
        Ty_::Tintersection(tys) => ExtDeclTypeStructure {
            type_: extract_type_name(outer_ty),
            kind: String::from("intersection"),
            nullable: false,
            subtypes: tys
                .iter()
                .map(build_unnamed_type_structure_subtype)
                .collect(),
        },
        Ty_::Tunion(tys) => ExtDeclTypeStructure {
            type_: extract_type_name(outer_ty),
            kind: String::from("union"),
            nullable: false,
            subtypes: tys
                .iter()
                .map(build_unnamed_type_structure_subtype)
                .collect(),
        },
        Ty_::Trefinement(&(root_ty, _class_ref)) => ExtDeclTypeStructure {
            type_: extract_type_name(outer_ty),
            kind: String::from("refinement"),
            nullable: false,
            subtypes: vec![build_unnamed_type_structure_subtype(&root_ty)],
        },
        _ => ExtDeclTypeStructure {
            type_: extract_type_name(outer_ty),
            kind: String::from("other"),
            nullable: false,
            subtypes: vec![],
        },
    }
}

// ========== Types ==========

fn extract_type_name_opt(t: Option<&Ty<'_>>) -> String {
    match t {
        Some(ty) => extract_type_name(ty),
        None => String::new(),
    }
}

fn extract_type_name(t: &Ty<'_>) -> String {
    #[allow(suspicious_double_ref_op)]
    let converted_ty: ty::decl::Ty<BReason> = (&t).clone().into();
    converted_ty.to_string()
}

fn extract_type_name_vec(arr: &[&Ty<'_>]) -> Vec<String> {
    arr.iter().map(|t| extract_type_name(t)).collect()
}

fn extract_type_name_cr_vec(arr: &[DeclConstraintRequirement<'_>]) -> Vec<String> {
    arr.iter()
        .filter_map(|cr| match cr {
            DeclConstraintRequirement::DCREqual(t) => Some(extract_type_name(t)),
            DeclConstraintRequirement::DCRSubtype(_) => None,
        })
        .collect()
}

fn fmt_type(original_name: &str) -> String {
    let unqualified = strip_global_ns(original_name);
    match unqualified.rsplit('\\').next() {
        Some(id) if original_name.starts_with('\\') && id.starts_with(':') => {
            // only mangle already qualified xhp ids - avoid mangling string literals
            // containing an xhp name, for example an attribute param ':foo:bar'
            mangle_xhp_id(id.to_string())
        }
        _ => String::from(unqualified),
    }
}

// ========== Simple ==========

fn id_or_empty(x: Option<Id<'_>>) -> String {
    str_or_empty(x.map(|y| y.name()))
}

fn str_or_empty(x: Option<&str>) -> String {
    x.map_or_else(String::new, |y| y.to_string())
}

// ========== ENUMS ==========
fn enum_class_kind(x: oxidized_by_ref::ast_defs::ClassishKind) -> String {
    use oxidized_by_ref::ast_defs::ClassishKind;
    match x {
        ClassishKind::Cclass(_) => String::from("class"),
        ClassishKind::Cinterface => String::from("interface"),
        ClassishKind::Ctrait => String::from("trait"),
        ClassishKind::Cenum => String::from("enum"),
        ClassishKind::CenumClass(_) => String::from("enum_class"),
    }
}

fn enum_variance(x: oxidized::ast_defs::Variance) -> String {
    use oxidized::ast_defs::Variance;
    match x {
        Variance::Covariant => String::from("covariant"),
        Variance::Contravariant => String::from("contravariant"),
        Variance::Invariant => String::from("invariant"),
    }
}

fn enum_reify_kind(x: oxidized::ast_defs::ReifyKind) -> String {
    use oxidized::ast_defs::ReifyKind;
    match x {
        ReifyKind::Erased => String::from("erased"),
        ReifyKind::SoftReified => String::from("soft_reified"),
        ReifyKind::Reified => String::from("reified"),
    }
}

fn enum_constraint_kind(x: oxidized::ast_defs::ConstraintKind) -> String {
    use oxidized::ast_defs::ConstraintKind;
    match x {
        ConstraintKind::ConstraintAs => String::from("constraint_as"),
        ConstraintKind::ConstraintEq => String::from("constraint_eq"),
        ConstraintKind::ConstraintSuper => String::from("constraint_super"),
    }
}

fn enum_visibility(x: oxidized_by_ref::ast_defs::Visibility) -> String {
    use oxidized_by_ref::ast_defs::Visibility;
    match x {
        Visibility::Private => String::from("private"),
        Visibility::Public => String::from("public"),
        Visibility::Protected => String::from("protected"),
        Visibility::Internal => String::from("internal"),
        Visibility::ProtectedInternal => String::from("protected internal"),
    }
}

fn enum_typedef_visibility(x: oxidized_by_ref::ast_defs::TypedefVisibility) -> String {
    use oxidized_by_ref::ast_defs::TypedefVisibility;
    match x {
        TypedefVisibility::Transparent => String::from("transparent"),
        TypedefVisibility::Opaque => String::from("opaque"),
        TypedefVisibility::OpaqueModule => String::from("opaque_module"),
    }
}
