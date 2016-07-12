(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * This module contains the code describing the structure of a syntax tree.
 *
 * The relationships between the various functors and signatures here needs
 * some explanation.
 *
 * First off, the structure of the syntax tree is described by the collection
 * of recursive types that makes up the bulk of this file. The type "t" is
 * the type of a node in the syntax tree; each node has associated with it
 * an arbitrary value of type SyntaxValue.t, and syntax node proper, which
 * has structure given by the "syntax" type.
 *
 * Note that every child in the syntax tree is of type t, except for the
 * "Token" type. This should be the *only* child of a type other than t.
 * We are explicitly NOT attempting to impose a type structure on the parse
 * tree beyond what is already implied by the types here. For example,
 * we are not attempting to put into the type system here the restriction that
 * the children of a binary operator must be expressions. The reason for this
 * is because we are potentially parsing code as it is being typed, and we
 * do not want to restrict our ability to make good error recovery by imposing
 * a restriction that will only be valid in correct program text.
 *
 * That said, it would of course be ideal if the only children of a compound
 * statement were statements, and so on. But those invariants should be
 * imposed by the design of the parser, not by the type system of the syntax
 * tree code.
 *
 * We want to be able to use different kinds of tokens, with different
 * performance characteristics. Therefore this module is first functorized by
 * token type.
 *
 * We wish to associate arbitrary values with the syntax nodes so that we can
 * construct syntax trees with various properties -- trees that only know
 * their widths and are thereby cheap to serialize, trees that have full
 * position data for each node, trees where the tokens know their text and
 * can therefore be edited, trees that have name annotations or type
 * annotations, and so on.
 *
 * Therefore this module is functorized again with the value type to be
 * associated with the node.
 *
 * We also wish to provide factory methods, so that nodes can be built up
 * from their child nodes. A factory method must not just know all the
 * children and the kind of node it is constructing; it also must know how
 * to construct the value that this node is going to be tagged with. For
 * that reason, a third, optional functor is provided. This functor requires
 * that methods be provided to construct the values associated with a token
 * or with any arbitrary node, given its children. If this functor is used
 * then the resulting module contains factory methods.
 *
 * This module also provides some useful helper functions, like an iterator,
 * a rewriting visitor, and so on.
 *
 *)

module SyntaxKind = Full_fidelity_syntax_kind
module Operator = Full_fidelity_operator

module type TokenType = sig
  type t
  val kind: t -> Full_fidelity_token_kind.t
  val to_json: t -> Hh_json.json
end

module type SyntaxValueType = sig
  type t
end

(* These functors describe the shape of a parse tree that has a particular
   kind of token in the leaves, and a particular kind of value associated
   with each node. *)
