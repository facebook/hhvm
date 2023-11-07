// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod ext_decl {
    use hhbc_string_utils::mangle_xhp_id;
    use hhbc_string_utils::strip_global_ns;
    use oxidized_by_ref::ast_defs::Id;
    use oxidized_by_ref::direct_decl_parser::ParsedFile;
    use oxidized_by_ref::file_info::Mode;
    use oxidized_by_ref::shallow_decl_defs::AbstractTypeconst;
    use oxidized_by_ref::shallow_decl_defs::ClassConstKind;
    use oxidized_by_ref::shallow_decl_defs::ConcreteTypeconst;
    use oxidized_by_ref::shallow_decl_defs::ShallowClass;
    use oxidized_by_ref::shallow_decl_defs::ShallowClassConst;
    use oxidized_by_ref::shallow_decl_defs::ShallowMethod;
    use oxidized_by_ref::shallow_decl_defs::ShallowProp;
    use oxidized_by_ref::shallow_decl_defs::ShallowTypeconst;
    use oxidized_by_ref::shallow_decl_defs::Typeconst;
    use oxidized_by_ref::typing_defs::ModuleReference;
    use oxidized_by_ref::typing_defs_core::Enforcement;
    use oxidized_by_ref::typing_defs_core::FunParams;
    use oxidized_by_ref::typing_defs_core::Tparam;
    use oxidized_by_ref::typing_defs_core::Ty;
    use oxidized_by_ref::typing_defs_core::Ty_;
    use oxidized_by_ref::typing_defs_core::UserAttribute;
    use oxidized_by_ref::typing_defs_core::UserAttributeParam;
    use ty::reason::BReason;

    use crate::compile_ffi::ExtDeclAttribute;
    use crate::compile_ffi::ExtDeclAttributeVec;
    use crate::compile_ffi::ExtDeclClass;
    use crate::compile_ffi::ExtDeclClassConst;
    use crate::compile_ffi::ExtDeclClassConstVec;
    use crate::compile_ffi::ExtDeclClassTypeConst;
    use crate::compile_ffi::ExtDeclClassTypeConstVec;
    use crate::compile_ffi::ExtDeclClassVec;
    use crate::compile_ffi::ExtDeclEnumType;
    use crate::compile_ffi::ExtDeclFile;
    use crate::compile_ffi::ExtDeclFileConst;
    use crate::compile_ffi::ExtDeclFileConstVec;
    use crate::compile_ffi::ExtDeclFileFunc;
    use crate::compile_ffi::ExtDeclFileFuncVec;
    use crate::compile_ffi::ExtDeclMethod;
    use crate::compile_ffi::ExtDeclMethodParam;
    use crate::compile_ffi::ExtDeclMethodVec;
    use crate::compile_ffi::ExtDeclModule;
    use crate::compile_ffi::ExtDeclModuleVec;
    use crate::compile_ffi::ExtDeclProp;
    use crate::compile_ffi::ExtDeclPropVec;
    use crate::compile_ffi::ExtDeclSignature;
    use crate::compile_ffi::ExtDeclTparam;
    use crate::compile_ffi::ExtDeclTypeConstraint;
    use crate::compile_ffi::ExtDeclTypeDef;
    use crate::compile_ffi::ExtDeclTypeDefVec;

    pub struct ExtDecl {}

    impl ExtDecl {
        fn find_class<'a>(
            parsed_file: &ParsedFile<'a>,
            symbol: &str,
        ) -> Option<&'a ShallowClass<'a>> {
            let input_symbol_formatted = symbol.starts_with('\\');
            let Some((_, shallow_class)) = parsed_file.decls.classes().find(|(mut sym, _)| {
                if !input_symbol_formatted {
                    sym = &sym[1..];
                }
                sym == symbol
            }) else {
                return None;
            };

            Some(shallow_class)
        }

        pub fn get_file_attributes(
            parsed_file: &ParsedFile<'_>,
            name: &str,
        ) -> ExtDeclAttributeVec {
            ExtDeclAttributeVec {
                vec: Self::get_attributes(parsed_file.file_attributes, name),
            }
        }

        pub fn get_file_consts(parsed_file: &ParsedFile<'_>, name: &str) -> ExtDeclFileConstVec {
            let consts = parsed_file
                .decls
                .consts()
                .filter(|(cname, _)| name.is_empty() || name == strip_global_ns(cname))
                .map(|(cname, decl)| ExtDeclFileConst {
                    name: Self::fmt_type(cname),
                    type_: Self::extract_type_name(decl.type_),
                })
                .collect();
            ExtDeclFileConstVec { vec: consts }
        }

        pub fn get_file_funcs(parsed_file: &ParsedFile<'_>, name: &str) -> ExtDeclFileFuncVec {
            let funs = parsed_file
                .decls
                .funs()
                .filter(|(cname, _)| name.is_empty() || name == strip_global_ns(cname))
                .map(|(cname, decl)| ExtDeclFileFunc {
                    name: Self::fmt_type(cname),
                    type_: Self::extract_type_name(decl.type_),
                    module: Self::id_or_empty(decl.module),
                    internal: decl.internal,
                    support_dynamic_type: decl.support_dynamic_type,
                    php_std_lib: decl.php_std_lib,
                    no_auto_dynamic: decl.no_auto_dynamic,
                    no_auto_likes: decl.no_auto_likes,
                    signature: Self::get_signature(decl.type_.1),
                })
                .collect();
            ExtDeclFileFuncVec { vec: funs }
        }

        pub fn get_file_modules(parsed_file: &ParsedFile<'_>, name: &str) -> ExtDeclModuleVec {
            let modules = parsed_file
                .decls
                .modules()
                .filter(|(cname, _)| name.is_empty() || cname == &name)
                .map(|(cname, decl)| ExtDeclModule {
                    name: Self::fmt_type(cname),
                    exports: Self::extract_module_refs(decl.exports),
                    imports: Self::extract_module_refs(decl.imports),
                })
                .collect();
            ExtDeclModuleVec { vec: modules }
        }

        pub fn get_file_typedefs(parsed_file: &ParsedFile<'_>, name: &str) -> ExtDeclTypeDefVec {
            let consts = parsed_file
                .decls
                .typedefs()
                .filter(|(cname, _)| name.is_empty() || name == strip_global_ns(cname))
                .map(|(cname, decl)| ExtDeclTypeDef {
                    name: Self::fmt_type(cname),
                    module: Self::id_or_empty(decl.module),
                    visibility: Self::enum_typedef_visibility(decl.vis),
                    tparams: Self::get_typed_params(decl.tparams),
                    as_constraint: Self::extract_type_name_opt(decl.as_constraint),
                    super_constraint: Self::extract_type_name_opt(decl.super_constraint),
                    type_: Self::extract_type_name(decl.type_),
                    is_ctx: decl.is_ctx,
                    attributes: Self::get_attributes(decl.attributes, ""),
                    internal: decl.internal,
                    docs_url: Self::str_or_empty(decl.docs_url),
                })
                .collect();
            ExtDeclTypeDefVec { vec: consts }
        }

        pub fn get_file(parsed_file: &ParsedFile<'_>) -> ExtDeclFile {
            ExtDeclFile {
                disable_xhp_element_mangling: parsed_file.disable_xhp_element_mangling,
                has_first_pass_parse_errors: parsed_file.has_first_pass_parse_errors,
                is_strict: parsed_file.mode.is_some_and(|x| x == Mode::Mstrict),
                typedefs: Self::get_file_typedefs(parsed_file, "").vec,
                functions: Self::get_file_funcs(parsed_file, "").vec,
                constants: Self::get_file_consts(parsed_file, "").vec,
                file_attributes: Self::get_file_attributes(parsed_file, "").vec,
                modules: Self::get_file_modules(parsed_file, "").vec,
                classes: Self::get_classes(parsed_file).vec,
            }
        }

        //pub fn get_method(parsed_file: &ParsedFile<'_>, name: &str) -> Option<ExtDeclClass> {
        pub fn get_class_methods(
            parsed_file: &ParsedFile<'_>,
            kls: &str,
            name: &str,
        ) -> ExtDeclMethodVec {
            let class_opt = Self::find_class(parsed_file, kls);
            let vec = match class_opt {
                Some(cls) => Self::get_methods(cls.methods, name),
                None => Vec::new(),
            };
            ExtDeclMethodVec { vec }
        }

        pub fn get_class_smethods(
            parsed_file: &ParsedFile<'_>,
            kls: &str,
            name: &str,
        ) -> ExtDeclMethodVec {
            let class_opt = Self::find_class(parsed_file, kls);
            let vec = match class_opt {
                Some(cls) => Self::get_methods(cls.static_methods, name),
                None => Vec::new(),
            };
            ExtDeclMethodVec { vec }
        }

        pub fn get_class_consts(
            parsed_file: &ParsedFile<'_>,
            kls: &str,
            name: &str,
        ) -> ExtDeclClassConstVec {
            let class_opt = Self::find_class(parsed_file, kls);
            let vec = match class_opt {
                Some(cls) => Self::get_consts(cls.consts, name),
                None => Vec::new(),
            };
            ExtDeclClassConstVec { vec }
        }

        pub fn get_class_typeconsts(
            parsed_file: &ParsedFile<'_>,
            kls: &str,
            name: &str,
        ) -> ExtDeclClassTypeConstVec {
            let class_opt = Self::find_class(parsed_file, kls);
            let vec = match class_opt {
                Some(cls) => Self::get_typeconsts(cls.typeconsts, name),
                None => Vec::new(),
            };
            ExtDeclClassTypeConstVec { vec }
        }

        pub fn get_class_props(
            parsed_file: &ParsedFile<'_>,
            kls: &str,
            name: &str,
        ) -> ExtDeclPropVec {
            let class_opt = Self::find_class(parsed_file, kls);
            let vec = match class_opt {
                Some(cls) => Self::get_props(cls.props, name),
                None => Vec::new(),
            };
            ExtDeclPropVec { vec }
        }

        pub fn get_class_sprops(
            parsed_file: &ParsedFile<'_>,
            kls: &str,
            name: &str,
        ) -> ExtDeclPropVec {
            let class_opt = Self::find_class(parsed_file, kls);
            let vec = match class_opt {
                Some(cls) => Self::get_props(cls.sprops, name),
                None => Vec::new(),
            };
            ExtDeclPropVec { vec }
        }

        pub fn get_class_attributes(
            parsed_file: &ParsedFile<'_>,
            kls: &str,
            name: &str,
        ) -> ExtDeclAttributeVec {
            let class_opt = Self::find_class(parsed_file, kls);
            let vec = match class_opt {
                Some(cls) => Self::get_attributes(cls.user_attributes, name),
                None => Vec::new(),
            };
            ExtDeclAttributeVec { vec }
        }

        pub fn get_classes(parsed_file: &ParsedFile<'_>) -> ExtDeclClassVec {
            let classes = parsed_file
                .decls
                .classes()
                .map(|(_kls, decl)| Self::get_class_impl(decl))
                .collect();
            ExtDeclClassVec { vec: classes }
        }

        pub fn get_class(parsed_file: &ParsedFile<'_>, name: &str) -> Option<ExtDeclClass> {
            let class_opt = Self::find_class(parsed_file, name);
            Some(Self::get_class_impl(class_opt?))
        }

        // ========== Complex fields ==========

        fn get_class_impl(class: &ShallowClass<'_>) -> ExtDeclClass {
            ExtDeclClass {
                kind: Self::enum_class_kind(class.kind), // ClassishKind (cls, iface, trait, enum)
                name: Self::fmt_type(class.name.1),

                // Optional strings
                module: Self::id_or_empty(class.module),
                docs_url: Self::str_or_empty(class.docs_url),
                enum_type: Self::get_enum_type(class.enum_type),

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
                tparams: Self::get_typed_params(class.tparams),
                where_constraints: Self::get_where_contraints(class.where_constraints),

                // Implementation
                extends: Self::extract_type_name_vec(class.extends),
                uses: Self::extract_type_name_vec(class.uses),
                implements: Self::extract_type_name_vec(class.implements),
                require_extends: Self::extract_type_name_vec(class.req_extends),
                require_implements: Self::extract_type_name_vec(class.req_implements),
                require_class: Self::extract_type_name_vec(class.req_class),
                xhp_attr_uses: Self::extract_type_name_vec(class.xhp_attr_uses),

                // Nested Types
                user_attributes: Self::get_attributes(class.user_attributes, ""),
                methods: Self::get_methods(class.methods, ""),
                static_methods: Self::get_methods(class.static_methods, ""),
                constructor: Self::get_method_opt(class.constructor),
                consts: Self::get_consts(class.consts, ""),
                typeconsts: Self::get_typeconsts(class.typeconsts, ""),
                props: Self::get_props(class.props, ""),
                sprops: Self::get_props(class.sprops, ""),
                /*
                    Not supported yet
                    pub xhp_enum_values: s_map::SMap<'a, &'a [ast_defs::XhpEnumValue<'a>]>,
                */
            }
        }

        fn get_props(props: &[&ShallowProp<'_>], name: &str) -> Vec<ExtDeclProp> {
            props
                .iter()
                .filter(|prop| name.is_empty() || prop.name.1 == name)
                .map(|prop| ExtDeclProp {
                    name: prop.name.1.to_string(),
                    visibility: Self::enum_visibility(prop.visibility),
                    type_: Self::extract_type_name(prop.type_),
                    flags: prop.flags.bits(),
                    // prop.xhp_attr - not supported yet
                })
                .collect()
        }

        fn get_method_params(params: &FunParams<'_>) -> Vec<ExtDeclMethodParam> {
            params
                .iter()
                .map(|p| ExtDeclMethodParam {
                    name: Self::str_or_empty(p.name),
                    type_: Self::extract_type_name(p.type_.type_),
                    enforced_type: p.type_.enforced == Enforcement::Enforced,
                    flags: p.flags.bits(),
                })
                .collect()
        }

        fn get_method_opt(method: Option<&ShallowMethod<'_>>) -> Vec<ExtDeclMethod> {
            let Some(method) = method else {
                return vec![];
            };
            vec![Self::get_method(method)]
        }

        fn get_method(meth: &ShallowMethod<'_>) -> ExtDeclMethod {
            ExtDeclMethod {
                name: Self::fmt_type(meth.name.1),
                type_: Self::extract_type_name(meth.type_),
                visibility: Self::enum_visibility(meth.visibility),
                attributes: Self::get_attributes(meth.attributes, ""),
                flags: meth.flags.bits(),
                signature: Self::get_signature(meth.type_.1),
            }
        }

        fn get_methods(methods: &[&ShallowMethod<'_>], name: &str) -> Vec<ExtDeclMethod> {
            methods
                .iter()
                .filter(|meth| name.is_empty() || meth.name.1 == name)
                .map(|meth| Self::get_method(meth))
                .collect()
        }

        fn get_typeconsts(
            typeconsts: &[&ShallowTypeconst<'_>],
            name: &str,
        ) -> Vec<ExtDeclClassTypeConst> {
            typeconsts
                .iter()
                .filter(|tc| name.is_empty() || tc.name.1 == name)
                .map(|tc| {
                    let kind = match &tc.kind {
                        Typeconst::TCAbstract(AbstractTypeconst { default, .. }) => {
                            if default.is_none() {
                                String::new()
                            } else {
                                Self::extract_type_name(default.unwrap())
                            }
                        }
                        Typeconst::TCConcrete(ConcreteTypeconst { tc_type, .. }) => {
                            Self::extract_type_name(tc_type)
                        }
                    };

                    ExtDeclClassTypeConst {
                        name: Self::fmt_type(tc.name.1),
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
                    name: Self::fmt_type(c.name.1),
                    type_: Self::extract_type_name(c.type_),
                    is_abstract: c.abstract_ != ClassConstKind::CCConcrete,
                })
                .collect()
        }

        fn get_enum_type(
            enum_type: Option<&oxidized_by_ref::shallow_decl_defs::EnumType<'_>>,
        ) -> Vec<ExtDeclEnumType> {
            let Some(en) = enum_type else {
                return vec![];
            };
            vec![ExtDeclEnumType {
                base: Self::extract_type_name(en.base),
                constraint: Self::extract_type_name_opt(en.constraint),
                includes: Self::extract_type_name_vec(en.includes),
            }]
        }

        fn get_where_contraints(
            arr: &[&oxidized_by_ref::shallow_decl_defs::WhereConstraint<'_>],
        ) -> Vec<ExtDeclTypeConstraint> {
            arr.iter()
                .map(|w| ExtDeclTypeConstraint {
                    type_: Self::extract_type_name(w.0),
                    kind: Self::enum_constraint_kind(w.1),
                })
                .collect()
        }

        fn get_typed_params(arr: &[&Tparam<'_>]) -> Vec<ExtDeclTparam> {
            arr.iter()
                .map(|t| ExtDeclTparam {
                    variance: Self::enum_variance(t.variance),
                    name: Self::fmt_type(t.name.1),
                    tparams: Self::get_typed_params(t.tparams),
                    constraints: t
                        .constraints
                        .iter()
                        .map(|c| ExtDeclTypeConstraint {
                            kind: Self::enum_constraint_kind(c.0),
                            type_: Self::extract_type_name(c.1),
                        })
                        .collect(),
                    reified: Self::enum_reify_kind(t.reified),
                    user_attributes: Self::get_attributes(t.user_attributes, ""),
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
                    name: Self::fmt_type(t.name.1),
                    args: t
                        .params
                        .iter()
                        .map(|arg| match arg {
                            UserAttributeParam::Classname(cn) => Self::fmt_type(cn),
                            UserAttributeParam::String(s) => s.to_string(),
                            UserAttributeParam::Int(i) => i.to_string(),
                            _ => String::from("__ext_decl_unknown"),
                        })
                        .collect(),
                })
                .collect()
        }

        fn extract_module_refs(refs: Option<&[ModuleReference<'_>]>) -> Vec<String> {
            if refs.is_none() {
                return vec![];
            }
            refs.unwrap()
                .iter()
                .map(|mref| match mref {
                    ModuleReference::MRGlobal => String::new(),
                    ModuleReference::MRPrefix(m) => m.to_string(),
                    ModuleReference::MRExact(m) => m.to_string(),
                })
                .collect()
        }

        fn get_signature(ty: Ty_<'_>) -> Vec<ExtDeclSignature> {
            match ty {
                oxidized_by_ref::typing_defs::Ty_::Tfun(ft) => {
                    let implicit_params = match ft.implicit_params.capability {
                        oxidized_by_ref::typing_defs_core::Capability::CapDefaults(_) => {
                            String::new()
                        }
                        oxidized_by_ref::typing_defs_core::Capability::CapTy(x) => {
                            Self::extract_type_name(x)
                        }
                    };

                    vec![ExtDeclSignature {
                        tparams: Self::get_typed_params(ft.tparams),
                        where_constraints: Self::get_where_contraints(ft.where_constraints),
                        return_type: Self::extract_type_name(ft.ret.type_),
                        return_enforced: ft.ret.enforced == Enforcement::Enforced,
                        flags: ft.flags.bits(),
                        params: Self::get_method_params(ft.params),
                        implicit_params,
                        cross_package: Self::str_or_empty(ft.cross_package),
                    }]
                }
                _ => Vec::new(),
            }
        }

        // ========== Types ==========

        fn extract_type_name_opt(t: Option<&Ty<'_>>) -> String {
            let Some(ty) = t else { return String::new() };
            Self::extract_type_name(ty)
        }

        fn extract_type_name(t: &Ty<'_>) -> String {
            let converted_ty: ty::decl::Ty<BReason> = (&t).clone().into();
            converted_ty.to_string()
        }

        fn extract_type_name_vec(arr: &[&Ty<'_>]) -> Vec<String> {
            arr.iter().map(|t| Self::extract_type_name(t)).collect()
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
            Self::str_or_empty(x.map(|y| y.name()))
        }

        fn str_or_empty(x: Option<&str>) -> String {
            x.map_or_else(String::new, |y| y.to_string())
        }

        // ========== ENUMS ==========
        fn enum_class_kind(x: oxidized_by_ref::ast_defs::ClassishKind) -> String {
            match x {
                oxidized_by_ref::ast_defs::ClassishKind::Cclass(_) => String::from("class"),
                oxidized_by_ref::ast_defs::ClassishKind::Cinterface => String::from("interface"),
                oxidized_by_ref::ast_defs::ClassishKind::Ctrait => String::from("trait"),
                oxidized_by_ref::ast_defs::ClassishKind::Cenum => String::from("enum"),
                oxidized_by_ref::ast_defs::ClassishKind::CenumClass(_) => {
                    String::from("enum_class")
                }
            }
        }

        fn enum_variance(x: oxidized::ast_defs::Variance) -> String {
            match x {
                oxidized::ast_defs::Variance::Covariant => String::from("covariant"),
                oxidized::ast_defs::Variance::Contravariant => String::from("contravariant"),
                oxidized::ast_defs::Variance::Invariant => String::from("invariant"),
            }
        }

        fn enum_reify_kind(x: oxidized::ast_defs::ReifyKind) -> String {
            match x {
                oxidized::ast_defs::ReifyKind::Erased => String::from("erased"),
                oxidized::ast_defs::ReifyKind::SoftReified => String::from("soft_reified"),
                oxidized::ast_defs::ReifyKind::Reified => String::from("reified"),
            }
        }

        fn enum_constraint_kind(x: oxidized::ast_defs::ConstraintKind) -> String {
            match x {
                oxidized::ast_defs::ConstraintKind::ConstraintAs => String::from("constraint_as"),
                oxidized::ast_defs::ConstraintKind::ConstraintEq => String::from("constraint_eq"),
                oxidized::ast_defs::ConstraintKind::ConstraintSuper => {
                    String::from("constraint_super")
                }
            }
        }

        fn enum_visibility(x: oxidized_by_ref::ast_defs::Visibility) -> String {
            match x {
                oxidized_by_ref::ast_defs::Visibility::Private => String::from("private"),
                oxidized_by_ref::ast_defs::Visibility::Public => String::from("public"),
                oxidized_by_ref::ast_defs::Visibility::Protected => String::from("protected"),
                oxidized_by_ref::ast_defs::Visibility::Internal => String::from("internal"),
            }
        }

        fn enum_typedef_visibility(x: oxidized_by_ref::ast_defs::TypedefVisibility) -> String {
            match x {
                oxidized_by_ref::ast_defs::TypedefVisibility::Transparent => {
                    String::from("transparent")
                }
                oxidized_by_ref::ast_defs::TypedefVisibility::Opaque => String::from("opaque"),
                oxidized_by_ref::ast_defs::TypedefVisibility::OpaqueModule => {
                    String::from("opaque_module")
                }
                oxidized_by_ref::ast_defs::TypedefVisibility::CaseType => String::from("case_type"),
            }
        }
    }
}
