// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::fmt;

use super::folded::FoldedClass;
use super::shallow::ShallowClass;
use crate::reason::Reason;

// Our Class structs have a lot of fields, but in a lot of cases, most of them
// will have empty or default values, making Debug output very noisy. These
// manual Debug impls omit fields with empty values, hopefully making the Debug
// output easier to read.

impl<R: Reason> fmt::Debug for ShallowClass<R> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let ShallowClass {
            mode,
            is_final,
            is_abstract,
            is_internal,
            is_xhp,
            has_xhp_keyword,
            kind,
            module,
            name,
            tparams,
            where_constraints,
            extends,
            uses,
            xhp_attr_uses,
            xhp_enum_values,
            xhp_marked_empty,
            req_extends,
            req_implements,
            req_class,
            implements,
            support_dynamic_type,
            consts,
            typeconsts,
            props,
            static_props,
            constructor,
            static_methods,
            methods,
            user_attributes,
            enum_type,
            docs_url,
        } = self;

        let mut s = f.debug_struct("ShallowClass");

        if *mode != oxidized::file_info::Mode::Mstrict {
            s.field("mode", mode);
        }
        if *is_final {
            s.field("is_final", is_final);
        }
        if *is_abstract {
            s.field("is_abstract", is_abstract);
        }
        if *is_internal {
            s.field("is_internal", is_internal);
        }
        if *is_xhp {
            s.field("is_xhp", is_xhp);
        }
        if *has_xhp_keyword {
            s.field("has_xhp_keyword", has_xhp_keyword);
        }

        s.field("kind", kind);

        if let Some(module) = module {
            s.field("module", module);
        }

        s.field("name", name);

        if !tparams.is_empty() {
            s.field("tparams", tparams);
        }
        if !where_constraints.is_empty() {
            s.field("where_constraints", where_constraints);
        }
        if !extends.is_empty() {
            s.field("extends", extends);
        }
        if !uses.is_empty() {
            s.field("uses", uses);
        }
        if !xhp_attr_uses.is_empty() {
            s.field("xhp_attr_uses", xhp_attr_uses);
        }
        if !xhp_enum_values.is_empty() {
            s.field("xhp_enum_values", xhp_enum_values);
        }
        if *xhp_marked_empty {
            s.field("xhp_marked_empty", xhp_marked_empty);
        }
        if !req_extends.is_empty() {
            s.field("req_extends", req_extends);
        }
        if !req_implements.is_empty() {
            s.field("req_implements", req_implements);
        }
        if !req_class.is_empty() {
            s.field("req_class", req_class);
        }
        if !implements.is_empty() {
            s.field("implements", implements);
        }
        if *support_dynamic_type {
            s.field("support_dynamic_type", support_dynamic_type);
        }
        if !consts.is_empty() {
            s.field("consts", consts);
        }
        if !typeconsts.is_empty() {
            s.field("typeconsts", typeconsts);
        }
        if !props.is_empty() {
            s.field("props", props);
        }
        if !static_props.is_empty() {
            s.field("static_props", static_props);
        }
        if let Some(constructor) = constructor {
            s.field("constructor", constructor);
        }
        if !static_methods.is_empty() {
            s.field("static_methods", static_methods);
        }
        if !methods.is_empty() {
            s.field("methods", methods);
        }
        if !user_attributes.is_empty() {
            s.field("user_attributes", user_attributes);
        }
        if let Some(enum_type) = enum_type {
            s.field("enum_type", enum_type);
        }

        if let Some(docs_url) = docs_url {
            s.field("docs_url", docs_url);
        }

        s.finish()
    }
}

impl<R: Reason> fmt::Debug for FoldedClass<R> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let FoldedClass {
            name,
            pos,
            kind,
            is_final,
            is_const,
            is_internal,
            is_xhp,
            has_xhp_keyword,
            support_dynamic_type,
            enum_type,
            module,
            is_module_level_trait,
            tparams,
            where_constraints,
            substs,
            ancestors,
            props,
            static_props,
            methods,
            static_methods,
            constructor,
            consts,
            type_consts,
            xhp_enum_values,
            xhp_marked_empty,
            extends,
            xhp_attr_deps,
            req_ancestors,
            req_ancestors_extends,
            req_class_ancestors,
            sealed_whitelist,
            deferred_init_members,
            decl_errors,
            docs_url,
            allow_multiple_instantiations,
        } = self;

        let mut s = f.debug_struct("FoldedClass");

        s.field("name", name);

        if std::mem::size_of::<R::Pos>() != 0 {
            s.field("pos", pos);
        }

        s.field("kind", kind);

        if *is_final {
            s.field("is_final", is_final);
        }
        if *is_const {
            s.field("is_const", is_const);
        }
        if *is_internal {
            s.field("is_internal", is_internal);
        }
        if *is_xhp {
            s.field("is_xhp", is_xhp);
        }
        if *has_xhp_keyword {
            s.field("has_xhp_keyword", has_xhp_keyword);
        }
        if *support_dynamic_type {
            s.field("support_dynamic_type", support_dynamic_type);
        }
        if let Some(enum_type) = enum_type {
            s.field("enum_type", enum_type);
        }
        if let Some(module) = module {
            s.field("module", module);
        }
        if *is_module_level_trait {
            s.field("is_module_level_trait", is_module_level_trait);
        }
        if !tparams.is_empty() {
            s.field("tparams", tparams);
        }
        if !where_constraints.is_empty() {
            s.field("where_constraints", where_constraints);
        }

        if !substs.is_empty() {
            s.field("substs", substs);
        }
        if !ancestors.is_empty() {
            s.field("ancestors", ancestors);
        }
        if !props.is_empty() {
            s.field("props", props);
        }
        if !static_props.is_empty() {
            s.field("static_props", static_props);
        }
        if !methods.is_empty() {
            s.field("methods", methods);
        }
        if !static_methods.is_empty() {
            s.field("static_methods", static_methods);
        }
        if let Some(elt) = &constructor.elt {
            s.field("constructor", &(elt, constructor.consistency));
        }
        if !consts.is_empty() {
            s.field("consts", consts);
        }
        if !type_consts.is_empty() {
            s.field("type_consts", type_consts);
        }
        if !xhp_enum_values.is_empty() {
            s.field("xhp_enum_values", xhp_enum_values);
        }
        if *xhp_marked_empty {
            s.field("xhp_marked_empty", xhp_marked_empty);
        }
        if !extends.is_empty() {
            s.field("extends", extends);
        }
        if !xhp_attr_deps.is_empty() {
            s.field("xhp_attr_deps", xhp_attr_deps);
        }
        if !req_ancestors.is_empty() {
            s.field("req_ancestors", req_ancestors);
        }
        if !req_ancestors_extends.is_empty() {
            s.field("req_ancestors_extends", req_ancestors_extends);
        }
        if !req_class_ancestors.is_empty() {
            s.field("req_class_ancestors", req_class_ancestors);
        }
        if let Some(sealed_whitelist) = sealed_whitelist {
            s.field("sealed_whitelist", sealed_whitelist);
        }
        if !deferred_init_members.is_empty() {
            s.field("deferred_init_members", deferred_init_members);
        }
        if !decl_errors.is_empty() {
            s.field("decl_errors", decl_errors);
        }
        if let Some(docs_url) = docs_url {
            s.field("docs_url", docs_url);
        }
        if *allow_multiple_instantiations {
            s.field(
                "allow_multiple_instantiations",
                allow_multiple_instantiations,
            );
        }

        s.finish()
    }
}