module WithToken(Token: TokenType) = struct
  module WithSyntaxValue(SyntaxValue: SyntaxValueType) = struct

    type script_header = {
      header_less_than : t;
      header_question : t;
      header_language : t
    }
    and script = {
      script_header : t;
      script_declarations : t
    }
    and enum_declaration = {
      enum_enum : t;
      enum_name : t;
      enum_colon : t;
      enum_base : t;
      enum_type : t;
      enum_left_brace : t;
      enum_enumerators : t;
      enum_right_brace : t
    }
    and enumerator = {
      enumerator_name : t;
      enumerator_equal : t;
      enumerator_value : t
    }
    and alias_declaration = {
      alias_token : t;
      alias_name : t;
      alias_constraint : t;
      alias_equal : t;
      alias_type : t;
      alias_semicolon : t
    }
    and namespace_declaration = {
      namespace_token : t;
      namespace_name : t;
      namespace_body : t
    }
    and namespace_body = {
      namespace_left_brace : t;
      namespace_declarations : t;
      namespace_right_brace : t
    }
    and namespace_use_declaration = {
      namespace_use : t;
      namespace_use_clauses : t;
      namespace_use_semicolon : t
    }
    and namespace_use_clause = {
      namespace_use_name : t;
      namespace_use_as : t;
      namespace_use_alias : t
    }
    and function_declaration = {
      function_attr : t;
      function_async : t;
      function_token : t;
      function_name : t;
      function_type_params : t;
      function_left_paren : t;
      function_params : t;
      function_right_paren : t;
      function_colon : t;
      function_type : t;
      function_body : t
    }
    and classish_declaration = {
      classish_attr : t;
      classish_abstract : t;
      classish_final : t;
      classish_token : t;
      classish_name : t;
      classish_type_params : t;
      classish_extends : t;
      classish_extends_list : t;
      classish_implements : t;
      classish_implements_list : t;
      classish_body : t;
    }
    and classish_body = {
      classish_body_left_brace : t;
      classish_body_elements : t;
      classish_body_right_brace : t;
    }
    and parameter_declaration = {
      param_attr : t;
      param_type : t;
      param_name : t;
      param_default : t
    }
    and default_argument_specifier = {
      default_equal : t;
      default_value : t
    }
    and attribute_specification = {
      attribute_spec_left_double_angle : t;
      attribute_spec_attribute_list : t;
      attribute_spec_right_double_angle : t
    }
    and attribute = {
      attribute_name : t;
      attribute_left_paren : t;
      attribute_values : t;
      attribute_right_paren : t
    }
    and inclusion_directive = {
      inclusion_require : t;
      inclusion_left_paren : t;
      inclusion_filename : t;
      inclusion_right_paren : t;
      inclusion_semicolon : t
    }
    and compound_statement = {
      compound_left_brace : t;
      compound_statements : t;
      compound_right_brace : t
    }
    and expression_statement = {
      expr_statement_expr : t;
      expr_statement_semicolon : t
    }
    and while_statement = {
      while_keyword : t;
      while_left_paren : t;
      while_condition_expr : t;
      while_right_paren: t;
      while_body: t
    }
    and if_statement = {
      if_keyword: t;
      if_left_paren: t;
      if_condition_expr: t;
      if_right_paren: t;
      if_statement: t;
      if_elseif_clauses: t;
      if_else_clause: t;
    }
    and elseif_clause ={
      elseif_keyword: t;
      elseif_left_paren: t;
      elseif_condition_expr: t;
      elseif_right_paren: t;
      elseif_statement: t
    }
    and else_clause = {
      else_keyword: t;
      else_statement: t;
    }
    and try_statement = {
      try_keyword: t;
      try_compound_statement: t;
      catch_clauses: t;
      finally_clause: t;
    }
    and catch_clause = {
      catch_keyword: t;
      catch_left_paren: t;
      catch_params: t;
      catch_right_paren: t;
      catch_compound_statement: t;
    }
    and finally_clause = {
      finally_keyword: t;
      finally_compound_statement: t;
    }
    and do_statement = {
      do_keyword: t;
      do_statement: t;
      do_while_keyword: t;
      do_left_paren: t;
      do_condition_expr: t;
      do_right_paren: t;
      do_semicolon: t
    }
    and for_statement = {
      for_keyword: t;
      for_left_paren: t;
      for_initializer_expr: t;
      for_first_semicolon: t;
      for_control_expr: t;
      for_second_semicolon: t;
      for_end_of_loop_expr: t;
      for_right_paren: t;
      for_statement: t;
    }
    and foreach_statement = {
      foreach_keyword: t;
      foreach_left_paren: t;
      foreach_collection_name: t;
      foreach_await_opt: t;
      foreach_as: t;
      foreach_key_opt: t;
      foreach_key_arrow_opt: t;
      foreach_value: t;
      foreach_right_paren: t;
      foreach_statement: t;
    }
    and switch_statement = {
      switch_keyword: t;
      switch_left_paren: t;
      switch_expr: t;
      switch_right_paren: t;
      switch_compound_statement: t
    }
    and case_statement = {
      case_keyword: t;
      case_expr: t;
      case_colon: t;
      case_stmt: t;
    }
    and default_statement = {
      default_keyword: t;
      default_colon: t;
      default_stmt: t;
    }
    and return_statement = {
      return_keyword: t;
      return_expr: t;
      return_semicolon: t
    }
    and throw_statement = {
      throw_keyword: t;
      throw_expr: t;
      throw_semicolon: t
    }
    and break_statement = {
      break_keyword: t;
      break_semicolon: t
    }
    and continue_statement = {
      continue_keyword: t;
      continue_semicolon: t
    }
    and function_static_statement = {
      static_static : t;
      static_declarations : t;
      static_semicolon: t
    }
    and static_declarator = {
      static_name : t;
      static_init : t
    }
    and static_initializer = {
      static_init_equal : t;
      static_init_value : t
    }
    and anonymous_function = {
      anonymous_async : t;
      anonymous_function : t;
      anonymous_left_paren : t;
      anonymous_params : t;
      anonymous_right_paren : t;
      anonymous_colon : t;
      anonymous_type : t;
      anonymous_use : t;
      anonymous_body : t
    }
    and anonymous_use = {
      anonymous_use_token : t;
      anonymous_use_left_paren : t;
      anonymous_use_variables : t;
      anonymous_use_right_paren : t
    }
    and unary_operator = {
      unary_operator : t;
      unary_operand : t
    }
    and binary_operator = {
      binary_left_operand : t;
      binary_operator : t;
      binary_right_operand : t
    }
    and conditional_expression = {
      conditional_test : t;
      conditional_question : t;
      conditional_consequence : t;
      conditional_colon : t;
      conditional_alternative : t
    }
    and function_call_expression = {
      function_call_receiver  : t;
      function_call_lparen : t;
      function_call_arguments : t;
      function_call_rparen : t
    }
    and parenthesized_expression = {
      paren_expr_left_paren : t;
      paren_expr : t;
      paren_expr_right_paren : t
    }
    and braced_expression = {
      braced_expr_left_brace : t;
      braced_expr : t;
      braced_expr_right_brace : t
    }
    and listlike_expression = {
      listlike_keyword: t;
      listlike_left_paren: t;
      listlike_members: t;
      listlike_right_paren: t;
    }
    and object_creation_expression = {
      object_creation_new : t;
      object_creation_class : t;
      object_creation_lparen : t;
      object_creation_arguments : t;
      object_creation_rparen : t
    }
    and array_creation_expression = {
      array_creation_left_bracket: t;
      array_creation_members: t;
      array_creation_right_bracket: t;
    }
    and array_intrinsic_expression = {
      array_intrinsic_keyword: t;
      array_intrinsic_left_paren: t;
      array_intrinsic_members: t;
      array_intrinsic_right_paren: t;
    }
    and xhp_attribute = {
      xhp_attr_name : t;
      xhp_attr_equal : t;
      xhp_attr_expr : t;
    }
    and xhp_open = {
      xhp_open_name : t;
      xhp_open_attrs : t;
      xhp_open_right_angle : t;
    }
    and xhp_expression = {
      xhp_open : t;
      xhp_body : t;
      xhp_close : t
    }
    and xhp_close = {
      xhp_close_left_angle : t;
      xhp_close_name : t;
      xhp_close_right_angle : t;
    }
    and type_constant = {
      type_constant_left_type : t;
      type_constant_separator : t;
      type_constant_right_type : t
    }
    and vector_type_specifier = {
      vector_array : t;
      vector_left_angle : t;
      vector_type : t;
      vector_right_angle : t
    }
    and type_parameter = {
      type_variance_opt: t;
      type_name : t;
      type_constraint_list_opt  : t;
    }
    and type_constraint_specifier = {
      constraint_token: t;
      matched_type: t;
    }
    and map_type_specifier = {
      map_array : t;
      map_left_angle : t;
      map_key : t;
      map_comma : t;
      map_value : t;
      map_right_angle : t
    }
    and closure_type_specifier = {
      closure_outer_left_paren : t;
      closure_function : t;
      closure_inner_left_paren : t;
      closure_parameter_types : t;
      closure_inner_right_paren : t;
      closure_colon : t;
      closure_return_type : t;
      closure_outer_right_paren : t
    }
    and classname_type_specifier = {
      classname_classname : t;
      classname_left_angle : t;
      classname_type : t;
      classname_right_angle : t
    }
    and field_specifier = {
      field_name : t;
      field_arrow : t;
      field_type : t
    }
    and field_initializer = {
      field_init_name : t;
      field_init_arrow : t;
      field_init_value : t
    }
    and shape = {
      shape_shape : t;
      shape_left_paren : t;
      shape_fields : t;
      shape_right_paren : t
    }
    and generic_type = {
      generic_class_type : t;
      generic_arguments : t
    }
    and nullable_type_specifier = {
      nullable_question : t;
      nullable_type : t
    }
    and type_arguments = {
      type_arguments_left_angle : t;
      type_arguments : t;
      type_arguments_right_angle : t
    }
    and tuple_type_specifier = {
      tuple_left_paren : t;
      tuple_types : t;
      tuple_right_paren : t
    }
    and list_item = {
      list_item : t;
      list_separator : t
    }
    and syntax =
    | Token of Token.t
    | Error of t list
    | Missing
    | SyntaxList of t list
    | ListItem of list_item
    | ScriptHeader of script_header
    | Script of script

    | NamespaceDeclaration of namespace_declaration
    | NamespaceBody of namespace_body
    | NamespaceUseDeclaration of namespace_use_declaration
    | NamespaceUseClause of namespace_use_clause
    | FunctionDeclaration of function_declaration
    | ClassishDeclaration of classish_declaration
    | ClassishBody of classish_body
    | EnumDeclaration of enum_declaration
    | Enumerator of enumerator
    | AliasDeclaration of alias_declaration
    | ParameterDeclaration of parameter_declaration
    | DefaultArgumentSpecifier of default_argument_specifier
    | AttributeSpecification of attribute_specification
    | Attribute of attribute
    | InclusionDirective of inclusion_directive
    | CompoundStatement of compound_statement
    | ExpressionStatement of expression_statement
    | WhileStatement of while_statement
    | IfStatement of if_statement
    | ElseifClause of elseif_clause
    | ElseClause of else_clause
    | TryStatement of try_statement
    | CatchClause of catch_clause
    | FinallyClause of finally_clause
    | DoStatement of do_statement
    | ForStatement of for_statement
    | ForeachStatement of foreach_statement
    | SwitchStatement of switch_statement
    | CaseStatement of case_statement
    | DefaultStatement of default_statement
    | ReturnStatement of return_statement
    | ThrowStatement of throw_statement
    | BreakStatement of break_statement
    | ContinueStatement of continue_statement
    | FunctionStaticStatement of function_static_statement
    | StaticInitializer of static_initializer
    | StaticDeclarator of static_declarator

    | AnonymousFunction of anonymous_function
    | AnonymousFunctionUseClause of anonymous_use
    | LiteralExpression of t
    | VariableExpression of t
    | QualifiedNameExpression of t
    | PrefixUnaryOperator of unary_operator
    | PostfixUnaryOperator of unary_operator
    | BinaryOperator of binary_operator
    | ConditionalExpression of conditional_expression
    | FunctionCallExpression of function_call_expression
    | ParenthesizedExpression of parenthesized_expression
    | BracedExpression of braced_expression
    | ListExpression of listlike_expression
    | ObjectCreationExpression of object_creation_expression
    | ShapeExpression of shape
    | FieldInitializer of field_initializer
    | ArrayCreationExpression of array_creation_expression
    | ArrayIntrinsicExpression of array_intrinsic_expression
    | XHPExpression of xhp_expression
    | XHPOpen of xhp_open
    | XHPAttribute of xhp_attribute
    | XHPClose of xhp_close

    | SimpleTypeSpecifier of t
    | NullableTypeSpecifier of nullable_type_specifier
    | TypeConstraint of type_constraint_specifier
    | TypeParameter of type_parameter
    | TypeConstant of type_constant
    | GenericTypeSpecifier of generic_type
    | TypeArguments of type_arguments
    | TupleTypeSpecifier of tuple_type_specifier
    | VectorTypeSpecifier of vector_type_specifier
    | MapTypeSpecifier of map_type_specifier
    | ClosureTypeSpecifier of closure_type_specifier
    | ClassnameTypeSpecifier of classname_type_specifier
    | ShapeTypeSpecifier of shape
    | FieldSpecifier of field_specifier

    and t = { syntax : syntax ; value : SyntaxValue.t}

    let make syntax value =
      { syntax; value }

    let syntax node =
      node.syntax

    let value node =
      node.value

    let to_kind syntax =
      match syntax with
      | Missing -> SyntaxKind.Missing
      | Token _  -> SyntaxKind.Token
      | AnonymousFunction _ -> SyntaxKind.AnonymousFunction
      | AnonymousFunctionUseClause _ -> SyntaxKind.AnonymousFunctionUseClause
      | LiteralExpression _ -> SyntaxKind.LiteralExpression
      | VariableExpression _ -> SyntaxKind.VariableExpression
      | QualifiedNameExpression _ -> SyntaxKind.QualifiedNameExpression
      | Error _ -> SyntaxKind.Error
      | SyntaxList _ -> SyntaxKind.SyntaxList
      | ListItem _ -> SyntaxKind.ListItem
      | ScriptHeader _ -> SyntaxKind.ScriptHeader
      | Script _ -> SyntaxKind.Script
      | EnumDeclaration _ -> SyntaxKind.EnumDeclaration
      | Enumerator _ -> SyntaxKind.Enumerator
      | AliasDeclaration _ -> SyntaxKind.AliasDeclaration
      | NamespaceDeclaration _ -> SyntaxKind.NamespaceDeclaration
      | NamespaceBody _ -> SyntaxKind.NamespaceBody
      | NamespaceUseDeclaration _ -> SyntaxKind.NamespaceUseDeclaration
      | NamespaceUseClause _ -> SyntaxKind.NamespaceUseClause
      | FunctionDeclaration _ -> SyntaxKind.FunctionDeclaration
      | ClassishDeclaration _ -> SyntaxKind.ClassishDeclaration
      | ClassishBody _ -> SyntaxKind.ClassishBody
      | ParameterDeclaration _ -> SyntaxKind.ParameterDeclaration
      | DefaultArgumentSpecifier _ -> SyntaxKind.DefaultArgumentSpecifier
      | AttributeSpecification _ -> SyntaxKind.AttributeSpecification
      | Attribute _ -> SyntaxKind.Attribute
      | InclusionDirective _ -> SyntaxKind.InclusionDirective
      | CompoundStatement _ -> SyntaxKind.CompoundStatement
      | ExpressionStatement _ -> SyntaxKind.ExpressionStatement
      | WhileStatement _ -> SyntaxKind.WhileStatement
      | IfStatement _ -> SyntaxKind.IfStatement
      | ElseifClause _ -> SyntaxKind.ElseifClause
      | ElseClause _ -> SyntaxKind.ElseClause
      | TryStatement _ -> SyntaxKind.TryStatement
      | CatchClause _ -> SyntaxKind.CatchClause
      | FinallyClause _ -> SyntaxKind.FinallyClause
      | DoStatement _ -> SyntaxKind.DoStatement
      | ForStatement _ -> SyntaxKind.ForStatement
      | ForeachStatement _ -> SyntaxKind.ForeachStatement
      | SwitchStatement _ -> SyntaxKind.SwitchStatement
      | CaseStatement _ -> SyntaxKind.CaseStatement
      | DefaultStatement _ -> SyntaxKind.DefaultStatement
      | ReturnStatement _ -> SyntaxKind.ReturnStatement
      | ThrowStatement _ -> SyntaxKind.ThrowStatement
      | BreakStatement _ -> SyntaxKind.BreakStatement
      | ContinueStatement _ -> SyntaxKind.ContinueStatement
      | FunctionStaticStatement _ -> SyntaxKind.FunctionStaticStatement
      | StaticInitializer _ -> SyntaxKind.StaticInitializer
      | StaticDeclarator _ -> SyntaxKind.StaticDeclarator
      | PrefixUnaryOperator _ -> SyntaxKind.PrefixUnaryOperator
      | PostfixUnaryOperator _ -> SyntaxKind.PostfixUnaryOperator
      | BinaryOperator _ -> SyntaxKind.BinaryOperator
      | ConditionalExpression _ -> SyntaxKind.ConditionalExpression
      | FunctionCallExpression _ -> SyntaxKind.FunctionCallExpression
      | ParenthesizedExpression _ -> SyntaxKind.ParenthesizedExpression
      | BracedExpression _ -> SyntaxKind.BracedExpression
      | ListExpression _ -> SyntaxKind.ListExpression
      | ObjectCreationExpression _ -> SyntaxKind.ObjectCreationExpression
      | ShapeExpression _ -> SyntaxKind.ShapeExpression
      | FieldInitializer _ -> SyntaxKind.FieldInitializer
      | ArrayCreationExpression _ -> SyntaxKind.ArrayCreationExpression
      | ArrayIntrinsicExpression _ -> SyntaxKind.ArrayIntrinsicExpression
      | XHPExpression _ -> SyntaxKind.XHPExpression
      | XHPOpen _ -> SyntaxKind.XHPOpen
      | XHPClose _ -> SyntaxKind.XHPClose
      | XHPAttribute _ -> SyntaxKind.XHPAttribute
      | TypeConstant _ ->  SyntaxKind.TypeConstant
      | SimpleTypeSpecifier _ -> SyntaxKind.SimpleTypeSpecifier
      | TypeConstraint _ -> SyntaxKind.TypeConstraint
      | TypeParameter _ -> SyntaxKind.TypeParameter
      | NullableTypeSpecifier _ -> SyntaxKind.NullableTypeSpecifier
      | GenericTypeSpecifier _ -> SyntaxKind.GenericTypeSpecifier
      | TypeArguments _ -> SyntaxKind.TypeArguments
      | TupleTypeSpecifier _ -> SyntaxKind.TupleTypeSpecifier
      | VectorTypeSpecifier _ -> SyntaxKind.VectorTypeSpecifier
      | MapTypeSpecifier _ -> SyntaxKind.MapTypeSpecifier
      | ClosureTypeSpecifier _ -> SyntaxKind.ClosureTypeSpecifier
      | ClassnameTypeSpecifier _ -> SyntaxKind.ClassnameTypeSpecifier
      | ShapeTypeSpecifier _ -> SyntaxKind.ShapeTypeSpecifier
      | FieldSpecifier _ -> SyntaxKind.FieldSpecifier

    let kind node =
      to_kind (syntax node)

    let is_missing node = kind node = SyntaxKind.Missing
    let is_token node = kind node = SyntaxKind.Token
    let is_anonymous_function node = kind node = SyntaxKind.AnonymousFunction
    let is_anonymous_function_use_clause node =
      kind node = SyntaxKind.AnonymousFunctionUseClause
    let is_literal node = kind node = SyntaxKind.LiteralExpression
    let is_variable node = kind node = SyntaxKind.VariableExpression
    let is_qualified_name node = kind node = SyntaxKind.QualifiedNameExpression
    let is_error node = kind node = SyntaxKind.Error
    let is_list node = kind node = SyntaxKind.SyntaxList
    let is_list_item node = kind node = SyntaxKind.ListItem
    let is_header node = kind node = SyntaxKind.ScriptHeader
    let is_script node = kind node = SyntaxKind.Script
    let is_enum node = kind node = SyntaxKind.EnumDeclaration
    let is_enumerator node = kind node = SyntaxKind.Enumerator
    let is_alias node = kind node = SyntaxKind.AliasDeclaration
    let is_namespace node = kind node = SyntaxKind.NamespaceDeclaration
    let is_namespace_body node = kind node = SyntaxKind.NamespaceBody
    let is_namespace_use node = kind node = SyntaxKind.NamespaceUseDeclaration
    let is_namespace_use_clause node = kind node = SyntaxKind.NamespaceUseClause
    let is_function node = kind node = SyntaxKind.FunctionDeclaration
    let is_classish node = kind node = SyntaxKind.ClassishDeclaration
    let is_classish_body node = kind node = SyntaxKind.ClassishBody
    let is_parameter node = kind node = SyntaxKind.ParameterDeclaration
    let is_default_arg_specifier node =
      kind node = SyntaxKind.DefaultArgumentSpecifier
    let is_attribute_specification node =
      kind node = SyntaxKind.AttributeSpecification
    let is_attribute node = kind node = SyntaxKind.Attribute
    let is_inclusion_directive node = kind node = SyntaxKind.InclusionDirective
    let is_compound_statement node = kind node = SyntaxKind.CompoundStatement
    let is_expression_statement node =
      kind node = SyntaxKind.ExpressionStatement
    let is_foreach_statement node = kind node = SyntaxKind.ForeachStatement
    let is_while_statement node = kind node = SyntaxKind.WhileStatement
    let is_if_statement node = kind node = SyntaxKind.IfStatement
    let is_elseif node = kind node = SyntaxKind.ElseifClause
    let is_else node = kind node = SyntaxKind.ElseClause
    let is_try_statement node = kind node = SyntaxKind.TryStatement
    let is_catch node = kind node = SyntaxKind.CatchClause
    let is_finally node = kind node = SyntaxKind.FinallyClause
    let is_do_statement node = kind node = SyntaxKind.DoStatement
    let is_function_static_statement node =
      kind node = SyntaxKind.FunctionStaticStatement
    let is_static_initializer node = kind node = SyntaxKind.StaticInitializer
    let is_static_declarator node = kind node = SyntaxKind.StaticDeclarator
    let is_switch_statement node = kind node = SyntaxKind.SwitchStatement
    let is_prefix_operator node = kind node = SyntaxKind.PrefixUnaryOperator
    let is_postfix_operator node = kind node = SyntaxKind.PostfixUnaryOperator
    let is_binary_operator node = kind node = SyntaxKind.BinaryOperator
    let is_type_constraint node = kind node = SyntaxKind.TypeConstraint
    let is_type_parameter node = kind node = SyntaxKind.TypeParameter
    let is_conditional_expression node =
      kind node = SyntaxKind.ConditionalExpression
    let is_function_call_expression node =
      kind node = SyntaxKind.FunctionCallExpression
    let is_parenthesized_expression node =
      kind node = SyntaxKind.ParenthesizedExpression
    let is_braced_expression node = kind node = SyntaxKind.BracedExpression
    let is_listlike_expression node = kind node = SyntaxKind.ListExpression
    let is_object_creation_expression node =
      kind node = SyntaxKind.ObjectCreationExpression
    let is_shape_expression node = kind node = SyntaxKind.ShapeExpression
    let is_field_initializer node = kind node = SyntaxKind.FieldInitializer
    let is_array_creation_expression node =
      kind node = SyntaxKind.ArrayCreationExpression
    let is_array_intrinsic_expression node =
      kind node = SyntaxKind.ArrayIntrinsicExpression
    let is_xhp_expression node = kind node = SyntaxKind.XHPExpression
    let is_xhp_open node = kind node = SyntaxKind.XHPOpen
    let is_xhp_attribute node = kind node = SyntaxKind.XHPAttribute
    let is_type_constant node = kind node = SyntaxKind.TypeConstant
    let is_simple_type node = kind node = SyntaxKind.SimpleTypeSpecifier
    let is_generic_type node = kind node = SyntaxKind.GenericTypeSpecifier
    let is_nullable_type_specifier node =
      kind node = SyntaxKind.NullableTypeSpecifier
    let is_type_arguments node = kind node = SyntaxKind.TypeArguments
    let is_tuple_type node = kind node = SyntaxKind.TupleTypeSpecifier
    let is_vector_type_specifier node =
      kind node = SyntaxKind.VectorTypeSpecifier
    let is_map_type_specifier node =
      kind node = SyntaxKind.MapTypeSpecifier
    let is_closure_type_specifier node =
      kind node = SyntaxKind.ClosureTypeSpecifier
    let is_classname_type_specifier node =
      kind node = SyntaxKind.ClassnameTypeSpecifier
    let is_shape_type_specifier node =
      kind node = SyntaxKind.ShapeTypeSpecifier
    let is_field_specifier node =
      kind node = SyntaxKind.FieldSpecifier

    let is_separable_prefix node =
      match syntax node with
      | Token t -> begin
        Full_fidelity_token_kind.(match Token.kind t with
        | PlusPlus | MinusMinus -> false
        | _ -> true) end
      | _ -> true

    let children node =
      match node.syntax with
      | Missing -> []
      | Token _ -> []
      | LiteralExpression x -> [x]
      | VariableExpression x -> [x]
      | QualifiedNameExpression x -> [x]
      | Error x -> x
      | SyntaxList x -> x
      | AnonymousFunction
        { anonymous_async; anonymous_function; anonymous_left_paren;
          anonymous_params; anonymous_right_paren; anonymous_colon;
          anonymous_type; anonymous_use; anonymous_body } ->
        [ anonymous_async; anonymous_function; anonymous_left_paren;
          anonymous_params; anonymous_right_paren; anonymous_colon;
          anonymous_type; anonymous_use; anonymous_body ]
      | AnonymousFunctionUseClause
        { anonymous_use_token; anonymous_use_left_paren;
          anonymous_use_variables; anonymous_use_right_paren } ->
        [ anonymous_use_token; anonymous_use_left_paren;
          anonymous_use_variables; anonymous_use_right_paren ]
      | ListItem
        { list_item; list_separator } ->
        [ list_item; list_separator ]
      | ScriptHeader
        { header_less_than; header_question; header_language } ->
        [ header_less_than; header_question; header_language ]
      | Script
        { script_header; script_declarations } ->
        [ script_header; script_declarations ]
      | EnumDeclaration
        { enum_enum; enum_name; enum_colon; enum_base; enum_type;
          enum_left_brace; enum_enumerators; enum_right_brace } ->
        [ enum_enum; enum_name; enum_colon; enum_base; enum_type;
          enum_left_brace; enum_enumerators; enum_right_brace ]
      | Enumerator
        { enumerator_name; enumerator_equal; enumerator_value } ->
        [ enumerator_name; enumerator_equal; enumerator_value ]
      | AliasDeclaration
        { alias_token; alias_name; alias_constraint;
          alias_equal; alias_type; alias_semicolon } ->
        [ alias_token; alias_name; alias_constraint;
          alias_equal; alias_type; alias_semicolon ]
      | NamespaceDeclaration
        { namespace_token; namespace_name; namespace_body } ->
        [ namespace_token; namespace_name; namespace_body ]
      | NamespaceBody
        { namespace_left_brace; namespace_declarations;
          namespace_right_brace } ->
        [ namespace_left_brace; namespace_declarations;
          namespace_right_brace ]
      | NamespaceUseDeclaration
        { namespace_use; namespace_use_clauses; namespace_use_semicolon } ->
        [ namespace_use; namespace_use_clauses; namespace_use_semicolon ]
      | NamespaceUseClause
        { namespace_use_name; namespace_use_as; namespace_use_alias } ->
        [ namespace_use_name; namespace_use_as; namespace_use_alias ]
      | FunctionDeclaration
        { function_attr; function_async; function_token; function_name;
          function_type_params; function_left_paren; function_params;
          function_right_paren; function_colon; function_type; function_body} ->
        [ function_attr; function_async; function_token; function_name;
          function_type_params; function_left_paren; function_params;
          function_right_paren; function_colon; function_type; function_body]
      | ClassishDeclaration
        { classish_attr; classish_abstract; classish_final; classish_token;
          classish_name; classish_type_params; classish_extends;
          classish_extends_list; classish_implements; classish_implements_list;
          classish_body } ->
        [ classish_attr; classish_abstract; classish_final; classish_token;
          classish_name; classish_type_params; classish_extends;
          classish_extends_list; classish_implements; classish_implements_list;
          classish_body ]
      | ClassishBody
        { classish_body_left_brace; classish_body_elements;
          classish_body_right_brace } ->
        [ classish_body_left_brace; classish_body_elements;
          classish_body_right_brace ]
      | ParameterDeclaration
        { param_attr; param_type; param_name; param_default } ->
        [ param_attr; param_type; param_name; param_default ]
      | DefaultArgumentSpecifier
        { default_equal; default_value } ->
        [ default_equal; default_value ]
      | AttributeSpecification
        { attribute_spec_left_double_angle; attribute_spec_attribute_list ;
          attribute_spec_right_double_angle } ->
        [ attribute_spec_left_double_angle; attribute_spec_attribute_list ;
          attribute_spec_right_double_angle ]
      | Attribute
        { attribute_name; attribute_left_paren; attribute_values;
          attribute_right_paren } ->
        [ attribute_name; attribute_left_paren; attribute_values;
          attribute_right_paren ]
      | InclusionDirective
        { inclusion_require; inclusion_left_paren; inclusion_filename;
          inclusion_right_paren; inclusion_semicolon } ->
        [ inclusion_require; inclusion_left_paren; inclusion_filename;
          inclusion_right_paren; inclusion_semicolon ]
      | CompoundStatement
        { compound_left_brace; compound_statements; compound_right_brace } ->
        [ compound_left_brace; compound_statements; compound_right_brace ]
      | ExpressionStatement
        { expr_statement_expr; expr_statement_semicolon } ->
        [ expr_statement_expr; expr_statement_semicolon ]
      | WhileStatement
        { while_keyword; while_left_paren; while_condition_expr;
          while_right_paren; while_body } ->
        [ while_keyword; while_left_paren; while_condition_expr;
          while_right_paren; while_body ]
      | IfStatement
        { if_keyword; if_left_paren; if_condition_expr; if_right_paren;
          if_statement; if_elseif_clauses; if_else_clause } ->
        [ if_keyword; if_left_paren; if_condition_expr; if_right_paren;
          if_statement; if_elseif_clauses; if_else_clause ]
      | ElseifClause
        { elseif_keyword; elseif_left_paren; elseif_condition_expr;
          elseif_right_paren; elseif_statement } ->
        [ elseif_keyword; elseif_left_paren; elseif_condition_expr;
          elseif_right_paren; elseif_statement ]
      | ElseClause
        { else_keyword; else_statement } ->
        [ else_keyword; else_statement ]
      | TryStatement {try_keyword; try_compound_statement; catch_clauses;
                      finally_clause} ->
        [try_keyword; try_compound_statement; catch_clauses; finally_clause]
      | CatchClause {catch_keyword; catch_left_paren; catch_params;
                     catch_right_paren; catch_compound_statement} ->
        [catch_keyword; catch_left_paren; catch_params; catch_right_paren;
         catch_compound_statement]
      | FinallyClause {finally_keyword; finally_compound_statement} ->
        [finally_keyword; finally_compound_statement]
      | DoStatement
        { do_keyword; do_statement; do_while_keyword; do_left_paren;
          do_condition_expr; do_right_paren; do_semicolon } ->
        [ do_keyword; do_statement; do_while_keyword; do_left_paren;
          do_condition_expr; do_right_paren; do_semicolon ]
      | ForStatement
        { for_keyword; for_left_paren; for_initializer_expr;
          for_first_semicolon; for_control_expr; for_second_semicolon;
          for_end_of_loop_expr; for_right_paren; for_statement } ->
        [ for_keyword; for_left_paren; for_initializer_expr;
          for_first_semicolon; for_control_expr; for_second_semicolon;
          for_end_of_loop_expr; for_right_paren; for_statement ]
      | ForeachStatement
        { foreach_keyword; foreach_left_paren; foreach_collection_name;
          foreach_await_opt; foreach_as; foreach_key_opt; foreach_key_arrow_opt;
          foreach_value; foreach_right_paren; foreach_statement } ->
        [ foreach_keyword; foreach_left_paren; foreach_collection_name;
          foreach_await_opt; foreach_as; foreach_key_opt; foreach_key_arrow_opt;
          foreach_value; foreach_right_paren; foreach_statement ]
      | SwitchStatement
        { switch_keyword; switch_left_paren; switch_expr; switch_right_paren;
          switch_compound_statement } ->
        [ switch_keyword; switch_left_paren; switch_expr; switch_right_paren;
          switch_compound_statement ]
      | CaseStatement
        { case_keyword; case_expr; case_colon; case_stmt } ->
        [ case_keyword; case_expr; case_colon; case_stmt ]
      | DefaultStatement
        { default_keyword; default_colon; default_stmt } ->
        [ default_keyword; default_colon; default_stmt ]
      | ReturnStatement
        { return_keyword; return_expr; return_semicolon } ->
        [ return_keyword; return_expr; return_semicolon ]
      | ThrowStatement
        { throw_keyword; throw_expr; throw_semicolon } ->
        [ throw_keyword; throw_expr; throw_semicolon ]
      | BreakStatement
        { break_keyword; break_semicolon } ->
        [ break_keyword; break_semicolon ]
      | ContinueStatement
        { continue_keyword; continue_semicolon } ->
        [ continue_keyword; continue_semicolon ]
      | FunctionStaticStatement
        { static_static; static_declarations; static_semicolon } ->
        [ static_static; static_declarations; static_semicolon ]
      | StaticInitializer
        { static_init_equal; static_init_value } ->
        [ static_init_equal; static_init_value ]
      | StaticDeclarator
        { static_name; static_init } ->
        [ static_name; static_init ]
      | PrefixUnaryOperator
        { unary_operator; unary_operand } ->
        [ unary_operator; unary_operand ]
      | PostfixUnaryOperator
        { unary_operand; unary_operator } ->
        [ unary_operand; unary_operator ]
      | BinaryOperator
        { binary_left_operand; binary_operator; binary_right_operand } ->
        [ binary_left_operand; binary_operator; binary_right_operand ]
      | ConditionalExpression
        { conditional_test; conditional_question; conditional_consequence;
          conditional_colon; conditional_alternative } ->
        [ conditional_test; conditional_question; conditional_consequence;
          conditional_colon; conditional_alternative ]
      | FunctionCallExpression
        { function_call_receiver; function_call_lparen;
          function_call_arguments; function_call_rparen } ->
        [ function_call_receiver; function_call_lparen;
          function_call_arguments; function_call_rparen ]
      | ParenthesizedExpression
        { paren_expr_left_paren; paren_expr; paren_expr_right_paren } ->
        [ paren_expr_left_paren; paren_expr; paren_expr_right_paren ]
      | BracedExpression
        { braced_expr_left_brace; braced_expr; braced_expr_right_brace } ->
        [ braced_expr_left_brace; braced_expr; braced_expr_right_brace ]
      | ListExpression
        { listlike_keyword; listlike_left_paren; listlike_members;
          listlike_right_paren } ->
        [ listlike_keyword; listlike_left_paren; listlike_members;
          listlike_right_paren ]
      | ObjectCreationExpression
        { object_creation_new; object_creation_class; object_creation_lparen;
          object_creation_arguments; object_creation_rparen } ->
        [ object_creation_new; object_creation_class; object_creation_lparen;
          object_creation_arguments; object_creation_rparen ]
      | ShapeExpression
        { shape_shape; shape_left_paren; shape_fields; shape_right_paren } ->
        [ shape_shape; shape_left_paren; shape_fields; shape_right_paren ]
      | FieldInitializer
        { field_init_name; field_init_arrow; field_init_value } ->
        [ field_init_name; field_init_arrow; field_init_value ]
      | ArrayCreationExpression
        { array_creation_left_bracket; array_creation_members;
          array_creation_right_bracket } ->
        [ array_creation_left_bracket; array_creation_members;
          array_creation_right_bracket ]
      | ArrayIntrinsicExpression
       { array_intrinsic_keyword; array_intrinsic_left_paren;
         array_intrinsic_members; array_intrinsic_right_paren } ->
       [ array_intrinsic_keyword; array_intrinsic_left_paren;
         array_intrinsic_members; array_intrinsic_right_paren ]
      | XHPExpression
        { xhp_open; xhp_body; xhp_close } ->
        [ xhp_open; xhp_body; xhp_close ]
      | XHPOpen
        { xhp_open_name; xhp_open_attrs; xhp_open_right_angle } ->
        [ xhp_open_name; xhp_open_attrs; xhp_open_right_angle ]
      | XHPClose
        { xhp_close_left_angle; xhp_close_name; xhp_close_right_angle } ->
        [ xhp_close_left_angle; xhp_close_name; xhp_close_right_angle ]
      | XHPAttribute
        { xhp_attr_name; xhp_attr_equal; xhp_attr_expr } ->
        [ xhp_attr_name; xhp_attr_equal; xhp_attr_expr ]
      | TypeConstant
        { type_constant_left_type; type_constant_separator;
          type_constant_right_type } ->
        [ type_constant_left_type; type_constant_separator;
        type_constant_right_type ]
      | SimpleTypeSpecifier x -> [x]
      | TypeConstraint
        { constraint_token; matched_type } ->
        [ constraint_token; matched_type ]
      | TypeParameter
        { type_variance_opt; type_name; type_constraint_list_opt } ->
        [ type_variance_opt; type_name; type_constraint_list_opt ]
      | NullableTypeSpecifier
        { nullable_question; nullable_type } ->
        [ nullable_question; nullable_type ]
      | GenericTypeSpecifier
        { generic_class_type; generic_arguments } ->
        [ generic_class_type; generic_arguments ]
      | TypeArguments
        { type_arguments_left_angle; type_arguments;
          type_arguments_right_angle } ->
        [ type_arguments_left_angle; type_arguments;
          type_arguments_right_angle ]
      | TupleTypeSpecifier
        { tuple_left_paren; tuple_types; tuple_right_paren } ->
        [ tuple_left_paren; tuple_types; tuple_right_paren ]
      | VectorTypeSpecifier
        { vector_array; vector_left_angle; vector_type; vector_right_angle } ->
        [ vector_array; vector_left_angle; vector_type; vector_right_angle ]
      | MapTypeSpecifier
        { map_array; map_left_angle; map_key; map_comma; map_value;
          map_right_angle } ->
        [ map_array; map_left_angle; map_key; map_comma; map_value;
          map_right_angle ]
      | ClosureTypeSpecifier
        { closure_outer_left_paren; closure_function;
          closure_inner_left_paren; closure_parameter_types;
          closure_inner_right_paren; closure_colon; closure_return_type;
          closure_outer_right_paren } ->
        [ closure_outer_left_paren; closure_function;
          closure_inner_left_paren; closure_parameter_types;
          closure_inner_right_paren; closure_colon; closure_return_type;
          closure_outer_right_paren ]
      | ClassnameTypeSpecifier
        { classname_classname; classname_left_angle; classname_type;
          classname_right_angle } ->
        [ classname_classname; classname_left_angle; classname_type;
          classname_right_angle ]
      | ShapeTypeSpecifier
        { shape_shape; shape_left_paren; shape_fields; shape_right_paren } ->
        [ shape_shape; shape_left_paren; shape_fields; shape_right_paren ]
      | FieldSpecifier
        { field_name; field_arrow; field_type } ->
        [ field_name; field_arrow; field_type ]

    let children_names node =
      match node.syntax with
      | Missing -> []
      | Token _ -> []
      | LiteralExpression _ -> [ "literal_expression" ]
      | VariableExpression _ -> [ "variable_expression" ]
      | QualifiedNameExpression _ -> [ "qualified_name_expression" ]
      | Error _ -> []
      | SyntaxList _ -> []
      | AnonymousFunction
        { anonymous_async; anonymous_function; anonymous_left_paren;
          anonymous_params; anonymous_right_paren; anonymous_colon;
          anonymous_type; anonymous_use; anonymous_body } ->
        [ "anonymous_async"; "anonymous_function"; "anonymous_left_paren";
          "anonymous_params"; "anonymous_right_paren"; "anonymous_colon";
          "anonymous_type"; "anonymous_use"; "anonymous_body" ]
      | AnonymousFunctionUseClause
        { anonymous_use_token; anonymous_use_left_paren;
          anonymous_use_variables; anonymous_use_right_paren } ->
        [ "anonymous_use_token"; "anonymous_use_left_paren";
          "anonymous_use_variables"; "anonymous_use_right_paren" ]
      | ListItem
        { list_item; list_separator } ->
        [ "list_item"; "list_separator" ]
      | ScriptHeader
        { header_less_than; header_question; header_language } ->
        [ "header_less_than"; "header_question"; "header_language" ]
      | Script
        { script_header; script_declarations } ->
        [ "script_header"; "script_declarations" ]
      | EnumDeclaration
        { enum_enum; enum_name; enum_colon; enum_base; enum_type;
          enum_left_brace; enum_enumerators; enum_right_brace } ->
        [ "enum_enum"; "enum_name"; "enum_colon"; "enum_base"; "enum_type";
          "enum_left_brace"; "enum_enumerators"; "enum_right_brace" ]
      | Enumerator
        { enumerator_name; enumerator_equal; enumerator_value } ->
        [ "enumerator_name"; "enumerator_equal"; "enumerator_value" ]
      | AliasDeclaration
        { alias_token; alias_name; alias_constraint;
          alias_equal; alias_type; alias_semicolon } ->
        [ "alias_token"; "alias_name"; "alias_constraint";
          "alias_equal"; "alias_type"; "alias_semicolon" ]
      | NamespaceDeclaration
        { namespace_token; namespace_name; namespace_body } ->
        [ "namespace_token"; "namespace_name"; "namespace_body" ]
      | NamespaceBody
        { namespace_left_brace; namespace_declarations;
          namespace_right_brace } ->
        [ "namespace_left_brace"; "namespace_declarations";
          "namespace_right_brace" ]
      | NamespaceUseDeclaration
        { namespace_use; namespace_use_clauses; namespace_use_semicolon } ->
        [ "namespace_use"; "namespace_use_clauses"; "namespace_use_semicolon" ]
      | NamespaceUseClause
        { namespace_use_name; namespace_use_as; namespace_use_alias } ->
        [ "namespace_use_name"; "namespace_use_as"; "namespace_use_alias" ]
      | FunctionDeclaration
        { function_attr; function_async; function_token; function_name;
          function_type_params; function_left_paren; function_params;
          function_right_paren; function_colon; function_type;
          function_body} ->
        [ "function_attr"; "function_async"; "function_token"; "function_name";
          "function_type_params"; "function_left_paren"; "function_params";
          "function_right_paren"; "function_colon"; "function_type";
          "function_body" ]
      | ClassishDeclaration
        { classish_attr; classish_abstract; classish_final; classish_token;
          classish_name; classish_type_params; classish_extends;
          classish_extends_list; classish_implements; classish_implements_list;
          classish_body } ->
        [ "classish_attr"; "classish_abstract"; "classish_final";
          "classish_token"; "classish_name"; "classish_type_params";
          "classish_extends"; "classish_extends_list"; "classish_implements";
          "classish_implements_list"; "classish_body" ]
      | ClassishBody
        { classish_body_left_brace; classish_body_elements;
          classish_body_right_brace } ->
        [ "classish_body_left_brace"; "classish_body_elements";
          "classish_body_right_brace" ]
      | ParameterDeclaration
        { param_attr; param_type; param_name; param_default } ->
        [ "param_attr"; "param_type"; "param_name"; "param_default" ]
      | DefaultArgumentSpecifier
        { default_equal; default_value } ->
        [ "default_equal"; "default_value" ]
      | AttributeSpecification
        { attribute_spec_left_double_angle; attribute_spec_attribute_list ;
          attribute_spec_right_double_angle } ->
        [ "attribute_spec_left_double_angle"; "attribute_spec_attribute_list" ;
          "attribute_spec_right_double_angle" ]
      | Attribute
        { attribute_name; attribute_left_paren; attribute_values;
          attribute_right_paren } ->
        [ "attribute_name"; "attribute_left_paren"; "attribute_values";
          "attribute_right_paren" ]
      | InclusionDirective
        { inclusion_require; inclusion_left_paren; inclusion_filename;
          inclusion_right_paren; inclusion_semicolon } ->
        [ "inclusion_require"; "inclusion_left_paren"; "inclusion_filename";
          "inclusion_right_paren"; "inclusion_semicolon" ]
      | CompoundStatement
        { compound_left_brace; compound_statements; compound_right_brace } ->
        [ "compound_left_brace"; "compound_statements"; "compound_right_brace" ]
      | ExpressionStatement
        { expr_statement_expr; expr_statement_semicolon } ->
        [ "expr_statement_expr"; "expr_statement_semicolon" ]
      | WhileStatement
        { while_keyword; while_left_paren; while_condition_expr;
          while_right_paren; while_body } ->
        [ "while_keyword"; "while_left_paren"; "while_condition_expr";
          "while_right_paren"; "while_body" ]
      | IfStatement
        { if_keyword; if_left_paren; if_condition_expr; if_right_paren;
          if_statement; if_elseif_clauses; if_else_clause } ->
        [ "if_keyword"; "if_left_paren"; "if_condition_expr"; "if_right_paren";
          "if_statement"; "if_elseif_clauses"; "if_else_clause" ]
      | ElseifClause
        { elseif_keyword; elseif_left_paren; elseif_condition_expr;
          elseif_right_paren; elseif_statement } ->
        [ "elseif_keyword"; "elseif_left_paren"; "elseif_condition_expr";
          "elseif_right_paren"; "elseif_statement" ]
      | ElseClause
        { else_keyword; else_statement } ->
        [ "else_keyword"; "else_statement" ]
      | TryStatement {try_keyword; try_compound_statement; catch_clauses;
                      finally_clause} ->
        ["try_keyword"; "try_compound_statement"; "catch_clauses";
        "finally_clause"]
      | CatchClause {catch_keyword; catch_left_paren; catch_params;
                     catch_right_paren; catch_compound_statement} ->
        ["catch_keyword"; "catch_left_paren"; "catch_params";
        "catch_right_paren"; "catch_compound_statement"]
      | FinallyClause {finally_keyword; finally_compound_statement} ->
        ["finally_keyword"; "finally_compound_statement"]
      | DoStatement
        { do_keyword; do_statement; do_while_keyword; do_left_paren;
          do_condition_expr; do_right_paren; do_semicolon } ->
        [ "do_keyword"; "do_statement"; "do_while_keyword"; "do_left_paren";
          "do_condition_expr"; "do_right_paren"; "do_semicolon" ]
      | ForStatement
        { for_keyword; for_left_paren; for_initializer_expr;
          for_first_semicolon; for_control_expr; for_second_semicolon;
          for_end_of_loop_expr; for_right_paren; for_statement } ->
        [ "for_keyword"; "for_left_paren"; "for_initializer_expr";
          "for_first_semicolon"; "for_control_expr"; "for_second_semicolon";
          "for_end_of_loop_expr"; "for_right_paren"; "for_statement" ]
      | ForeachStatement
        { foreach_keyword; foreach_left_paren; foreach_collection_name;
          foreach_await_opt; foreach_as; foreach_key_opt; foreach_key_arrow_opt;
          foreach_value; foreach_right_paren; foreach_statement } ->
        [ "foreach_keyword"; "foreach_left_paren"; "foreach_collection_name";
          "foreach_await_opt"; "foreach_as"; "foreach_key_opt";
          "foreach_key_arrow_opt"; "foreach_value"; "foreach_right_paren";
          "foreach_statement" ]
      | SwitchStatement
        { switch_keyword; switch_left_paren; switch_expr;
          switch_right_paren; switch_compound_statement } ->
        [ "switch_keyword"; "switch_left_paren"; "switch_expr";
          "switch_right_paren"; "switch_compound_statement" ]
      | CaseStatement
        { case_keyword; case_expr; case_colon; case_stmt } ->
        [ "case_keyword"; "case_expr"; "case_colon"; "case_stmt" ]
      | DefaultStatement
        { default_keyword; default_colon; default_stmt } ->
        [ "default_keyword"; "default_colon"; "default_stmt" ]
      | ReturnStatement
        { return_keyword; return_expr; return_semicolon } ->
        [ "return_keyword"; "return_expr"; "return_semicolon" ]
      | ThrowStatement
        { throw_keyword; throw_expr; throw_semicolon } ->
        [ "throw_keyword"; "throw_expr"; "throw_semicolon" ]
      | BreakStatement
        { break_keyword; break_semicolon } ->
        [ "break_keyword"; "break_semicolon" ]
      | ContinueStatement
        { continue_keyword; continue_semicolon } ->
        [ "continue_keyword"; "continue_semicolon" ]
      | FunctionStaticStatement
        { static_static; static_declarations; static_semicolon } ->
        [ "static_static"; "static_declarations"; "static_semicolon" ]
      | StaticInitializer
        { static_init_equal; static_init_value } ->
        [ "static_init_equal"; "static_init_value" ]
      | StaticDeclarator
        { static_name; static_init } ->
        [ "static_name"; "static_init" ]
      | PrefixUnaryOperator
        { unary_operator; unary_operand } ->
        [ "unary_operator"; "unary_operand" ]
      | PostfixUnaryOperator
        { unary_operand; unary_operator } ->
        [ "unary_operand"; "unary_operator" ]
      | BinaryOperator
        { binary_left_operand; binary_operator; binary_right_operand } ->
        [ "binary_left_operand"; "binary_operator"; "binary_right_operand" ]
      | ConditionalExpression
        { conditional_test; conditional_question; conditional_consequence;
          conditional_colon; conditional_alternative } ->
        [ "conditional_test"; "conditional_question"; "conditional_consequence";
          "conditional_colon"; "conditional_alternative" ]
      | FunctionCallExpression
        { function_call_receiver; function_call_lparen;
          function_call_arguments; function_call_rparen } ->
        [ "function_call_receiver"; "function_call_lparen";
          "function_call_arguments"; "function_call_rparen" ]
      | ParenthesizedExpression
        { paren_expr_left_paren; paren_expr; paren_expr_right_paren } ->
        [ "paren_expr_left_paren"; "paren_expr"; "paren_expr_right_paren" ]
      | BracedExpression
        { braced_expr_left_brace; braced_expr; braced_expr_right_brace } ->
        [ "braced_expr_left_brace"; "braced_expr"; "braced_expr_right_brace" ]
      | ListExpression
        { listlike_keyword; listlike_left_paren; listlike_members;
          listlike_right_paren } ->
        [ "listlike_keyword"; "listlike_left_paren"; "listlike_members";
          "listlike_right_paren" ]
      | ObjectCreationExpression
        { object_creation_new; object_creation_class;
          object_creation_lparen; object_creation_arguments;
          object_creation_rparen } ->
        [ "object_creation_new"; "object_creation_class";
          "object_creation_lparen"; "object_creation_arguments";
          "object_creation_rparen" ]
      | ShapeExpression
        { shape_shape; shape_left_paren; shape_fields; shape_right_paren } ->
        [ "shape_shape"; "shape_left_paren"; "shape_fields";
          "shape_right_paren" ]
      | FieldInitializer
        { field_init_name; field_init_arrow; field_init_value } ->
        [ "field_init_name"; "field_init_arrow"; "field_init_value" ]
      | ArrayCreationExpression
        { array_creation_left_bracket; array_creation_members;
          array_creation_right_bracket } ->
        [ "array_creation_left_bracket"; "array_creation_members";
          "array_creation_right_bracket" ]
      | ArrayIntrinsicExpression
       { array_intrinsic_keyword; array_intrinsic_left_paren;
         array_intrinsic_members; array_intrinsic_right_paren } ->
       [ "array_intrinsic_keyword"; "array_intrinsic_left_paren";
         "array_intrinsic_members"; "array_intrinsic_right_paren" ]
      | XHPExpression
        { xhp_open; xhp_body; xhp_close } ->
        [ "xhp_open"; "xhp_body"; "xhp_close" ]
      | XHPOpen
        { xhp_open_name; xhp_open_attrs; xhp_open_right_angle } ->
        [ "xhp_open_name"; "xhp_open_attrs"; "xhp_open_right_angle" ]
      | XHPClose
        { xhp_close_left_angle; xhp_close_name; xhp_close_right_angle } ->
        [ "xhp_close_left_angle"; "xhp_close_name"; "xhp_close_right_angle" ]
      | XHPAttribute
        { xhp_attr_name; xhp_attr_equal; xhp_attr_expr } ->
        [ "xhp_attr_name"; "xhp_attr_equal"; "xhp_attr_expr" ]
      | TypeConstant
        { type_constant_left_type; type_constant_separator;
          type_constant_right_type } ->
        [ "type_constant_left_type"; "type_constant_separator";
        "type_constant_right_type" ]
      | SimpleTypeSpecifier _ -> [ "simple_type_specifier" ]
      | TypeParameter
        { type_variance_opt; type_name; type_constraint_list_opt  } ->
        [ "type_variance_opt"; "type_name"; "type_constraint_list_opt " ]
      | TypeConstraint { constraint_token; matched_type } ->
        [ "constraint_token"; "matched_type" ]
      | NullableTypeSpecifier
        { nullable_question; nullable_type } ->
        [ "nullable_question"; "nullable_type" ]
      | GenericTypeSpecifier
        { generic_class_type; generic_arguments } ->
        [ "generic_class_type"; "generic_arguments" ]
      | TypeArguments
        { type_arguments_left_angle; type_arguments;
          type_arguments_right_angle } ->
        [ "type_arguments_left_angle"; "type_arguments";
          "type_arguments_right_angle" ]
      | TupleTypeSpecifier
        { tuple_left_paren; tuple_types; tuple_right_paren } ->
        [ "tuple_left_paren"; "tuple_types"; "tuple_right_paren" ]
      | VectorTypeSpecifier
        { vector_array; vector_left_angle; vector_type; vector_right_angle } ->
        [ "vector_array"; "vector_left_angle"; "vector_type";
          "vector_right_angle" ]
      | MapTypeSpecifier
        { map_array; map_left_angle; map_key; map_comma; map_value;
          map_right_angle } ->
        [ "map_array"; "map_left_angle"; "map_key"; "map_comma"; "map_value";
          "map_right_angle" ]
      | ClosureTypeSpecifier
        { closure_outer_left_paren; closure_function;
          closure_inner_left_paren; closure_parameter_types;
          closure_inner_right_paren; closure_colon; closure_return_type;
          closure_outer_right_paren } ->
        [ "closure_outer_left_paren"; "closure_function";
          "closure_inner_left_paren"; "closure_parameter_types";
          "closure_inner_right_paren"; "closure_colon"; "closure_return_type";
          "closure_outer_right_paren" ]
      | ClassnameTypeSpecifier
        { classname_classname; classname_left_angle; classname_type;
          classname_right_angle } ->
        [ "classname_classname"; "classname_left_angle"; "classname_type";
          "classname_right_angle" ]
      | ShapeTypeSpecifier
        { shape_shape; shape_left_paren; shape_fields; shape_right_paren } ->
        [ "shape_shape"; "shape_left_paren"; "shape_fields";
          "shape_right_paren" ]
      | FieldSpecifier
        { field_name; field_arrow; field_type } ->
        [ "field_name"; "field_arrow"; "field_type" ]


    let rec to_json node =
      let open Hh_json in
      let ch = match node.syntax with
      | Token t -> [ "token", Token.to_json t ]
      | Error x -> [ ("errors", JSON_Array (List.map to_json x)) ]
      | SyntaxList x -> [ ("elements", JSON_Array (List.map to_json x)) ]
      | _ ->
        let rec aux acc c n =
          match c, n with
          | ([], []) -> acc
          | ((hc :: tc), (hn :: tn)) ->
            aux ((hn, to_json hc) :: acc) tc tn
          | _ -> failwith "mismatch between children and names" in
        List.rev (aux [] (children node) (children_names node)) in
      let k = ("kind", JSON_String (SyntaxKind.to_string (kind node))) in
      JSON_Object (k :: ch)

    let script_header x = x.script_header
    let script_declarations x = x.script_declarations
    let header_less_than x = x.header_less_than
    let header_question x = x.header_question
    let header_language x = x.header_language
    let function_attr x = x.function_attr
    let function_async x = x.function_async
    let function_token x = x.function_token
    let function_name x = x.function_name
    let function_type_params x = x.function_type_params
    let function_left_paren x = x.function_left_paren
    let function_params x = x.function_params
    let function_right_paren x = x.function_right_paren
    let function_colon x = x.function_colon
    let function_type x = x.function_type
    let function_body x = x.function_body
    let classish_attr x = x.classish_attr
    let classish_abstract x = x.classish_abstract
    let classish_final x = x.classish_final
    let classish_token x = x.classish_token
    let classish_name x = x.classish_name
    let classish_type_params x = x.classish_type_params
    let classish_extends x = x.classish_extends
    let classish_extends_list x = x.classish_extends_list
    let classish_implements x = x.classish_implements
    let classish_implements_list x = x.classish_implements_list
    let classish_body x = x.classish_body
    let classish_body_left_brace x = x.classish_body_left_brace
    let classish_body_elements x = x.classish_body_elements
    let classish_body_right_brace x = x.classish_body_right_brace
    let param_attr x = x.param_attr
    let param_type x = x.param_type
    let param_name x = x.param_name
    let param_default x = x.param_default
    let default_equal x = x.default_equal
    let default_value x = x.default_value
    let attribute_spec_left_double_angle x = x.attribute_spec_left_double_angle
    let attribute_spec_attribute_list x = x.attribute_spec_attribute_list
    let attribute_spec_right_double_angle x =
      x.attribute_spec_right_double_angle
    let attribute_name x = x.attribute_name
    let attribute_left_paren x = x.attribute_left_paren
    let attribute_values x = x.attribute_values
    let attribute_right_paren x = x.attribute_right_paren
    let compound_left_brace x = x.compound_left_brace
    let compound_statements x = x.compound_statements
    let compound_right_brace x = x.compound_right_brace
    let while_keyword x = x.while_keyword
    let while_left_paren x = x.while_left_paren
    let while_condition_expr x = x.while_condition_expr
    let while_right_paren x = x.while_right_paren
    let while_body x = x.while_body
    let foreach_keyword x = x.foreach_keyword
    let foreach_left_paren x = x.foreach_left_paren
    let foreach_collection_name x = x.foreach_collection_name
    let foreach_await_opt x = x.foreach_await_opt
    let foreach_as x = x.foreach_as
    let foreach_key_opt x = x.foreach_key_opt
    let foreach_key_arrow_opt x = x.foreach_key_arrow_opt
    let foreach_value x = x.foreach_value
    let foreach_right_paren x = x.foreach_right_paren
    let foreach_statement x = x.foreach_statement
    let if_keyword x = x.if_keyword
    let if_left_paren x = x.if_left_paren
    let if_condition_expr x = x.if_condition_expr
    let if_right_paren x = x.if_right_paren
    let if_statement x = x.if_statement
    let if_elseif_clauses x = x.if_elseif_clauses
    let if_else_clause x = x.if_else_clause
    let elseif_keyword x = x.elseif_keyword
    let elseif_left_paren x = x.elseif_left_paren
    let elseif_condition_expr x = x.elseif_condition_expr
    let elseif_right_paren x = x.elseif_right_paren
    let elseif_statement x = x.elseif_statement
    let else_keyword x = x.else_keyword
    let else_statement x = x.else_statement
    let try_keyword x = x.try_keyword
    let try_compound_statement x = x.try_compound_statement
    let try_catch_clauses x = x.catch_clauses
    let try_finally_clause x = x.finally_clause
    let catch_keyword x = x.catch_keyword
    let catch_left_paren x = x.catch_left_paren
    let catch_params x = x.catch_params
    let catch_right_paren x = x.catch_right_paren
    let catch_compound_statement x = x.catch_compound_statement
    let finally_keyword x = x.finally_keyword
    let finally_compound_statement x = x.finally_compound_statement
    let do_keyword x = x.do_keyword
    let do_statement x = x.do_statement
    let do_while_keyword x = x.do_while_keyword
    let do_left_paren x = x.do_left_paren
    let do_condition_expr x = x.do_condition_expr
    let do_right_paren x = x.do_right_paren
    let do_semicolon x = x.do_semicolon
    let switch_keyword x = x.switch_keyword
    let switch_left_paren x = x.switch_left_paren
    let switch_expr x = x.switch_expr
    let switch_right_paren x = x.switch_right_paren
    let switch_compound_statement x = x.switch_compound_statement
    let case_keyword x = x.case_keyword
    let case_expr x = x.case_expr
    let case_colon x = x.case_colon
    let case_stmt x = x.case_stmt
    let default_keyword x = x.default_keyword
    let default_colon x = x.default_colon
    let default_stmt x = x.default_stmt
    let return_keyword x = x.return_keyword
    let return_expr x = x.return_expr
    let return_semicolon x = x.return_semicolon
    let throw_keyword x = x.throw_keyword
    let throw_expr x = x.throw_expr
    let throw_semicolon x = x.throw_semicolon
    let break_keyword x = x.break_keyword
    let break_semicolon x = x.break_semicolon
    let continue_keyword x = x.continue_keyword
    let continue_semicolon x = x.continue_semicolon
    let expr_statement_expr x = x.expr_statement_expr
    let expr_statement_semicolon x = x.expr_statement_semicolon
    let unary_operator x = x.unary_operator
    let unary_operand x = x.unary_operand
    let binary_left_operand b = b.binary_left_operand
    let binary_operator b = b.binary_operator
    let binary_right_operand b = b.binary_right_operand
    let conditional_test x = x.conditional_test
    let conditional_question x = x.conditional_question
    let conditional_consequence x = x.conditional_consequence
    let conditional_colon x = x.conditional_colon
    let conditional_alternative x = x.conditional_alternative
    let paren_expr_left_paren x = x.paren_expr_left_paren
    let paren_expr x = x.paren_expr
    let paren_expr_right_paren x = x.paren_expr_right_paren
    let braced_expr_left_brace x = x.braced_expr_left_brace
    let braced_expr x = x.braced_expr
    let braced_expr_right_brace x = x.braced_expr_right_brace
    let listlike_keyword x = x.listlike_keyword
    let listlike_left_paren x = x.listlike_left_paren
    let listlike_members x = x.listlike_members
    let listlike_right_paren x = x.listlike_right_paren
    let array_creation_left_bracket x = x.array_creation_left_bracket
    let array_creation_members x = x.array_creation_members
    let array_creation_right_bracket x = x.array_creation_right_bracket
    let array_intrinsic_keyword x = x.array_intrinsic_keyword
    let array_intrinsic_left_paren x = x.array_intrinsic_left_paren
    let array_intrinsic_members x = x.array_intrinsic_members
    let array_intrinsic_right_paren x = x.array_intrinsic_right_paren
    let xhp_open x = x.xhp_open
    let xhp_body x = x.xhp_body
    let xhp_close x = x.xhp_close
    let xhp_open_name x = x.xhp_open_name
    let xhp_open_attrs x = x.xhp_open_attrs
    let xhp_open_right_angle x = x.xhp_open_right_angle
    let xhp_close_left_angle x = x.xhp_close_left_angle
    let xhp_close_name x = x.xhp_close_name
    let xhp_close_right_angle x = x.xhp_close_right_angle
    let xhp_attr_name x = x.xhp_attr_name
    let xhp_attr_equal x = x.xhp_attr_equal
    let xhp_attr_expr x = x.xhp_attr_expr
    let type_constant_left_type x = x.type_constant_left_type
    let type_constant_separator x = x.type_constant_separator
    let type_constant_right_type x = x.type_constant_right_type
    let generic_class_type x = x.generic_class_type
    let generic_arguments x = x.generic_arguments
    let type_arguments_left_angle x = x.type_arguments_left_angle
    let type_arguments x = x.type_arguments
    let type_arguments_right_angle x = x.type_arguments_right_angle
    let type_parameter_variance x = x.type_variance_opt
    let type_parameter_name x = x.type_name
    let type_parameter_constraints x = x.type_constraint_list_opt
    let type_constraint_token x = x.constraint_token
    let type_constraint_matched_type x = x.matched_type
    let binary_operator_kind b =
      match syntax b.binary_operator with
      | Token token ->
        let kind = Token.kind token in
        if Operator.is_trailing_operator_token kind then
          Some (Operator.trailing_from_token kind)
        else
          None
      | _ -> None

    let get_token node =
      match (syntax node) with
      | Token token -> Some token
      | _ -> None

    let leading_token node =
      let rec aux nodes =
        match nodes with
        | [] -> None
        | h :: t ->
          let token = get_token h in
          if token = None then
            let result = aux (children h) in
            if result = None then aux t else result
          else
            token in
      aux [node]

    let trailing_token node =
      let rec aux nodes =
        match nodes with
        | [] -> None
        | h :: t ->
          let token = get_token h in
          if token = None then
            let result = aux (List.rev (children h)) in
            if result = None then aux t else result
          else
            token in
      aux [node]

    let syntax_from_children kind ts =
      match kind, ts with
      | (SyntaxKind.Missing, []) -> Missing
      | (SyntaxKind.SyntaxList, x) -> SyntaxList x
      | (SyntaxKind.AnonymousFunction,
        [ anonymous_async; anonymous_function; anonymous_left_paren;
          anonymous_params; anonymous_right_paren; anonymous_colon;
          anonymous_type; anonymous_use; anonymous_body ]) ->
        AnonymousFunction
        { anonymous_async; anonymous_function; anonymous_left_paren;
          anonymous_params; anonymous_right_paren; anonymous_colon;
          anonymous_type; anonymous_use; anonymous_body }
      | (SyntaxKind.AnonymousFunctionUseClause,
        [ anonymous_use_token; anonymous_use_left_paren;
          anonymous_use_variables; anonymous_use_right_paren ]) ->
        AnonymousFunctionUseClause
        { anonymous_use_token; anonymous_use_left_paren;
          anonymous_use_variables; anonymous_use_right_paren }
      | (SyntaxKind.ListItem, [ list_item; list_separator ]) ->
        ListItem { list_item; list_separator }
      | (SyntaxKind.Error, x) -> Error x
      | (SyntaxKind.LiteralExpression, [x]) -> LiteralExpression x
      | (SyntaxKind.VariableExpression, [x]) -> VariableExpression x
      | (SyntaxKind.QualifiedNameExpression,[x]) -> QualifiedNameExpression x
      | (SyntaxKind.SimpleTypeSpecifier, [x]) -> SimpleTypeSpecifier x
      | (SyntaxKind.ScriptHeader,
        [ header_less_than; header_question; header_language ]) ->
        ScriptHeader { header_less_than; header_question; header_language }
      | (SyntaxKind.Script, [ script_header; script_declarations ]) ->
        Script { script_header; script_declarations }
      | (SyntaxKind.EnumDeclaration,
          [ enum_enum; enum_name; enum_colon; enum_base; enum_type;
            enum_left_brace; enum_enumerators; enum_right_brace ]) ->
          EnumDeclaration
          { enum_enum; enum_name; enum_colon; enum_base; enum_type;
            enum_left_brace; enum_enumerators; enum_right_brace }
      | (SyntaxKind.Enumerator,
        [ enumerator_name; enumerator_equal; enumerator_value ]) ->
        Enumerator
        { enumerator_name; enumerator_equal; enumerator_value }
      | (SyntaxKind.AliasDeclaration,
        [ alias_token; alias_name; alias_constraint;
          alias_equal; alias_type; alias_semicolon ]) ->
        AliasDeclaration
        { alias_token; alias_name; alias_constraint;
          alias_equal; alias_type; alias_semicolon }

      | (SyntaxKind.NamespaceDeclaration,
        [ namespace_token; namespace_name; namespace_body ]) ->
        NamespaceDeclaration
        { namespace_token; namespace_name; namespace_body }
      | (SyntaxKind.NamespaceBody,
        [ namespace_left_brace; namespace_declarations;
          namespace_right_brace ]) ->
        NamespaceBody
        { namespace_left_brace; namespace_declarations;
          namespace_right_brace }
      | (SyntaxKind.NamespaceUseDeclaration,
        [ namespace_use; namespace_use_clauses; namespace_use_semicolon ]) ->
        NamespaceUseDeclaration
        { namespace_use; namespace_use_clauses; namespace_use_semicolon }
      | (SyntaxKind.NamespaceUseClause,
        [ namespace_use_name; namespace_use_as; namespace_use_alias ]) ->
        NamespaceUseClause
        { namespace_use_name; namespace_use_as; namespace_use_alias }
      | (SyntaxKind.FunctionDeclaration, [ function_attr; function_async;
        function_token; function_name; function_type_params;
        function_left_paren; function_params; function_right_paren;
        function_colon; function_type; function_body]) ->
            FunctionDeclaration { function_attr; function_async;
              function_token; function_name; function_type_params;
              function_left_paren; function_params; function_right_paren;
              function_colon; function_type; function_body }
      | (SyntaxKind.ClassishDeclaration,
        [ classish_attr; classish_abstract; classish_final; classish_token;
          classish_name; classish_type_params; classish_extends;
          classish_extends_list; classish_implements; classish_implements_list;
          classish_body ]) ->
        ClassishDeclaration {
          classish_attr; classish_abstract; classish_final; classish_token;
          classish_name; classish_type_params; classish_extends;
          classish_extends_list; classish_implements; classish_implements_list;
          classish_body }
      | (SyntaxKind.ClassishBody,
        [ classish_body_left_brace; classish_body_elements;
          classish_body_right_brace ]) ->
        ClassishBody {
          classish_body_left_brace; classish_body_elements;
          classish_body_right_brace }
      | (SyntaxKind.ParameterDeclaration, [ param_attr; param_type; param_name;
        param_default ]) ->
        ParameterDeclaration { param_attr; param_type; param_name;
          param_default }
      | (SyntaxKind.DefaultArgumentSpecifier, [ default_equal;
        default_value ]) ->
        DefaultArgumentSpecifier { default_equal; default_value }
      | SyntaxKind.AttributeSpecification, [ attribute_spec_left_double_angle;
        attribute_spec_attribute_list; attribute_spec_right_double_angle ] ->
        AttributeSpecification { attribute_spec_left_double_angle;
          attribute_spec_attribute_list; attribute_spec_right_double_angle }
      | SyntaxKind.Attribute, [ attribute_name; attribute_left_paren;
        attribute_values; attribute_right_paren ] ->
        Attribute { attribute_name; attribute_left_paren; attribute_values;
          attribute_right_paren }
      | (SyntaxKind.InclusionDirective ,
        [ inclusion_require; inclusion_left_paren; inclusion_filename;
          inclusion_right_paren; inclusion_semicolon ]) ->
        InclusionDirective
        { inclusion_require; inclusion_left_paren; inclusion_filename;
          inclusion_right_paren; inclusion_semicolon }
      | (SyntaxKind.CompoundStatement, [ compound_left_brace;
        compound_statements; compound_right_brace ]) ->
        CompoundStatement { compound_left_brace; compound_statements;
          compound_right_brace }
      | (SyntaxKind.ExpressionStatement, [ expr_statement_expr;
        expr_statement_semicolon ]) ->
        ExpressionStatement { expr_statement_expr; expr_statement_semicolon }
      | (SyntaxKind.WhileStatement, [ while_keyword; while_left_paren;
        while_condition_expr; while_right_paren; while_body ]) ->
        WhileStatement{ while_keyword; while_left_paren;
          while_condition_expr; while_right_paren; while_body }
      | (SyntaxKind.IfStatement, [ if_keyword; if_left_paren; if_condition_expr;
        if_right_paren; if_statement; if_elseif_clauses; if_else_clause ]) ->
        IfStatement { if_keyword; if_left_paren; if_condition_expr;
          if_right_paren; if_statement; if_elseif_clauses; if_else_clause }
      | (SyntaxKind.ElseifClause, [ elseif_keyword; elseif_left_paren;
        elseif_condition_expr; elseif_right_paren; elseif_statement ]) ->
        ElseifClause { elseif_keyword; elseif_left_paren;
          elseif_condition_expr; elseif_right_paren; elseif_statement }
      | (SyntaxKind.ElseClause, [ else_keyword; else_statement ]) ->
        ElseClause { else_keyword; else_statement }

      | SyntaxKind.TryStatement, [try_keyword; try_compound_statement;
        catch_clauses; finally_clause] ->
        TryStatement {try_keyword; try_compound_statement; catch_clauses;
          finally_clause}
      | SyntaxKind.CatchClause, [catch_keyword; catch_left_paren; catch_params;
        catch_right_paren; catch_compound_statement] ->
        CatchClause {catch_keyword; catch_left_paren; catch_params;
          catch_right_paren; catch_compound_statement}
      | SyntaxKind.FinallyClause, [finally_keyword;
        finally_compound_statement] ->
        FinallyClause {finally_keyword; finally_compound_statement}
      | (SyntaxKind.DoStatement, [ do_keyword; do_statement;
        do_while_keyword; do_left_paren; do_condition_expr;
        do_right_paren; do_semicolon ]) ->
        DoStatement { do_keyword; do_statement;
          do_while_keyword; do_left_paren; do_condition_expr;
          do_right_paren; do_semicolon }
      | (SyntaxKind.ForStatement, [ for_keyword; for_left_paren;
        for_initializer_expr; for_first_semicolon; for_control_expr;
        for_second_semicolon; for_end_of_loop_expr; for_right_paren;
        for_statement ]) ->
        ForStatement { for_keyword; for_left_paren;
          for_initializer_expr; for_first_semicolon; for_control_expr;
          for_second_semicolon; for_end_of_loop_expr; for_right_paren;
          for_statement }
      | (SyntaxKind.ForeachStatement, [ foreach_keyword; foreach_left_paren;
          foreach_collection_name; foreach_await_opt; foreach_as;
          foreach_key_opt; foreach_key_arrow_opt; foreach_value;
          foreach_right_paren; foreach_statement ]) ->
        ForeachStatement { foreach_keyword; foreach_left_paren;
          foreach_collection_name; foreach_await_opt; foreach_as;
          foreach_key_opt; foreach_key_arrow_opt; foreach_value;
          foreach_right_paren; foreach_statement }
      | (SyntaxKind.SwitchStatement, [ switch_keyword; switch_left_paren;
        switch_expr; switch_right_paren; switch_compound_statement ]) ->
        SwitchStatement{ switch_keyword; switch_left_paren;
          switch_expr; switch_right_paren; switch_compound_statement }
      | (SyntaxKind.CaseStatement, [ case_keyword;
        case_expr; case_colon; case_stmt ]) ->
        CaseStatement { case_keyword; case_expr; case_colon; case_stmt }
      | (SyntaxKind.DefaultStatement, [ default_keyword;
          default_colon; default_stmt ]) ->
        DefaultStatement { default_keyword; default_colon; default_stmt }
      | (SyntaxKind.ReturnStatement, [ return_keyword;
        return_expr; return_semicolon ]) ->
        ReturnStatement { return_keyword; return_expr; return_semicolon }
      | (SyntaxKind.ThrowStatement, [ throw_keyword;
        throw_expr; throw_semicolon ]) ->
        ThrowStatement { throw_keyword; throw_expr; throw_semicolon }
      | (SyntaxKind.BreakStatement, [ break_keyword; break_semicolon ]) ->
        BreakStatement { break_keyword; break_semicolon }
      | (SyntaxKind.ContinueStatement,
          [ continue_keyword; continue_semicolon ]) ->
        ContinueStatement { continue_keyword; continue_semicolon }
      | (SyntaxKind.FunctionStaticStatement,
        [ static_static; static_declarations; static_semicolon ]) ->
        FunctionStaticStatement
        { static_static; static_declarations; static_semicolon }
      | (SyntaxKind.StaticInitializer,
        [ static_init_equal; static_init_value ]) ->
        StaticInitializer
        { static_init_equal; static_init_value }
      | (SyntaxKind.StaticDeclarator,
        [ static_name; static_init ]) ->
        StaticDeclarator
        { static_name; static_init }
      | (SyntaxKind.PrefixUnaryOperator, [ unary_operator; unary_operand ]) ->
        PrefixUnaryOperator { unary_operator; unary_operand }
      | (SyntaxKind.PostfixUnaryOperator, [ unary_operand; unary_operator ]) ->
        PostfixUnaryOperator { unary_operand; unary_operator }
      | (SyntaxKind.BinaryOperator, [ binary_left_operand; binary_operator;
        binary_right_operand ]) ->
        BinaryOperator { binary_left_operand; binary_operator;
        binary_right_operand }
      | (SyntaxKind.ConditionalExpression,
        [ conditional_test; conditional_question; conditional_consequence;
          conditional_colon; conditional_alternative ]) ->
        ConditionalExpression
        { conditional_test; conditional_question; conditional_consequence;
          conditional_colon; conditional_alternative }
      | (SyntaxKind.FunctionCallExpression,
          [ function_call_receiver; function_call_lparen;
            function_call_arguments; function_call_rparen ]) ->
        FunctionCallExpression
          { function_call_receiver; function_call_lparen;
            function_call_arguments; function_call_rparen }
      | (SyntaxKind.ParenthesizedExpression, [ paren_expr_left_paren;
        paren_expr; paren_expr_right_paren ]) ->
        ParenthesizedExpression { paren_expr_left_paren; paren_expr;
          paren_expr_right_paren }
      | (SyntaxKind.BracedExpression, [ braced_expr_left_brace;
        braced_expr; braced_expr_right_brace ]) ->
        BracedExpression { braced_expr_left_brace; braced_expr;
          braced_expr_right_brace }
      | (SyntaxKind.ListExpression, [ listlike_keyword; listlike_left_paren;
        listlike_members; listlike_right_paren ]) ->
        ListExpression { listlike_keyword; listlike_left_paren;
          listlike_members; listlike_right_paren }
      | (SyntaxKind.ObjectCreationExpression,
        [ object_creation_new; object_creation_class; object_creation_lparen;
          object_creation_arguments; object_creation_rparen ]) ->
        ObjectCreationExpression
        { object_creation_new; object_creation_class; object_creation_lparen;
          object_creation_arguments; object_creation_rparen }
      | (SyntaxKind.ShapeExpression,
        [ shape_shape; shape_left_paren; shape_fields; shape_right_paren ]) ->
        ShapeExpression
        { shape_shape; shape_left_paren; shape_fields; shape_right_paren }
      | (SyntaxKind.FieldInitializer,
        [ field_init_name; field_init_arrow; field_init_value ]) ->
        FieldInitializer
        { field_init_name; field_init_arrow; field_init_value }
      | SyntaxKind.ArrayCreationExpression, [ array_creation_left_bracket;
        array_creation_members; array_creation_right_bracket ] ->
        ArrayCreationExpression { array_creation_left_bracket;
          array_creation_members; array_creation_right_bracket }
      | SyntaxKind.ArrayIntrinsicExpression, [ array_intrinsic_keyword;
        array_intrinsic_left_paren; array_intrinsic_members;
        array_intrinsic_right_paren ] ->
        ArrayIntrinsicExpression { array_intrinsic_keyword;
          array_intrinsic_left_paren; array_intrinsic_members;
          array_intrinsic_right_paren }
      | (SyntaxKind.XHPExpression, [ xhp_open; xhp_body; xhp_close ]) ->
        XHPExpression { xhp_open; xhp_body; xhp_close }
      | (SyntaxKind.XHPOpen, [ xhp_open_name; xhp_open_attrs;
          xhp_open_right_angle ]) ->
        XHPOpen { xhp_open_name; xhp_open_attrs; xhp_open_right_angle }
      | (SyntaxKind.XHPClose, [ xhp_close_left_angle; xhp_close_name;
          xhp_close_right_angle ]) ->
        XHPClose { xhp_close_left_angle; xhp_close_name; xhp_close_right_angle }
      | (SyntaxKind.XHPAttribute, [ xhp_attr_name; xhp_attr_equal;
          xhp_attr_expr ]) ->
        XHPAttribute { xhp_attr_name; xhp_attr_equal; xhp_attr_expr }
      | (SyntaxKind.TypeConstant, [ type_constant_left_type;
          type_constant_separator; type_constant_right_type ]) ->
        TypeConstant { type_constant_left_type; type_constant_separator;
          type_constant_right_type }
      | (SyntaxKind.GenericTypeSpecifier, [ generic_class_type;
          generic_arguments ]) ->
        GenericTypeSpecifier { generic_class_type; generic_arguments }
      | (SyntaxKind.NullableTypeSpecifier,
          [ nullable_question; nullable_type ]) ->
          NullableTypeSpecifier
          { nullable_question; nullable_type }
      | (SyntaxKind.TypeArguments, [ type_arguments_left_angle;
          type_arguments; type_arguments_right_angle ]) ->
        TypeArguments { type_arguments_left_angle;
            type_arguments; type_arguments_right_angle }
      | (SyntaxKind.TypeParameter, [ type_variance_opt;
          type_name; type_constraint_list_opt  ]) ->
        TypeParameter { type_variance_opt;
          type_name; type_constraint_list_opt  }
      | (SyntaxKind.TypeConstraint, [ constraint_token;
          matched_type ]) ->
        TypeConstraint { constraint_token; matched_type }
      | (SyntaxKind.TupleTypeSpecifier,
          [ tuple_left_paren; tuple_types; tuple_right_paren ]) ->
        TupleTypeSpecifier
          { tuple_left_paren; tuple_types; tuple_right_paren }
      | (SyntaxKind.VectorTypeSpecifier,
        [ vector_array; vector_left_angle; vector_type; vector_right_angle ]) ->
        VectorTypeSpecifier
        { vector_array; vector_left_angle; vector_type; vector_right_angle }
      | (SyntaxKind.MapTypeSpecifier,
        [ map_array; map_left_angle; map_key; map_comma; map_value;
          map_right_angle ]) ->
        MapTypeSpecifier
        { map_array; map_left_angle; map_key; map_comma; map_value;
          map_right_angle }
      | (SyntaxKind.ClosureTypeSpecifier,
        [ closure_outer_left_paren; closure_function;
          closure_inner_left_paren; closure_parameter_types;
          closure_inner_right_paren; closure_colon; closure_return_type;
          closure_outer_right_paren ]) ->
        ClosureTypeSpecifier
        { closure_outer_left_paren; closure_function;
          closure_inner_left_paren; closure_parameter_types;
          closure_inner_right_paren; closure_colon; closure_return_type;
          closure_outer_right_paren }
      | (SyntaxKind.ClassnameTypeSpecifier,
        [ classname_classname; classname_left_angle; classname_type;
          classname_right_angle ]) ->
        ClassnameTypeSpecifier
        { classname_classname; classname_left_angle; classname_type;
          classname_right_angle }
      | (SyntaxKind.ShapeTypeSpecifier,
        [ shape_shape; shape_left_paren; shape_fields; shape_right_paren ]) ->
        ShapeTypeSpecifier
        { shape_shape; shape_left_paren; shape_fields; shape_right_paren }
      | (SyntaxKind.FieldSpecifier,
        [ field_name; field_arrow; field_type ]) ->
        FieldSpecifier
        { field_name; field_arrow; field_type }

      | _ -> failwith "with_children called with wrong number of children"

    let all_tokens node =
      let rec aux acc nodes =
        match nodes with
        | [] -> acc
        | h :: t ->
          begin
            match syntax h with
            | Token token -> aux (token :: acc) t
            | _ -> aux (aux acc (children h)) t
          end in
      List.rev (aux [] [node])

    module type ValueBuilderType = sig
      val value_from_children:
        Full_fidelity_syntax_kind.t -> t list -> SyntaxValue.t
      val value_from_token: Token.t -> SyntaxValue.t
    end

    module WithValueBuilder(ValueBuilder: ValueBuilderType) = struct
      let from_children kind ts =
        let syntax = syntax_from_children kind ts in
        let value = ValueBuilder.value_from_children kind ts in
        make syntax value

      let make_token token =
        let syntax = Token token in
        let value = ValueBuilder.value_from_token token in
        make syntax value

      let make_missing () =
        from_children SyntaxKind.Missing []

      let make_prefix_unary_operator unary_operator unary_operand =
        from_children SyntaxKind.PrefixUnaryOperator
          [ unary_operator; unary_operand ]

      let make_postfix_unary_operator unary_operator unary_operand =
        from_children SyntaxKind.PostfixUnaryOperator
          [ unary_operand; unary_operator ]

      let make_binary_operator
        binary_left_operand binary_operator binary_right_operand =
          from_children SyntaxKind.BinaryOperator
            [ binary_left_operand; binary_operator; binary_right_operand ]

      let make_conditional_expression
        conditional_test conditional_question conditional_consequence
        conditional_colon conditional_alternative =
        from_children SyntaxKind.ConditionalExpression
        [ conditional_test; conditional_question; conditional_consequence;
          conditional_colon; conditional_alternative ]

      let make_function_call_expression
          function_call_receiver function_call_lparen
          function_call_arguments function_call_rparen =
        from_children SyntaxKind.FunctionCallExpression
          [ function_call_receiver; function_call_lparen;
            function_call_arguments; function_call_rparen ]

      let make_anonymous_function
          async func left params right colon return_type uses body =
        from_children SyntaxKind.AnonymousFunction
          [async; func; left; params; right; colon; return_type; uses; body ]

      let make_anonymous_function_use_clause use_token left vars right =
        from_children SyntaxKind.AnonymousFunctionUseClause
          [ use_token; left; vars; right ]

      let make_parenthesized_expression
        paren_expr_left_paren paren_expr paren_expr_right_paren =
          from_children SyntaxKind.ParenthesizedExpression
            [ paren_expr_left_paren; paren_expr; paren_expr_right_paren ]

      let make_braced_expression
        braced_expr_left_brace braced_expr braced_expr_right_brace =
          from_children SyntaxKind.BracedExpression
            [ braced_expr_left_brace; braced_expr; braced_expr_right_brace ]

      let make_listlike_expression
        listlike_keyword listlike_left_paren listlike_members
        listlike_right_paren =
        from_children SyntaxKind.ListExpression
          [ listlike_keyword; listlike_left_paren; listlike_members;
          listlike_right_paren ]

      let make_object_creation_expression
        object_creation_new object_creation_class object_creation_lparen
        object_creation_arguments object_creation_rparen =
        from_children SyntaxKind.ObjectCreationExpression
        [ object_creation_new; object_creation_class; object_creation_lparen;
          object_creation_arguments; object_creation_rparen]

      let make_field_initializer name arrow value =
        from_children SyntaxKind.FieldInitializer [ name; arrow; value ]

      let make_shape_expression shape lparen fields rparen =
        from_children SyntaxKind.ShapeExpression
          [ shape; lparen; fields; rparen ]

      let make_array_creation_expression
        array_creation_left_bracket array_creation_members
        array_creation_right_bracket =
        from_children SyntaxKind.ArrayCreationExpression
          [array_creation_left_bracket; array_creation_members;
          array_creation_right_bracket ]

      let make_array_intrinsic_expression
        array_intrinsic_keyword array_intrinsic_left_paren
        array_intrinsic_members array_intrinsic_right_paren =
        from_children SyntaxKind.ArrayIntrinsicExpression
          [ array_intrinsic_keyword; array_intrinsic_left_paren;
          array_intrinsic_members; array_intrinsic_right_paren ]

      let make_xhp xhp_open xhp_body xhp_close =
        from_children SyntaxKind.XHPExpression [xhp_open; xhp_body; xhp_close ]

      let make_xhp_open xhp_open_name xhp_open_attrs xhp_open_right_angle =
        from_children SyntaxKind.XHPOpen
          [xhp_open_name; xhp_open_attrs; xhp_open_right_angle ]

      let make_xhp_close
          xhp_close_left_angle xhp_close_name xhp_close_right_angle =
        from_children SyntaxKind.XHPClose
          [xhp_close_left_angle; xhp_close_name; xhp_close_right_angle ]

      let make_xhp_attr xhp_attr_name xhp_attr_equal xhp_attr_expr =
        from_children SyntaxKind.XHPAttribute
          [ xhp_attr_name; xhp_attr_equal; xhp_attr_expr ]

      let make_list items =
        match items with
        | [] -> make_missing()
        | h :: [] -> h
        | _ -> from_children SyntaxKind.SyntaxList items

      let make_list_item item separator =
        from_children SyntaxKind.ListItem [item; separator]

      let make_error items =
        from_children SyntaxKind.Error items

      let make_script_header header_less_than header_question header_language =
        from_children SyntaxKind.ScriptHeader
          [ header_less_than; header_question; header_language ]

      let make_script script_header script_declarations =
        from_children SyntaxKind.Script [ script_header; script_declarations ]

      let make_enum
          enum name colon base enum_type left_brace items right_brace =
        from_children SyntaxKind.EnumDeclaration
          [ enum; name; colon; base; enum_type; left_brace; items; right_brace ]

      let make_enumerator name equal value =
        from_children SyntaxKind.Enumerator [ name; equal; value ]

      let make_alias token name constr equal ty semi =
        from_children SyntaxKind.AliasDeclaration
          [ token; name; constr; equal; ty; semi ]

      let make_namespace token name body =
        from_children SyntaxKind.NamespaceDeclaration
          [ token; name; body ]

      let make_namespace_body left decls right =
        from_children SyntaxKind.NamespaceBody
          [ left; decls; right ]

      let make_namespace_use use clauses semi =
        from_children SyntaxKind.NamespaceUseDeclaration
          [ use; clauses; semi ]

      let make_namespace_use_clause name as_token alias =
        from_children SyntaxKind.NamespaceUseClause [ name; as_token; alias ]

      let make_function function_attr function_async function_token
        function_name function_type_params function_left_paren function_params
        function_right_paren function_colon function_type function_body =
      from_children SyntaxKind.FunctionDeclaration [
        function_attr; function_async; function_token; function_name;
        function_type_params; function_left_paren; function_params;
        function_right_paren; function_colon; function_type; function_body ]

    let make_classish classish_attr classish_abstract classish_final
      classish_token classish_name classish_type_params classish_extends
      classish_extends_list classish_implements classish_implements_list
      classish_body =
      from_children SyntaxKind.ClassishDeclaration [
        classish_attr; classish_abstract; classish_final; classish_token;
        classish_name; classish_type_params; classish_extends;
        classish_extends_list; classish_implements; classish_implements_list;
        classish_body ]

    let make_classish_body classish_body_left_brace classish_body_elements
      classish_body_right_brace =
      from_children SyntaxKind.ClassishBody [
        classish_body_left_brace; classish_body_elements;
        classish_body_right_brace ]

      let make_parameter_declaration
        param_attr param_type param_name param_default =
        from_children SyntaxKind.ParameterDeclaration
          [ param_attr; param_type; param_name; param_default ]

      let make_default_argument_specifier default_equal default_value =
        from_children SyntaxKind.DefaultArgumentSpecifier
          [ default_equal; default_value ]

      let make_attribute_specification attribute_spec_left_double_angle
        attribute_spec_attribute_list attribute_spec_right_double_angle =
        from_children SyntaxKind.AttributeSpecification
          [ attribute_spec_left_double_angle; attribute_spec_attribute_list;
          attribute_spec_right_double_angle ]

      let make_attribute attribute_name attribute_left_paren attribute_values
        attribute_right_paren =
        from_children SyntaxKind.Attribute
        [ attribute_name; attribute_left_paren; attribute_values;
          attribute_right_paren ]

      let make_inclusion_directive
          require left_paren filename right_paren semicolon =
        from_children SyntaxKind.InclusionDirective
          [ require; left_paren; filename; right_paren; semicolon ]

      let make_compound_statement
        compound_left_brace compound_statements compound_right_brace =
        from_children SyntaxKind.CompoundStatement
          [ compound_left_brace; compound_statements; compound_right_brace ]

      let make_expression_statement expr_statement_expr
        expr_statement_semicolon =
        from_children SyntaxKind.ExpressionStatement
          [ expr_statement_expr; expr_statement_semicolon ]

      let make_while_statement
        while_keyword while_left_paren while_condition_expr
        while_right_paren while_body =
        from_children SyntaxKind.WhileStatement
          [ while_keyword; while_left_paren; while_condition_expr;
          while_right_paren; while_body ]

      let make_if_statement
        if_keyword if_left_paren if_condition_expr if_right_paren if_statement
        if_elseif_clauses if_else_clause =
        from_children SyntaxKind.IfStatement
          [ if_keyword; if_left_paren; if_condition_expr; if_right_paren;
          if_statement; if_elseif_clauses; if_else_clause ]

      let make_elseif_clause
        elseif_keyword elseif_left_paren elseif_condition_expr
        elseif_right_paren elseif_statement =
        from_children SyntaxKind.ElseifClause
          [ elseif_keyword; elseif_left_paren; elseif_condition_expr;
          elseif_right_paren; elseif_statement ]

      let make_else_clause else_keyword else_statement =
        from_children SyntaxKind.ElseClause [ else_keyword; else_statement ]

      let make_try_statement
        try_keyword try_compound_statement catch_clauses finally_clause =
          from_children SyntaxKind.TryStatement
          [ try_keyword; try_compound_statement; catch_clauses; finally_clause ]

      let make_catch_clause catch_keyword catch_left_paren catch_params
        catch_right_paren catch_compound_statement =
        from_children SyntaxKind.CatchClause
          [ catch_keyword; catch_left_paren; catch_params; catch_right_paren;
          catch_compound_statement ]

      let make_finally_clause finally_keyword finally_compound_statement =
        from_children SyntaxKind.FinallyClause
          [finally_keyword; finally_compound_statement]

      let make_do_statement
        do_keyword do_statement do_while_keyword do_left_paren
        do_condition_expr do_right_paren do_semicolon =
        from_children SyntaxKind.DoStatement
          [ do_keyword; do_statement; do_while_keyword; do_left_paren;
          do_condition_expr; do_right_paren; do_semicolon ]

      let make_for_statement
        for_keyword for_left_paren for_initializer_expr for_first_semicolon
        for_control_expr for_second_semicolon for_end_of_loop_expr
        for_right_paren for_statement =
        from_children SyntaxKind.ForStatement
        [ for_keyword; for_left_paren; for_initializer_expr;
        for_first_semicolon; for_control_expr; for_second_semicolon;
        for_end_of_loop_expr; for_right_paren; for_statement ]

      let make_foreach_statement foreach_keyword foreach_left_paren
        foreach_collection_name foreach_await_opt foreach_as foreach_key_opt
        foreach_arrow foreach_value foreach_right_paren foreach_statement =
        from_children SyntaxKind.ForeachStatement
        [ foreach_keyword; foreach_left_paren; foreach_collection_name;
        foreach_await_opt; foreach_as; foreach_key_opt; foreach_arrow;
        foreach_value; foreach_right_paren; foreach_statement]

      let make_switch_statement
        switch_keyword switch_left_paren switch_expr
        switch_right_paren switch_compound_statement =
        from_children SyntaxKind.SwitchStatement
          [ switch_keyword; switch_left_paren; switch_expr;
          switch_right_paren; switch_compound_statement ]

      let make_case_statement case_keyword case_expr case_colon case_stmt =
        from_children SyntaxKind.CaseStatement
          [ case_keyword; case_expr; case_colon; case_stmt ]

      let make_default_statement default_keyword default_colon default_stmt =
        from_children SyntaxKind.DefaultStatement
          [ default_keyword; default_colon; default_stmt ]

      let make_return_statement return_keyword return_expr return_semicolon =
        from_children SyntaxKind.ReturnStatement
          [ return_keyword; return_expr; return_semicolon ]

      let make_throw_statement throw_keyword throw_expr throw_semicolon =
        from_children SyntaxKind.ThrowStatement
          [ throw_keyword; throw_expr; throw_semicolon ]

      let make_break_statement break_keyword break_semicolon =
        from_children SyntaxKind.BreakStatement
          [ break_keyword; break_semicolon ]

      let make_continue_statement continue_keyword continue_semicolon =
        from_children SyntaxKind.ContinueStatement
          [ continue_keyword; continue_semicolon ]

      let make_function_static_statement static decls semi =
        from_children SyntaxKind.FunctionStaticStatement [ static; decls; semi ]

      let make_static_initializer equal value =
        from_children SyntaxKind.StaticInitializer [ equal; value ]

      let make_static_declarator variable init =
        from_children SyntaxKind.StaticDeclarator [ variable; init ]

      let make_type_constant type_constant_left_type type_constant_separator
          type_constant_right_type =
        from_children SyntaxKind.TypeConstant
          [ type_constant_left_type; type_constant_separator;
          type_constant_right_type ]

      let make_type_constraint constraint_token matched_type =
        from_children SyntaxKind.TypeConstraint
          [ constraint_token; matched_type ]

      let make_type_parameter variance_opt type_name constraint_list_opt =
        from_children SyntaxKind.TypeParameter
          [ variance_opt; type_name; constraint_list_opt ]

      let make_simple_type_specifier simple_type =
        from_children SyntaxKind.SimpleTypeSpecifier [ simple_type ]

      let make_nullable_type_specifier nullable_question nullable_type =
        from_children SyntaxKind.NullableTypeSpecifier
          [ nullable_question; nullable_type ]

      let make_generic_type_specifier generic_class_type generic_arguments =
        from_children SyntaxKind.GenericTypeSpecifier
          [ generic_class_type; generic_arguments ]

      let make_type_arguments left items right =
        from_children SyntaxKind.TypeArguments [ left; items; right ]

      let make_tuple_type_specifier left types right =
        from_children SyntaxKind.TupleTypeSpecifier [ left; types; right ]

      let make_vector_type_specifier
          vector_array vector_left_angle vector_type vector_right_angle =
        from_children SyntaxKind.VectorTypeSpecifier
          [ vector_array; vector_left_angle; vector_type; vector_right_angle ]

      let make_map_type_specifier
          map_array map_left_angle map_key map_comma map_value map_right_angle =
        from_children SyntaxKind.MapTypeSpecifier
          [ map_array; map_left_angle; map_key; map_comma; map_value;
            map_right_angle ]

      let make_closure_type_specifier
          closure_outer_left_paren closure_function
          closure_inner_left_paren closure_parameter_types
          closure_inner_right_paren closure_colon closure_return_type
          closure_outer_right_paren =
        from_children SyntaxKind.ClosureTypeSpecifier
          [ closure_outer_left_paren; closure_function;
            closure_inner_left_paren; closure_parameter_types;
            closure_inner_right_paren; closure_colon; closure_return_type;
            closure_outer_right_paren ]
      let make_classname_type_specifier classname left classname_type right =
        from_children SyntaxKind.ClassnameTypeSpecifier
          [ classname; left; classname_type; right ]

      let make_field_specifier name arrow field_type =
        from_children SyntaxKind.FieldSpecifier [ name; arrow; field_type ]

      let make_shape_type_specifier shape lparen fields rparen =
        from_children SyntaxKind.ShapeTypeSpecifier
          [ shape; lparen; fields; rparen ]

      let make_literal_expression literal =
        from_children SyntaxKind.LiteralExpression [ literal ]

      let make_variable_expression variable =
        from_children SyntaxKind.VariableExpression [ variable ]

      let make_qualified_name_expression name =
        from_children SyntaxKind.QualifiedNameExpression [ name ]

    end (* WithValueBuilder *)
  end (* WithSyntaxValue *)
end (* WithToken *)
