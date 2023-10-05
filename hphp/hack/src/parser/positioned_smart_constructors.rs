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
    lexable_token::LexableToken,
    token_factory::TokenFactory,
};
use smart_constructors::SmartConstructors;
use syntax_smart_constructors::{SyntaxSmartConstructors, StateType};

#[derive(Clone)]
pub struct PositionedSmartConstructors<S, TF, St: StateType<S>> {
    pub state: St,
    token_factory: TF,
    phantom_s: std::marker::PhantomData<S>,
}

impl<S, TF, St: StateType<S>> PositionedSmartConstructors<S, TF, St> {
    pub fn new(state: St, token_factory: TF) -> Self {
        Self { state, token_factory, phantom_s: std::marker::PhantomData }
    }
}

impl<S, TF, St> SyntaxSmartConstructors<S, TF, St> for PositionedSmartConstructors<S, TF, St>
where
    TF: TokenFactory<Token = S::Token>,
    St: StateType<S>,
    S: SyntaxType<St> + Clone,
    S::Token: LexableToken,
{}

impl<S, TF, St> SmartConstructors for PositionedSmartConstructors<S, TF, St>
where
    TF: TokenFactory<Token = S::Token>,
    S::Token: LexableToken,
    S: SyntaxType<St> + Clone,
    St: StateType<S>,
{
    type Factory = TF;
    type State = St;
    type Output = S;

    fn state_mut(&mut self) -> &mut St {
       &mut self.state
    }

    fn into_state(self) -> St {
      self.state
    }

    fn token_factory_mut(&mut self) -> &mut Self::Factory {
        &mut self.token_factory
    }

    fn make_missing(&mut self, offset: usize) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_missing(self, offset)
    }

    fn make_token(&mut self, offset: <Self::Factory as TokenFactory>::Token) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_token(self, offset)
    }

    fn make_list(&mut self, lst: Vec<Self::Output>, offset: usize) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_list(self, lst, offset)
    }
    fn make_end_of_file(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_end_of_file(self, arg0)
    }

    fn make_script(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_script(self, arg0)
    }

    fn make_qualified_name(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_qualified_name(self, arg0)
    }

    fn make_module_name(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_module_name(self, arg0)
    }

    fn make_simple_type_specifier(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_simple_type_specifier(self, arg0)
    }

    fn make_literal_expression(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_literal_expression(self, arg0)
    }

    fn make_prefixed_string_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_prefixed_string_expression(self, arg0, arg1)
    }

    fn make_prefixed_code_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_prefixed_code_expression(self, arg0, arg1, arg2, arg3)
    }

    fn make_variable_expression(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_variable_expression(self, arg0)
    }

    fn make_pipe_variable_expression(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_pipe_variable_expression(self, arg0)
    }

    fn make_file_attribute_specification(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_file_attribute_specification(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_enum_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_enum_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
    }

    fn make_enum_use(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_enum_use(self, arg0, arg1, arg2)
    }

    fn make_enumerator(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_enumerator(self, arg0, arg1, arg2, arg3)
    }

    fn make_enum_class_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output, arg11: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_enum_class_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_enum_class_enumerator(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_enum_class_enumerator(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_alias_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_alias_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_context_alias_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_context_alias_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
    }

    fn make_case_type_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_case_type_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
    }

    fn make_case_type_variant(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_case_type_variant(self, arg0, arg1)
    }

    fn make_property_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_property_declaration(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_property_declarator(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_property_declarator(self, arg0, arg1)
    }

    fn make_namespace_declaration(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_namespace_declaration(self, arg0, arg1)
    }

    fn make_namespace_declaration_header(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_namespace_declaration_header(self, arg0, arg1)
    }

    fn make_namespace_body(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_namespace_body(self, arg0, arg1, arg2)
    }

    fn make_namespace_empty_body(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_namespace_empty_body(self, arg0)
    }

    fn make_namespace_use_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_namespace_use_declaration(self, arg0, arg1, arg2, arg3)
    }

    fn make_namespace_group_use_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_namespace_group_use_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_namespace_use_clause(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_namespace_use_clause(self, arg0, arg1, arg2, arg3)
    }

    fn make_function_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_function_declaration(self, arg0, arg1, arg2)
    }

    fn make_function_declaration_header(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output, arg11: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_function_declaration_header(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_contexts(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_contexts(self, arg0, arg1, arg2)
    }

    fn make_where_clause(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_where_clause(self, arg0, arg1)
    }

    fn make_where_constraint(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_where_constraint(self, arg0, arg1, arg2)
    }

    fn make_methodish_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_methodish_declaration(self, arg0, arg1, arg2, arg3)
    }

    fn make_methodish_trait_resolution(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_methodish_trait_resolution(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_classish_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output, arg11: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_classish_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_classish_body(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_classish_body(self, arg0, arg1, arg2)
    }

    fn make_trait_use(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_trait_use(self, arg0, arg1, arg2)
    }

    fn make_require_clause(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_require_clause(self, arg0, arg1, arg2, arg3)
    }

    fn make_const_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_const_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_constant_declarator(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_constant_declarator(self, arg0, arg1)
    }

    fn make_type_const_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_type_const_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_context_const_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_context_const_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_decorated_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_decorated_expression(self, arg0, arg1)
    }

    fn make_parameter_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_parameter_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_variadic_parameter(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_variadic_parameter(self, arg0, arg1, arg2)
    }

    fn make_old_attribute_specification(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_old_attribute_specification(self, arg0, arg1, arg2)
    }

    fn make_attribute_specification(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_attribute_specification(self, arg0)
    }

    fn make_attribute(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_attribute(self, arg0, arg1)
    }

    fn make_inclusion_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_inclusion_expression(self, arg0, arg1)
    }

    fn make_inclusion_directive(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_inclusion_directive(self, arg0, arg1)
    }

    fn make_compound_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_compound_statement(self, arg0, arg1, arg2)
    }

    fn make_expression_statement(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_expression_statement(self, arg0, arg1)
    }

    fn make_markup_section(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_markup_section(self, arg0, arg1)
    }

    fn make_markup_suffix(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_markup_suffix(self, arg0, arg1)
    }

    fn make_unset_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_unset_statement(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_declare_local_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_declare_local_statement(self, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_block_scoped(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_using_statement_block_scoped(self, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_function_scoped(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_using_statement_function_scoped(self, arg0, arg1, arg2, arg3)
    }

    fn make_while_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_while_statement(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_if_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_if_statement(self, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_else_clause(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_else_clause(self, arg0, arg1)
    }

    fn make_try_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_try_statement(self, arg0, arg1, arg2, arg3)
    }

    fn make_catch_clause(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_catch_clause(self, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_finally_clause(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_finally_clause(self, arg0, arg1)
    }

    fn make_do_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_do_statement(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_for_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_for_statement(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_foreach_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_foreach_statement(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_switch_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_switch_statement(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_switch_section(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_switch_section(self, arg0, arg1, arg2)
    }

    fn make_switch_fallthrough(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_switch_fallthrough(self, arg0, arg1)
    }

    fn make_case_label(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_case_label(self, arg0, arg1, arg2)
    }

    fn make_default_label(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_default_label(self, arg0, arg1)
    }

    fn make_match_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_match_statement(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_match_statement_arm(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_match_statement_arm(self, arg0, arg1, arg2)
    }

    fn make_return_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_return_statement(self, arg0, arg1, arg2)
    }

    fn make_yield_break_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_yield_break_statement(self, arg0, arg1, arg2)
    }

    fn make_throw_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_throw_statement(self, arg0, arg1, arg2)
    }

    fn make_break_statement(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_break_statement(self, arg0, arg1)
    }

    fn make_continue_statement(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_continue_statement(self, arg0, arg1)
    }

    fn make_echo_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_echo_statement(self, arg0, arg1, arg2)
    }

    fn make_concurrent_statement(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_concurrent_statement(self, arg0, arg1)
    }

    fn make_simple_initializer(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_simple_initializer(self, arg0, arg1)
    }

    fn make_anonymous_class(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_anonymous_class(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_anonymous_function(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output, arg11: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_anonymous_function(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_anonymous_function_use_clause(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_anonymous_function_use_clause(self, arg0, arg1, arg2, arg3)
    }

    fn make_variable_pattern(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_variable_pattern(self, arg0)
    }

    fn make_constructor_pattern(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_constructor_pattern(self, arg0, arg1, arg2, arg3)
    }

    fn make_refinement_pattern(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_refinement_pattern(self, arg0, arg1, arg2)
    }

    fn make_lambda_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_lambda_expression(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_lambda_signature(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_lambda_signature(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_cast_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_cast_expression(self, arg0, arg1, arg2, arg3)
    }

    fn make_scope_resolution_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_scope_resolution_expression(self, arg0, arg1, arg2)
    }

    fn make_member_selection_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_member_selection_expression(self, arg0, arg1, arg2)
    }

    fn make_safe_member_selection_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_safe_member_selection_expression(self, arg0, arg1, arg2)
    }

    fn make_embedded_member_selection_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_embedded_member_selection_expression(self, arg0, arg1, arg2)
    }

    fn make_yield_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_yield_expression(self, arg0, arg1)
    }

    fn make_prefix_unary_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_prefix_unary_expression(self, arg0, arg1)
    }

    fn make_postfix_unary_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_postfix_unary_expression(self, arg0, arg1)
    }

    fn make_binary_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_binary_expression(self, arg0, arg1, arg2)
    }

    fn make_is_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_is_expression(self, arg0, arg1, arg2)
    }

    fn make_as_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_as_expression(self, arg0, arg1, arg2)
    }

    fn make_nullable_as_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_nullable_as_expression(self, arg0, arg1, arg2)
    }

    fn make_upcast_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_upcast_expression(self, arg0, arg1, arg2)
    }

    fn make_conditional_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_conditional_expression(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_eval_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_eval_expression(self, arg0, arg1, arg2, arg3)
    }

    fn make_isset_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_isset_expression(self, arg0, arg1, arg2, arg3)
    }

    fn make_nameof_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_nameof_expression(self, arg0, arg1)
    }

    fn make_function_call_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_function_call_expression(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_function_pointer_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_function_pointer_expression(self, arg0, arg1)
    }

    fn make_parenthesized_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_parenthesized_expression(self, arg0, arg1, arg2)
    }

    fn make_braced_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_braced_expression(self, arg0, arg1, arg2)
    }

    fn make_et_splice_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_et_splice_expression(self, arg0, arg1, arg2, arg3)
    }

    fn make_embedded_braced_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_embedded_braced_expression(self, arg0, arg1, arg2)
    }

    fn make_list_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_list_expression(self, arg0, arg1, arg2, arg3)
    }

    fn make_collection_literal_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_collection_literal_expression(self, arg0, arg1, arg2, arg3)
    }

    fn make_object_creation_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_object_creation_expression(self, arg0, arg1)
    }

    fn make_constructor_call(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_constructor_call(self, arg0, arg1, arg2, arg3)
    }

    fn make_darray_intrinsic_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_darray_intrinsic_expression(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_dictionary_intrinsic_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_dictionary_intrinsic_expression(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_intrinsic_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_keyset_intrinsic_expression(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_varray_intrinsic_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_varray_intrinsic_expression(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_intrinsic_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_vector_intrinsic_expression(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_element_initializer(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_element_initializer(self, arg0, arg1, arg2)
    }

    fn make_subscript_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_subscript_expression(self, arg0, arg1, arg2, arg3)
    }

    fn make_embedded_subscript_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_embedded_subscript_expression(self, arg0, arg1, arg2, arg3)
    }

    fn make_awaitable_creation_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_awaitable_creation_expression(self, arg0, arg1, arg2)
    }

    fn make_xhp_children_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_children_declaration(self, arg0, arg1, arg2)
    }

    fn make_xhp_children_parenthesized_list(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_children_parenthesized_list(self, arg0, arg1, arg2)
    }

    fn make_xhp_category_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_category_declaration(self, arg0, arg1, arg2)
    }

    fn make_xhp_enum_type(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_enum_type(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_xhp_lateinit(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_lateinit(self, arg0, arg1)
    }

    fn make_xhp_required(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_required(self, arg0, arg1)
    }

    fn make_xhp_class_attribute_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_class_attribute_declaration(self, arg0, arg1, arg2)
    }

    fn make_xhp_class_attribute(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_class_attribute(self, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_simple_class_attribute(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_simple_class_attribute(self, arg0)
    }

    fn make_xhp_simple_attribute(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_simple_attribute(self, arg0, arg1, arg2)
    }

    fn make_xhp_spread_attribute(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_spread_attribute(self, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_open(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_open(self, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_expression(self, arg0, arg1, arg2)
    }

    fn make_xhp_close(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_xhp_close(self, arg0, arg1, arg2)
    }

    fn make_type_constant(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_type_constant(self, arg0, arg1, arg2)
    }

    fn make_vector_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_vector_type_specifier(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_keyset_type_specifier(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_tuple_type_explicit_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_tuple_type_explicit_specifier(self, arg0, arg1, arg2, arg3)
    }

    fn make_varray_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_varray_type_specifier(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_function_ctx_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_function_ctx_type_specifier(self, arg0, arg1)
    }

    fn make_type_parameter(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_type_parameter(self, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_type_constraint(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_type_constraint(self, arg0, arg1)
    }

    fn make_context_constraint(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_context_constraint(self, arg0, arg1)
    }

    fn make_darray_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_darray_type_specifier(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_dictionary_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_dictionary_type_specifier(self, arg0, arg1, arg2, arg3)
    }

    fn make_closure_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_closure_type_specifier(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
    }

    fn make_closure_parameter_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_closure_parameter_type_specifier(self, arg0, arg1, arg2)
    }

    fn make_type_refinement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_type_refinement(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_type_in_refinement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_type_in_refinement(self, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_ctx_in_refinement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_ctx_in_refinement(self, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_classname_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_classname_type_specifier(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_class_args_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_class_args_type_specifier(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_field_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_field_specifier(self, arg0, arg1, arg2, arg3)
    }

    fn make_field_initializer(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_field_initializer(self, arg0, arg1, arg2)
    }

    fn make_shape_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_shape_type_specifier(self, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_shape_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_shape_expression(self, arg0, arg1, arg2, arg3)
    }

    fn make_tuple_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_tuple_expression(self, arg0, arg1, arg2, arg3)
    }

    fn make_generic_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_generic_type_specifier(self, arg0, arg1)
    }

    fn make_nullable_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_nullable_type_specifier(self, arg0, arg1)
    }

    fn make_like_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_like_type_specifier(self, arg0, arg1)
    }

    fn make_soft_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_soft_type_specifier(self, arg0, arg1)
    }

    fn make_attributized_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_attributized_specifier(self, arg0, arg1)
    }

    fn make_reified_type_argument(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_reified_type_argument(self, arg0, arg1)
    }

    fn make_type_arguments(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_type_arguments(self, arg0, arg1, arg2)
    }

    fn make_type_parameters(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_type_parameters(self, arg0, arg1, arg2)
    }

    fn make_tuple_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_tuple_type_specifier(self, arg0, arg1, arg2)
    }

    fn make_union_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_union_type_specifier(self, arg0, arg1, arg2)
    }

    fn make_intersection_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_intersection_type_specifier(self, arg0, arg1, arg2)
    }

    fn make_error(&mut self, arg0: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_error(self, arg0)
    }

    fn make_list_item(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_list_item(self, arg0, arg1)
    }

    fn make_enum_class_label_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_enum_class_label_expression(self, arg0, arg1, arg2)
    }

    fn make_module_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_module_declaration(self, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
    }

    fn make_module_exports(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_module_exports(self, arg0, arg1, arg2, arg3)
    }

    fn make_module_imports(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_module_imports(self, arg0, arg1, arg2, arg3)
    }

    fn make_module_membership_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_module_membership_declaration(self, arg0, arg1, arg2)
    }

    fn make_package_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        <Self as SyntaxSmartConstructors<S, TF, St>>::make_package_expression(self, arg0, arg1)
    }

}
