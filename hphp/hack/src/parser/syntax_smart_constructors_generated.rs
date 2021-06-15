/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 */
use parser_core_types::{
    syntax::*,
    token_factory::TokenFactory,
};
use smart_constructors::{NoState, SmartConstructors};
use crate::StateType;

pub trait SyntaxSmartConstructors<S: SyntaxType<State>, TF: TokenFactory<Token = S::Token>, State = NoState>:
    SmartConstructors<State = State, R=S, TF = TF>
where
    State: StateType<S>,
{
    fn make_missing(&mut self, offset: usize) -> Self::R {
        let r = Self::R::make_missing(self.state_mut(), offset);
        self.state_mut().next(&[]);
        r
    }

    fn make_token(&mut self, arg: <Self::TF as TokenFactory>::Token) -> Self::R {
        let r = Self::R::make_token(self.state_mut(), arg);
        self.state_mut().next(&[]);
        r
    }

    fn make_list(&mut self, items: Vec<Self::R>, offset: usize) -> Self::R {
        if items.is_empty() {
            <Self as SyntaxSmartConstructors<S, TF, State>>::make_missing(self, offset)
        } else {
            let item_refs: Vec<_> = items.iter().collect();
            self.state_mut().next(&item_refs);
            Self::R::make_list(self.state_mut(), items, offset)
        }
    }

    fn make_end_of_file(&mut self, arg0 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0]);
        Self::R::make_end_of_file(self.state_mut(), arg0)
    }

    fn make_script(&mut self, arg0 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0]);
        Self::R::make_script(self.state_mut(), arg0)
    }

    fn make_qualified_name(&mut self, arg0 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0]);
        Self::R::make_qualified_name(self.state_mut(), arg0)
    }

    fn make_simple_type_specifier(&mut self, arg0 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0]);
        Self::R::make_simple_type_specifier(self.state_mut(), arg0)
    }

    fn make_literal_expression(&mut self, arg0 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0]);
        Self::R::make_literal_expression(self.state_mut(), arg0)
    }

    fn make_prefixed_string_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_prefixed_string_expression(self.state_mut(), arg0, arg1)
    }

    fn make_prefixed_code_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_prefixed_code_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_variable_expression(&mut self, arg0 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0]);
        Self::R::make_variable_expression(self.state_mut(), arg0)
    }

    fn make_pipe_variable_expression(&mut self, arg0 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0]);
        Self::R::make_pipe_variable_expression(self.state_mut(), arg0)
    }

    fn make_file_attribute_specification(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_file_attribute_specification(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_enum_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9]);
        Self::R::make_enum_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_enum_use(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_enum_use(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_enumerator(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_enumerator(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_enum_class_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9, &arg10]);
        Self::R::make_enum_class_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
    }

    fn make_enum_class_enumerator(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_enum_class_enumerator(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_record_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        Self::R::make_record_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_record_field(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_record_field(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_alias_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7]);
        Self::R::make_alias_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
    }

    fn make_context_alias_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7]);
        Self::R::make_context_alias_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
    }

    fn make_property_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_property_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_property_declarator(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_property_declarator(self.state_mut(), arg0, arg1)
    }

    fn make_namespace_declaration(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_namespace_declaration(self.state_mut(), arg0, arg1)
    }

    fn make_namespace_declaration_header(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_namespace_declaration_header(self.state_mut(), arg0, arg1)
    }

    fn make_namespace_body(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_namespace_body(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_namespace_empty_body(&mut self, arg0 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0]);
        Self::R::make_namespace_empty_body(self.state_mut(), arg0)
    }

    fn make_namespace_use_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_namespace_use_declaration(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_namespace_group_use_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        Self::R::make_namespace_group_use_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_namespace_use_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_namespace_use_clause(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_function_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_function_declaration(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_function_declaration_header(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9, &arg10, &arg11]);
        Self::R::make_function_declaration_header(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_contexts(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_contexts(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_where_clause(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_where_clause(self.state_mut(), arg0, arg1)
    }

    fn make_where_constraint(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_where_constraint(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_methodish_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_methodish_declaration(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_methodish_trait_resolution(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_methodish_trait_resolution(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_classish_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9, &arg10, &arg11]);
        Self::R::make_classish_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_classish_body(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_classish_body(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_trait_use_precedence_item(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_trait_use_precedence_item(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_trait_use_alias_item(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_trait_use_alias_item(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_trait_use_conflict_resolution(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_trait_use_conflict_resolution(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_trait_use(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_trait_use(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_require_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_require_clause(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_const_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_const_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_constant_declarator(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_constant_declarator(self.state_mut(), arg0, arg1)
    }

    fn make_type_const_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9]);
        Self::R::make_type_const_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_context_const_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        Self::R::make_context_const_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_decorated_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_decorated_expression(self.state_mut(), arg0, arg1)
    }

    fn make_parameter_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        Self::R::make_parameter_declaration(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_variadic_parameter(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_variadic_parameter(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_old_attribute_specification(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_old_attribute_specification(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_attribute_specification(&mut self, arg0 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0]);
        Self::R::make_attribute_specification(self.state_mut(), arg0)
    }

    fn make_attribute(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_attribute(self.state_mut(), arg0, arg1)
    }

    fn make_inclusion_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_inclusion_expression(self.state_mut(), arg0, arg1)
    }

    fn make_inclusion_directive(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_inclusion_directive(self.state_mut(), arg0, arg1)
    }

    fn make_compound_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_compound_statement(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_expression_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_expression_statement(self.state_mut(), arg0, arg1)
    }

    fn make_markup_section(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_markup_section(self.state_mut(), arg0, arg1)
    }

    fn make_markup_suffix(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_markup_suffix(self.state_mut(), arg0, arg1)
    }

    fn make_unset_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_unset_statement(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_using_statement_block_scoped(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        Self::R::make_using_statement_block_scoped(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_function_scoped(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_using_statement_function_scoped(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_while_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_while_statement(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_if_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        Self::R::make_if_statement(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_elseif_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_elseif_clause(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_else_clause(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_else_clause(self.state_mut(), arg0, arg1)
    }

    fn make_try_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_try_statement(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_catch_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        Self::R::make_catch_clause(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_finally_clause(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_finally_clause(self.state_mut(), arg0, arg1)
    }

    fn make_do_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        Self::R::make_do_statement(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_for_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        Self::R::make_for_statement(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_foreach_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9]);
        Self::R::make_foreach_statement(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_switch_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        Self::R::make_switch_statement(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_switch_section(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_switch_section(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_switch_fallthrough(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_switch_fallthrough(self.state_mut(), arg0, arg1)
    }

    fn make_case_label(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_case_label(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_default_label(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_default_label(self.state_mut(), arg0, arg1)
    }

    fn make_return_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_return_statement(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_yield_break_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_yield_break_statement(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_throw_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_throw_statement(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_break_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_break_statement(self.state_mut(), arg0, arg1)
    }

    fn make_continue_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_continue_statement(self.state_mut(), arg0, arg1)
    }

    fn make_echo_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_echo_statement(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_concurrent_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_concurrent_statement(self.state_mut(), arg0, arg1)
    }

    fn make_simple_initializer(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_simple_initializer(self.state_mut(), arg0, arg1)
    }

    fn make_anonymous_class(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        Self::R::make_anonymous_class(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_anonymous_function(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9, &arg10, &arg11]);
        Self::R::make_anonymous_function(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_anonymous_function_use_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_anonymous_function_use_clause(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_lambda_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_lambda_expression(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_lambda_signature(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        Self::R::make_lambda_signature(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_cast_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_cast_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_scope_resolution_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_scope_resolution_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_member_selection_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_member_selection_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_safe_member_selection_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_safe_member_selection_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_embedded_member_selection_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_embedded_member_selection_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_yield_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_yield_expression(self.state_mut(), arg0, arg1)
    }

    fn make_prefix_unary_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_prefix_unary_expression(self.state_mut(), arg0, arg1)
    }

    fn make_postfix_unary_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_postfix_unary_expression(self.state_mut(), arg0, arg1)
    }

    fn make_binary_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_binary_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_is_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_is_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_as_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_as_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_nullable_as_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_nullable_as_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_conditional_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_conditional_expression(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_eval_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_eval_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_isset_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_isset_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_function_call_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        Self::R::make_function_call_expression(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_function_pointer_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_function_pointer_expression(self.state_mut(), arg0, arg1)
    }

    fn make_parenthesized_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_parenthesized_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_braced_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_braced_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_et_splice_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_et_splice_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_embedded_braced_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_embedded_braced_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_list_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_list_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_collection_literal_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_collection_literal_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_object_creation_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_object_creation_expression(self.state_mut(), arg0, arg1)
    }

    fn make_constructor_call(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_constructor_call(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_record_creation_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_record_creation_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_darray_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_darray_intrinsic_expression(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_dictionary_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_dictionary_intrinsic_expression(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_keyset_intrinsic_expression(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_varray_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_varray_intrinsic_expression(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_vector_intrinsic_expression(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_element_initializer(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_element_initializer(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_subscript_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_subscript_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_embedded_subscript_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_embedded_subscript_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_awaitable_creation_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_awaitable_creation_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_xhp_children_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_xhp_children_declaration(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_xhp_children_parenthesized_list(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_xhp_children_parenthesized_list(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_xhp_category_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_xhp_category_declaration(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_xhp_enum_type(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_xhp_enum_type(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_xhp_lateinit(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_xhp_lateinit(self.state_mut(), arg0, arg1)
    }

    fn make_xhp_required(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_xhp_required(self.state_mut(), arg0, arg1)
    }

    fn make_xhp_class_attribute_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_xhp_class_attribute_declaration(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_xhp_class_attribute(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_xhp_class_attribute(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_xhp_simple_class_attribute(&mut self, arg0 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0]);
        Self::R::make_xhp_simple_class_attribute(self.state_mut(), arg0)
    }

    fn make_xhp_simple_attribute(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_xhp_simple_attribute(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_xhp_spread_attribute(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_xhp_spread_attribute(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_xhp_open(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_xhp_open(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_xhp_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_xhp_expression(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_xhp_close(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_xhp_close(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_type_constant(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_type_constant(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_vector_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_vector_type_specifier(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_keyset_type_specifier(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_tuple_type_explicit_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_tuple_type_explicit_specifier(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_varray_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_varray_type_specifier(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_function_ctx_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_function_ctx_type_specifier(self.state_mut(), arg0, arg1)
    }

    fn make_type_parameter(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        Self::R::make_type_parameter(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_type_constraint(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_type_constraint(self.state_mut(), arg0, arg1)
    }

    fn make_context_constraint(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_context_constraint(self.state_mut(), arg0, arg1)
    }

    fn make_darray_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        Self::R::make_darray_type_specifier(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_dictionary_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_dictionary_type_specifier(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_closure_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9, &arg10]);
        Self::R::make_closure_type_specifier(self.state_mut(), arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
    }

    fn make_closure_parameter_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_closure_parameter_type_specifier(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_classname_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_classname_type_specifier(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_field_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_field_specifier(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_field_initializer(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_field_initializer(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_shape_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3, &arg4]);
        Self::R::make_shape_type_specifier(self.state_mut(), arg0, arg1, arg2, arg3, arg4)
    }

    fn make_shape_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_shape_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_tuple_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2, &arg3]);
        Self::R::make_tuple_expression(self.state_mut(), arg0, arg1, arg2, arg3)
    }

    fn make_generic_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_generic_type_specifier(self.state_mut(), arg0, arg1)
    }

    fn make_nullable_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_nullable_type_specifier(self.state_mut(), arg0, arg1)
    }

    fn make_like_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_like_type_specifier(self.state_mut(), arg0, arg1)
    }

    fn make_soft_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_soft_type_specifier(self.state_mut(), arg0, arg1)
    }

    fn make_attributized_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_attributized_specifier(self.state_mut(), arg0, arg1)
    }

    fn make_reified_type_argument(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_reified_type_argument(self.state_mut(), arg0, arg1)
    }

    fn make_type_arguments(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_type_arguments(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_type_parameters(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_type_parameters(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_tuple_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_tuple_type_specifier(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_union_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_union_type_specifier(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_intersection_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_intersection_type_specifier(self.state_mut(), arg0, arg1, arg2)
    }

    fn make_error(&mut self, arg0 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0]);
        Self::R::make_error(self.state_mut(), arg0)
    }

    fn make_list_item(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1]);
        Self::R::make_list_item(self.state_mut(), arg0, arg1)
    }

    fn make_enum_class_label_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        self.state_mut().next(&[&arg0, &arg1, &arg2]);
        Self::R::make_enum_class_label_expression(self.state_mut(), arg0, arg1, arg2)
    }

}
