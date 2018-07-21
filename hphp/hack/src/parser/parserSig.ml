(**
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
 * This module contains a signature which can be used to describe smart
 * constructors.
  
 *)

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
  module Token = Syntax.Token
  module TokenKind = Full_fidelity_token_kind
  module SyntaxError = Full_fidelity_syntax_error
  module Env = Full_fidelity_parser_env
  module type SCWithKind_S = SmartConstructorsWrappers.SyntaxKind_S
  module type Lexer_S = Full_fidelity_lexer_sig.WithToken(Token).Lexer_S

  module WithLexer(Lexer : Lexer_S) = struct
    module type Parser_S = sig
      module SC : SCWithKind_S with module Token = Token
      type t [@@deriving show]
      val pos : t -> Full_fidelity_source_text.pos
      val sc_call : t -> (SC.t -> SC.t * SC.r) -> t * SC.r
      val lexer : t -> Lexer.t
      val errors : t -> Full_fidelity_syntax_error.t list
      val env : t -> Full_fidelity_parser_env.t
      val sc_state : t -> SC.t
      val with_errors : t -> SyntaxError.t list -> t
      val with_lexer : t -> Lexer.t -> t
      val expect : t -> TokenKind.t list -> t
      val skipped_tokens : t -> Token.t list
      val with_skipped_tokens : t -> Token.t list -> t
      val clear_skipped_tokens : t -> t

      module Make : sig
        val token : t -> Token.t -> t * SC.r
        val missing : t -> Full_fidelity_source_text.pos -> t * SC.r
        val list : t -> Full_fidelity_source_text.pos -> SC.r list -> t * SC.r
        val end_of_file : t -> SC.r -> t * SC.r
        val script : t -> SC.r -> t * SC.r
        val qualified_name : t -> SC.r -> t * SC.r
        val simple_type_specifier : t -> SC.r -> t * SC.r
        val literal_expression : t -> SC.r -> t * SC.r
        val prefixed_string_expression : t -> SC.r -> SC.r -> t * SC.r
        val variable_expression : t -> SC.r -> t * SC.r
        val pipe_variable_expression : t -> SC.r -> t * SC.r
        val enum_declaration : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val enumerator : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val alias_declaration : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val property_declaration : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val property_declarator : t -> SC.r -> SC.r -> t * SC.r
        val namespace_declaration : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val namespace_body : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val namespace_empty_body : t -> SC.r -> t * SC.r
        val namespace_use_declaration : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val namespace_group_use_declaration : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val namespace_use_clause : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val function_declaration : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val function_declaration_header : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val where_clause : t -> SC.r -> SC.r -> t * SC.r
        val where_constraint : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val methodish_declaration : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val classish_declaration : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val classish_body : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val trait_use_precedence_item : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val trait_use_alias_item : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val trait_use_conflict_resolution : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val trait_use : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val require_clause : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val const_declaration : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val constant_declarator : t -> SC.r -> SC.r -> t * SC.r
        val type_const_declaration : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val decorated_expression : t -> SC.r -> SC.r -> t * SC.r
        val parameter_declaration : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val variadic_parameter : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val attribute_specification : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val attribute : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val inclusion_expression : t -> SC.r -> SC.r -> t * SC.r
        val inclusion_directive : t -> SC.r -> SC.r -> t * SC.r
        val compound_statement : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val alternate_loop_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val expression_statement : t -> SC.r -> SC.r -> t * SC.r
        val markup_section : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val markup_suffix : t -> SC.r -> SC.r -> t * SC.r
        val unset_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val let_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val using_statement_block_scoped : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val using_statement_function_scoped : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val declare_directive_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val declare_block_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val while_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val if_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val elseif_clause : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val else_clause : t -> SC.r -> SC.r -> t * SC.r
        val alternate_if_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val alternate_elseif_clause : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val alternate_else_clause : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val try_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val catch_clause : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val finally_clause : t -> SC.r -> SC.r -> t * SC.r
        val do_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val for_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val foreach_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val switch_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val alternate_switch_statement : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val switch_section : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val switch_fallthrough : t -> SC.r -> SC.r -> t * SC.r
        val case_label : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val default_label : t -> SC.r -> SC.r -> t * SC.r
        val return_statement : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val goto_label : t -> SC.r -> SC.r -> t * SC.r
        val goto_statement : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val throw_statement : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val break_statement : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val continue_statement : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val function_static_statement : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val static_declarator : t -> SC.r -> SC.r -> t * SC.r
        val echo_statement : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val global_statement : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val simple_initializer : t -> SC.r -> SC.r -> t * SC.r
        val anonymous_class : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val anonymous_function : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val php7_anonymous_function : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val anonymous_function_use_clause : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val lambda_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val lambda_signature : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val cast_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val scope_resolution_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val member_selection_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val safe_member_selection_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val embedded_member_selection_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val yield_expression : t -> SC.r -> SC.r -> t * SC.r
        val yield_from_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val prefix_unary_expression : t -> SC.r -> SC.r -> t * SC.r
        val postfix_unary_expression : t -> SC.r -> SC.r -> t * SC.r
        val binary_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val instanceof_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val is_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val as_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val nullable_as_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val conditional_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val eval_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val empty_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val define_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val halt_compiler_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val isset_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val function_call_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val function_call_with_type_arguments_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val parenthesized_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val braced_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val embedded_braced_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val list_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val collection_literal_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val object_creation_expression : t -> SC.r -> SC.r -> t * SC.r
        val constructor_call : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val array_creation_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val array_intrinsic_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val darray_intrinsic_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val dictionary_intrinsic_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val keyset_intrinsic_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val varray_intrinsic_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val vector_intrinsic_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val element_initializer : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val subscript_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val embedded_subscript_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val awaitable_creation_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val xhp_children_declaration : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val xhp_children_parenthesized_list : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val xhp_category_declaration : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val xhp_enum_type : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val xhp_required : t -> SC.r -> SC.r -> t * SC.r
        val xhp_class_attribute_declaration : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val xhp_class_attribute : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val xhp_simple_class_attribute : t -> SC.r -> t * SC.r
        val xhp_simple_attribute : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val xhp_spread_attribute : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val xhp_open : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val xhp_expression : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val xhp_close : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val type_constant : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val vector_type_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val keyset_type_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val tuple_type_explicit_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val varray_type_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val vector_array_type_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val type_parameter : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val type_constraint : t -> SC.r -> SC.r -> t * SC.r
        val darray_type_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val map_array_type_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val dictionary_type_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val closure_type_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val closure_parameter_type_specifier : t -> SC.r -> SC.r -> t * SC.r
        val classname_type_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val field_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val field_initializer : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val shape_type_specifier : t -> SC.r -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val shape_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val tuple_expression : t -> SC.r -> SC.r -> SC.r -> SC.r -> t * SC.r
        val generic_type_specifier : t -> SC.r -> SC.r -> t * SC.r
        val nullable_type_specifier : t -> SC.r -> SC.r -> t * SC.r
        val soft_type_specifier : t -> SC.r -> SC.r -> t * SC.r
        val reified_type_argument : t -> SC.r -> SC.r -> t * SC.r
        val type_arguments : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val type_parameters : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val tuple_type_specifier : t -> SC.r -> SC.r -> SC.r -> t * SC.r
        val error : t -> SC.r -> t * SC.r
        val list_item : t -> SC.r -> SC.r -> t * SC.r

      end (* Make *)
    end (* Parser_S *)
  end (* WithLexer *)
end (* WithSyntax *)
