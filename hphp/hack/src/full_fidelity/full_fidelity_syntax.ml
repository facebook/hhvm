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
      enumerator_value : t;
      enumerator_semicolon : t
    }
    and alias_declaration = {
      alias_attribute_spec : t;
      alias_token : t;
      alias_name : t;
      alias_generic_parameter : t;
      alias_constraint : t;
      alias_equal : t;
      alias_type : t;
      alias_semicolon : t
    }
    and property_declaration = {
      prop_modifiers : t;
      prop_type : t;
      prop_declarators : t;
      prop_semicolon : t
    }
    and property_declarator = {
      prop_name : t;
      prop_init : t
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
      namespace_use_kind : t;
      namespace_use_clauses : t;
      namespace_use_semicolon : t
    }
    and namespace_group_use_declaration = {
      namespace_group_use : t;
      namespace_group_use_kind : t;
      namespace_group_use_prefix : t;
      namespace_group_use_left_brace : t;
      namespace_group_use_clauses : t;
      namespace_group_use_right_brace : t;
      namespace_group_use_semicolon : t
    }
    and namespace_use_clause = {
      namespace_use_clause_kind : t;
      namespace_use_name : t;
      namespace_use_as : t;
      namespace_use_alias : t
    }
    and function_declaration = {
      function_attribute_spec : t;
      function_declaration_header : t;
      function_body : t
    }
    and function_declaration_header = {
      function_async : t;
      function_token : t;
      function_name : t;
      function_type_params : t;
      function_left_paren : t;
      function_params : t;
      function_right_paren : t;
      function_colon : t;
      function_type : t;
    }
    and methodish_declaration = {
        methodish_attr : t;
        methodish_modifiers : t;
        methodish_function_decl_header : t;
        (* Only one of function body and semicolon can be used *)
        methodish_function_body : t;
        methodish_semicolon : t;
    }
    and classish_declaration = {
      classish_attr : t;
      classish_modifiers : t;
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
    and trait_use = {
      trait_use_token : t;
      trait_use_name_list : t;
      trait_use_semicolon : t;
    }
    and require_clause = {
      require_token : t;
      require_kind : t;
      require_name : t;
      require_semicolon : t
    }
    and const_declaration = {
      const_abstract : t;
      const_token : t;
      const_type_specifier : t;
      const_declarator_list : t;
      const_semicolon : t;
    }
    and constant_declarator = {
      constant_declarator_name : t;
      constant_declarator_initializer : t;
    }
    and type_const_declaration = {
      type_const_abstract : t;
      type_const_const_token : t;
      type_const_type_token : t;
      type_const_name : t;
      type_const_type_constraint : t;
      type_const_equal : t;
      type_const_type_specifier : t;
      type_const_semicolon : t;
    }
    and decorated_expression = {
      decorated_expression_decorator: t;
      decorated_expression_expression: t;
    }
    and parameter_declaration = {
      param_attr : t;
      param_visibility : t;
      param_type : t;
      param_name : t;
      param_default : t
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
    and inclusion_expression = {
      inclusion_require : t;
      inclusion_filename : t;
    }
    and inclusion_directive = {
      inclusion_expression : t;
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
      catch_type: t;
      catch_variable: t;
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
      break_level: t;
      break_semicolon: t
    }
    and continue_statement = {
      continue_keyword: t;
      continue_level: t;
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
    and echo_statement = {
      echo_token : t;
      echo_expression_list: t;
      echo_semicolon: t;
    }
    and simple_initializer = {
      simple_init_equal : t;
      simple_init_value : t
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
    and lambda_expression = {
      lambda_async : t;
      lambda_signature : t;
      lambda_arrow : t;
      lambda_body : t
    }
    and lambda_signature = {
      lambda_left_paren : t;
      lambda_params : t;
      lambda_right_paren : t;
      lambda_colon : t;
      lambda_type : t
    }
    and cast_expression = {
      cast_left_paren : t;
      cast_type : t;
      cast_right_paren : t;
      cast_operand : t
    }
    and scope_resolution_expression = {
      scope_resolution_qualifier : t;
      scope_resolution_operator : t;
      scope_resolution_name : t
    }
    and member_selection_expression = {
      member_object : t;
      member_operator : t;
      member_name : t
    }
    and yield_expression = {
      yield_token : t;
      yield_operand : t
    }
    and print_expression = {
      print_token : t;
      print_expr : t;
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
    and instanceof_expression = {
      instanceof_left_operand : t;
      instanceof_operator : t;
      instanceof_right_operand : t
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
    and collection_literal_expression = {
      collection_literal_name : t;
      collection_literal_left_brace : t;
      collection_literal_initialization_list : t;
      collection_literal_right_brace : t;
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
    and element_initializer = {
      element_key : t;
      element_arrow : t;
      element_value : t
    }
    and subscript_expression = {
      subscript_receiver : t;
      subscript_left : t;
      subscript_index : t;
      subscript_right : t
    }
    and awaitable_creation_expression = {
      awaitable_async : t;
      awaitable_compound_statement : t;
    }
    and xhp_children_declaration = {
      xhp_children : t;
      xhp_children_expression : t;
      xhp_children_semicolon : t
    }
    and xhp_category_declaration = {
      xhp_category : t;
      xhp_category_list : t;
      xhp_category_semicolon : t
    }
    and xhp_enum_type = {
      xhp_enum_token : t;
      xhp_enum_left_brace : t;
      xhp_enum_values : t;
      xhp_enum_right_brace : t
    }
    and xhp_required = {
      xhp_required_at : t;
      xhp_required : t
    }
    and xhp_class_attribute_declaration = {
      xhp_attr_token : t;
      xhp_attr_list : t;
      xhp_attr_semicolon : t
    }
    and xhp_class_attribute = {
      xhp_attr_decl_type : t;
      xhp_attr_decl_name : t;
      xhp_attr_decl_init : t;
      xhp_attr_decl_required : t
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
    and soft_type_specifier = {
      soft_at : t;
      soft_type : t
    }
    and type_arguments = {
      type_arguments_left_angle : t;
      type_arguments : t;
      type_arguments_right_angle : t
    }
    and type_parameters = {
      type_parameters_left_angle : t;
      type_parameters : t;
      type_parameters_right_angle : t
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
    | NamespaceGroupUseDeclaration of namespace_group_use_declaration
    | NamespaceUseDeclaration of namespace_use_declaration
    | NamespaceUseClause of namespace_use_clause
    | FunctionDeclaration of function_declaration
    | FunctionDeclarationHeader of function_declaration_header
    | MethodishDeclaration of methodish_declaration
    | ClassishDeclaration of classish_declaration
    | ClassishBody of classish_body
    | XHPChildrenDeclaration of xhp_children_declaration
    | XHPCategoryDeclaration of xhp_category_declaration
    | XHPEnumType of xhp_enum_type
    | XHPRequired of xhp_required
    | XHPClassAttributeDeclaration of xhp_class_attribute_declaration
    | XHPClassAttribute of xhp_class_attribute
    | TraitUse of trait_use
    | RequireClause of require_clause
    | ConstDeclaration of const_declaration
    | ConstantDeclarator of constant_declarator
    | TypeConstDeclaration of type_const_declaration
    | EnumDeclaration of enum_declaration
    | Enumerator of enumerator
    | AliasDeclaration of alias_declaration
    | PropertyDeclaration of property_declaration
    | PropertyDeclarator of property_declarator
    | DecoratedExpression of decorated_expression
    | ParameterDeclaration of parameter_declaration
    | AttributeSpecification of attribute_specification
    | Attribute of attribute
    | InclusionExpression of inclusion_expression
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
    | SimpleInitializer of simple_initializer
    | StaticDeclarator of static_declarator
    | EchoStatement of echo_statement

    | MemberSelectionExpression of member_selection_expression
    | SafeMemberSelectionExpression of member_selection_expression
    | ScopeResolutionExpression of scope_resolution_expression
    | YieldExpression of yield_expression
    | PrintExpression of print_expression
    | CastExpression of cast_expression
    | LambdaExpression of lambda_expression
    | LambdaSignature of lambda_signature
    | AnonymousFunction of anonymous_function
    | AnonymousFunctionUseClause of anonymous_use
    | LiteralExpression of t
    | VariableExpression of t
    | QualifiedNameExpression of t
    | PipeVariableExpression of t
    | PrefixUnaryOperator of unary_operator
    | PostfixUnaryOperator of unary_operator
    | BinaryOperator of binary_operator
    | InstanceofExpression of instanceof_expression
    | ConditionalExpression of conditional_expression
    | FunctionCallExpression of function_call_expression
    | ParenthesizedExpression of parenthesized_expression
    | BracedExpression of braced_expression
    | ListExpression of listlike_expression
    | CollectionLiteralExpression of collection_literal_expression
    | ObjectCreationExpression of object_creation_expression
    | ShapeExpression of shape
    | FieldInitializer of field_initializer
    | ArrayCreationExpression of array_creation_expression
    | ArrayIntrinsicExpression of array_intrinsic_expression
    | ElementInitializer of element_initializer
    | SubscriptExpression of subscript_expression
    | AwaitableCreationExpression of awaitable_creation_expression
    | XHPExpression of xhp_expression
    | XHPOpen of xhp_open
    | XHPAttribute of xhp_attribute
    | XHPClose of xhp_close

    | SimpleTypeSpecifier of t
    | NullableTypeSpecifier of nullable_type_specifier
    | SoftTypeSpecifier of soft_type_specifier
    | TypeConstraint of type_constraint_specifier
    | TypeParameter of type_parameter
    | TypeConstant of type_constant
    | GenericTypeSpecifier of generic_type
    | TypeArguments of type_arguments
    | TypeParameters of type_parameters
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
      | MemberSelectionExpression _ -> SyntaxKind.MemberSelectionExpression
      | SafeMemberSelectionExpression _ ->
        SyntaxKind.SafeMemberSelectionExpression
      | ScopeResolutionExpression _ -> SyntaxKind.ScopeResolutionExpression
      | YieldExpression _ -> SyntaxKind.YieldExpression
      | PrintExpression _ -> SyntaxKind.PrintExpression
      | CastExpression _ -> SyntaxKind.CastExpression
      | LambdaExpression _ -> SyntaxKind.LambdaExpression
      | LambdaSignature _ -> SyntaxKind.LambdaSignature
      | AnonymousFunction _ -> SyntaxKind.AnonymousFunction
      | AnonymousFunctionUseClause _ -> SyntaxKind.AnonymousFunctionUseClause
      | LiteralExpression _ -> SyntaxKind.LiteralExpression
      | VariableExpression _ -> SyntaxKind.VariableExpression
      | QualifiedNameExpression _ -> SyntaxKind.QualifiedNameExpression
      | PipeVariableExpression _ -> SyntaxKind.PipeVariableExpression
      | Error _ -> SyntaxKind.Error
      | SyntaxList _ -> SyntaxKind.SyntaxList
      | ListItem _ -> SyntaxKind.ListItem
      | ScriptHeader _ -> SyntaxKind.ScriptHeader
      | Script _ -> SyntaxKind.Script
      | EnumDeclaration _ -> SyntaxKind.EnumDeclaration
      | Enumerator _ -> SyntaxKind.Enumerator
      | AliasDeclaration _ -> SyntaxKind.AliasDeclaration
      | PropertyDeclaration _ -> SyntaxKind.PropertyDeclaration
      | PropertyDeclarator _ -> SyntaxKind.PropertyDeclarator
      | NamespaceDeclaration _ -> SyntaxKind.NamespaceDeclaration
      | NamespaceBody _ -> SyntaxKind.NamespaceBody
      | NamespaceGroupUseDeclaration _ ->
        SyntaxKind.NamespaceGroupUseDeclaration
      | NamespaceUseDeclaration _ -> SyntaxKind.NamespaceUseDeclaration
      | NamespaceUseClause _ -> SyntaxKind.NamespaceUseClause
      | FunctionDeclaration _ -> SyntaxKind.FunctionDeclaration
      | FunctionDeclarationHeader _ -> SyntaxKind.FunctionDeclarationHeader
      | MethodishDeclaration _ -> SyntaxKind.MethodishDeclaration
      | ClassishDeclaration _ -> SyntaxKind.ClassishDeclaration
      | ClassishBody _ -> SyntaxKind.ClassishBody
      | XHPChildrenDeclaration _ -> SyntaxKind.XHPChildrenDeclaration
      | XHPCategoryDeclaration _ -> SyntaxKind.XHPCategoryDeclaration
      | XHPEnumType _ -> SyntaxKind.XHPEnumType
      | XHPRequired _ -> SyntaxKind.XHPRequired
      | XHPClassAttributeDeclaration _ ->
        SyntaxKind.XHPClassAttributeDeclaration
      | XHPClassAttribute _ -> SyntaxKind.XHPClassAttribute
      | TraitUse _ -> SyntaxKind.TraitUse
      | RequireClause _ -> SyntaxKind.RequireClause
      | ConstDeclaration _ -> SyntaxKind.ConstDeclaration
      | ConstantDeclarator _ -> SyntaxKind.ConstantDeclarator
      | TypeConstDeclaration _ -> SyntaxKind.TypeConstDeclaration
      | DecoratedExpression _ -> SyntaxKind.DecoratedExpression
      | ParameterDeclaration _ -> SyntaxKind.ParameterDeclaration
      | AttributeSpecification _ -> SyntaxKind.AttributeSpecification
      | Attribute _ -> SyntaxKind.Attribute
      | InclusionExpression _ -> SyntaxKind.InclusionExpression
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
      | SimpleInitializer _ -> SyntaxKind.SimpleInitializer
      | StaticDeclarator _ -> SyntaxKind.StaticDeclarator
      | EchoStatement _ -> SyntaxKind.EchoStatement
      | PrefixUnaryOperator _ -> SyntaxKind.PrefixUnaryOperator
      | PostfixUnaryOperator _ -> SyntaxKind.PostfixUnaryOperator
      | BinaryOperator _ -> SyntaxKind.BinaryOperator
      | InstanceofExpression _ -> SyntaxKind.InstanceofExpression
      | ConditionalExpression _ -> SyntaxKind.ConditionalExpression
      | FunctionCallExpression _ -> SyntaxKind.FunctionCallExpression
      | ParenthesizedExpression _ -> SyntaxKind.ParenthesizedExpression
      | BracedExpression _ -> SyntaxKind.BracedExpression
      | ListExpression _ -> SyntaxKind.ListExpression
      | CollectionLiteralExpression _ -> SyntaxKind.CollectionLiteralExpression
      | ObjectCreationExpression _ -> SyntaxKind.ObjectCreationExpression
      | ShapeExpression _ -> SyntaxKind.ShapeExpression
      | FieldInitializer _ -> SyntaxKind.FieldInitializer
      | ArrayCreationExpression _ -> SyntaxKind.ArrayCreationExpression
      | ArrayIntrinsicExpression _ -> SyntaxKind.ArrayIntrinsicExpression
      | ElementInitializer _ -> SyntaxKind.ElementInitializer
      | SubscriptExpression _ -> SyntaxKind.SubscriptExpression
      | AwaitableCreationExpression _ -> SyntaxKind.AwaitableCreationExpression
      | XHPExpression _ -> SyntaxKind.XHPExpression
      | XHPOpen _ -> SyntaxKind.XHPOpen
      | XHPClose _ -> SyntaxKind.XHPClose
      | XHPAttribute _ -> SyntaxKind.XHPAttribute
      | TypeConstant _ ->  SyntaxKind.TypeConstant
      | SimpleTypeSpecifier _ -> SyntaxKind.SimpleTypeSpecifier
      | TypeConstraint _ -> SyntaxKind.TypeConstraint
      | TypeParameter _ -> SyntaxKind.TypeParameter
      | NullableTypeSpecifier _ -> SyntaxKind.NullableTypeSpecifier
      | SoftTypeSpecifier _ -> SyntaxKind.SoftTypeSpecifier
      | GenericTypeSpecifier _ -> SyntaxKind.GenericTypeSpecifier
      | TypeArguments _ -> SyntaxKind.TypeArguments
      | TypeParameters _ -> SyntaxKind.TypeParameters
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
    let is_scope_resolution_expression node =
      kind node = SyntaxKind.ScopeResolutionExpression
    let is_member_selection_expression node =
      kind node = SyntaxKind.MemberSelectionExpression
    let is_safe_member_selection_expression node =
      kind node = SyntaxKind.SafeMemberSelectionExpression
    let is_yield_expression node = kind node = SyntaxKind.YieldExpression
    let is_print_expression node = kind node = SyntaxKind.PrintExpression
    let is_cast_expression node = kind node = SyntaxKind.CastExpression
    let is_lambda_expression node = kind node = SyntaxKind.LambdaExpression
    let is_lambda_signature node = kind node = SyntaxKind.LambdaSignature
    let is_anonymous_function node = kind node = SyntaxKind.AnonymousFunction
    let is_anonymous_function_use_clause node =
      kind node = SyntaxKind.AnonymousFunctionUseClause
    let is_literal node = kind node = SyntaxKind.LiteralExpression
    let is_variable node = kind node = SyntaxKind.VariableExpression
    let is_qualified_name node = kind node = SyntaxKind.QualifiedNameExpression
    let is_pipe_variable node = kind node = SyntaxKind.PipeVariableExpression
    let is_awaitable_creation node =
      kind node = SyntaxKind.AwaitableCreationExpression
    let is_error node = kind node = SyntaxKind.Error
    let is_list node = kind node = SyntaxKind.SyntaxList
    let is_list_item node = kind node = SyntaxKind.ListItem
    let is_header node = kind node = SyntaxKind.ScriptHeader
    let is_script node = kind node = SyntaxKind.Script
    let is_enum node = kind node = SyntaxKind.EnumDeclaration
    let is_enumerator node = kind node = SyntaxKind.Enumerator
    let is_alias node = kind node = SyntaxKind.AliasDeclaration
    let is_property_declaration node =
      kind node = SyntaxKind.PropertyDeclaration
    let is_property_declarator node =
      kind node = SyntaxKind.PropertyDeclarator
    let is_namespace node = kind node = SyntaxKind.NamespaceDeclaration
    let is_namespace_body node = kind node = SyntaxKind.NamespaceBody
    let is_namespace_group_use node =
      kind node = SyntaxKind.NamespaceGroupUseDeclaration
    let is_namespace_use node = kind node = SyntaxKind.NamespaceUseDeclaration
    let is_namespace_use_clause node = kind node = SyntaxKind.NamespaceUseClause
    let is_function node = kind node = SyntaxKind.FunctionDeclaration
    let is_method node = kind node = SyntaxKind.MethodishDeclaration
    let is_classish node = kind node = SyntaxKind.ClassishDeclaration
    let is_classish_body node = kind node = SyntaxKind.ClassishBody
    let is_trait_use node = kind node = SyntaxKind.TraitUse
    let is_require_clause node = kind node = SyntaxKind.RequireClause
    let is_const_declaration node = kind node = SyntaxKind.ConstDeclaration
    let is_constant_declarator node = kind node = SyntaxKind.ConstantDeclarator
    let is_type_const_declaration node =
      kind node = SyntaxKind.TypeConstDeclaration
    let is_decorated_expression node =
      kind node = SyntaxKind.DecoratedExpression
    let is_parameter node = kind node = SyntaxKind.ParameterDeclaration
    let is_attribute_specification node =
      kind node = SyntaxKind.AttributeSpecification
    let is_attribute node = kind node = SyntaxKind.Attribute
    let is_inclusion_directive node = kind node = SyntaxKind.InclusionDirective
    let is_inclusion_expression node =
      kind node = SyntaxKind.InclusionExpression
    let is_compound_statement node = kind node = SyntaxKind.CompoundStatement
    let is_expression_statement node =
      kind node = SyntaxKind.ExpressionStatement
    let is_for_statement node = kind node = SyntaxKind.ForStatement
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
    let is_simple_initializer node = kind node = SyntaxKind.SimpleInitializer
    let is_static_declarator node = kind node = SyntaxKind.StaticDeclarator
    let is_echo_statement node =
      kind node = SyntaxKind.EchoStatement
    let is_switch_statement node = kind node = SyntaxKind.SwitchStatement
    let is_prefix_operator node = kind node = SyntaxKind.PrefixUnaryOperator
    let is_postfix_operator node = kind node = SyntaxKind.PostfixUnaryOperator
    let is_binary_operator node = kind node = SyntaxKind.BinaryOperator
    let is_instanceof_operator node =
      kind node = SyntaxKind.InstanceofExpression
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
    let is_collection_literal_expression node =
      kind node = SyntaxKind.CollectionLiteralExpression
    let is_object_creation_expression node =
      kind node = SyntaxKind.ObjectCreationExpression
    let is_shape_expression node = kind node = SyntaxKind.ShapeExpression
    let is_field_initializer node = kind node = SyntaxKind.FieldInitializer
    let is_array_creation_expression node =
      kind node = SyntaxKind.ArrayCreationExpression
    let is_array_intrinsic_expression node =
      kind node = SyntaxKind.ArrayIntrinsicExpression
    let is_element_initializer node = kind node = SyntaxKind.ElementInitializer
    let is_subscript_expression node =
      kind node = SyntaxKind.SubscriptExpression
    let is_xhp_category_declaration node =
      kind node = SyntaxKind.XHPCategoryDeclaration
    let is_xhp_children_declaration node =
      kind node = SyntaxKind.XHPChildrenDeclaration
    let is_xhp_enum_type node = kind node = SyntaxKind.XHPEnumType
    let is_xhp_required node = kind node = SyntaxKind.XHPRequired
    let is_xhp_expression node = kind node = SyntaxKind.XHPExpression
    let is_xhp_open node = kind node = SyntaxKind.XHPOpen
    let is_xhp_attribute node = kind node = SyntaxKind.XHPAttribute
    let is_type_constant node = kind node = SyntaxKind.TypeConstant
    let is_simple_type node = kind node = SyntaxKind.SimpleTypeSpecifier
    let is_generic_type node = kind node = SyntaxKind.GenericTypeSpecifier
    let is_nullable_type_specifier node =
      kind node = SyntaxKind.NullableTypeSpecifier
    let is_soft_type_specifier node =
      kind node = SyntaxKind.SoftTypeSpecifier
    let is_type_arguments node = kind node = SyntaxKind.TypeArguments
    let is_type_parameters node = kind node = SyntaxKind.TypeParameters
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

    let is_loop_statement node =
      is_for_statement node ||
      is_foreach_statement node ||
      is_while_statement node ||
      is_do_statement node

    let is_separable_prefix node =
      match syntax node with
      | Token t -> begin
        Full_fidelity_token_kind.(match Token.kind t with
        | PlusPlus | MinusMinus -> false
        | _ -> true) end
      | _ -> true

    let is_specific_token kind node =
      match syntax node with
      | Token t -> Token.kind t = kind
      | _ -> false


    let is_semicolon = is_specific_token Full_fidelity_token_kind.Semicolon
    let is_name = is_specific_token Full_fidelity_token_kind.Name
    let is_construct = is_specific_token Full_fidelity_token_kind.Construct
    let is_destruct = is_specific_token Full_fidelity_token_kind.Destruct
    let is_static = is_specific_token Full_fidelity_token_kind.Static
    let is_private = is_specific_token Full_fidelity_token_kind.Private
    let is_public = is_specific_token Full_fidelity_token_kind.Public
    let is_protected = is_specific_token Full_fidelity_token_kind.Protected
    let is_abstract = is_specific_token Full_fidelity_token_kind.Abstract
    let is_final = is_specific_token Full_fidelity_token_kind.Final
    let is_void = is_specific_token Full_fidelity_token_kind.Void
    let is_left_brace = is_specific_token Full_fidelity_token_kind.LeftBrace
    let is_ellipsis = is_specific_token Full_fidelity_token_kind.DotDotDot
    let is_comma = is_specific_token Full_fidelity_token_kind.Comma
    let is_array = is_specific_token Full_fidelity_token_kind.Array

    let children node =
      match node.syntax with
      | Missing -> []
      | Token _ -> []
      | LiteralExpression x -> [x]
      | VariableExpression x -> [x]
      | QualifiedNameExpression x -> [x]
      | PipeVariableExpression x -> [x]
      | Error x -> x
      | SyntaxList x -> x
      | ScopeResolutionExpression
        { scope_resolution_qualifier; scope_resolution_operator;
          scope_resolution_name } ->
        [ scope_resolution_qualifier; scope_resolution_operator;
          scope_resolution_name ]
      | MemberSelectionExpression
        { member_object; member_operator; member_name } ->
        [ member_object; member_operator; member_name ]
      | SafeMemberSelectionExpression
        { member_object; member_operator; member_name } ->
        [ member_object; member_operator; member_name ]
      | YieldExpression
        { yield_token; yield_operand } ->
        [ yield_token; yield_operand ]
      | PrintExpression
        { print_token; print_expr; } ->
        [ print_token; print_expr; ]
      | CastExpression
        { cast_left_paren; cast_type; cast_right_paren; cast_operand } ->
        [ cast_left_paren; cast_type; cast_right_paren; cast_operand ]
      | LambdaExpression
        { lambda_async; lambda_signature; lambda_arrow; lambda_body } ->
        [ lambda_async; lambda_signature; lambda_arrow; lambda_body ]
      | LambdaSignature
        { lambda_left_paren; lambda_params; lambda_right_paren;
          lambda_colon; lambda_type } ->
        [ lambda_left_paren; lambda_params; lambda_right_paren;
          lambda_colon; lambda_type ]
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
        { enumerator_name; enumerator_equal; enumerator_value;
          enumerator_semicolon } ->
        [ enumerator_name; enumerator_equal; enumerator_value;
          enumerator_semicolon ]
      | AliasDeclaration
        { alias_attribute_spec; alias_token; alias_name;
          alias_generic_parameter; alias_constraint; alias_equal; alias_type;
          alias_semicolon } ->
        [ alias_attribute_spec; alias_token; alias_name;
          alias_generic_parameter; alias_constraint; alias_equal; alias_type;
          alias_semicolon ]
      | PropertyDeclaration
        { prop_modifiers; prop_type; prop_declarators; prop_semicolon } ->
        [ prop_modifiers; prop_type; prop_declarators; prop_semicolon ]
      | PropertyDeclarator
        { prop_name; prop_init } ->
        [ prop_name; prop_init ]
      | NamespaceDeclaration
        { namespace_token; namespace_name; namespace_body } ->
        [ namespace_token; namespace_name; namespace_body ]
      | NamespaceBody
        { namespace_left_brace; namespace_declarations;
          namespace_right_brace } ->
        [ namespace_left_brace; namespace_declarations;
          namespace_right_brace ]
      | NamespaceGroupUseDeclaration
        { namespace_group_use; namespace_group_use_kind;
          namespace_group_use_prefix; namespace_group_use_left_brace;
          namespace_group_use_clauses; namespace_group_use_right_brace;
          namespace_group_use_semicolon } ->
        [ namespace_group_use; namespace_group_use_kind;
          namespace_group_use_prefix; namespace_group_use_left_brace;
          namespace_group_use_clauses; namespace_group_use_right_brace;
          namespace_group_use_semicolon ]
      | NamespaceUseDeclaration
        { namespace_use; namespace_use_kind; namespace_use_clauses;
          namespace_use_semicolon } ->
        [ namespace_use; namespace_use_kind; namespace_use_clauses;
          namespace_use_semicolon ]
      | NamespaceUseClause
        { namespace_use_clause_kind; namespace_use_name;
          namespace_use_as; namespace_use_alias } ->
        [ namespace_use_clause_kind; namespace_use_name;
          namespace_use_as; namespace_use_alias ]
      | FunctionDeclaration
        { function_attribute_spec; function_declaration_header; function_body}
        ->
        [ function_attribute_spec; function_declaration_header; function_body]
      | FunctionDeclarationHeader
        { function_async; function_token; function_name;
          function_type_params; function_left_paren; function_params;
          function_right_paren; function_colon; function_type } ->
        [ function_async; function_token; function_name;
          function_type_params; function_left_paren; function_params;
          function_right_paren; function_colon; function_type ]
      | MethodishDeclaration
        { methodish_attr; methodish_modifiers; methodish_function_decl_header;
          methodish_function_body; methodish_semicolon } ->
        [ methodish_attr; methodish_modifiers; methodish_function_decl_header;
          methodish_function_body; methodish_semicolon ]
      | ClassishDeclaration
        { classish_attr; classish_modifiers; classish_token;
          classish_name; classish_type_params; classish_extends;
          classish_extends_list; classish_implements; classish_implements_list;
          classish_body } ->
        [ classish_attr; classish_modifiers; classish_token;
          classish_name; classish_type_params; classish_extends;
          classish_extends_list; classish_implements; classish_implements_list;
          classish_body ]
      | ClassishBody
        { classish_body_left_brace; classish_body_elements;
          classish_body_right_brace } ->
        [ classish_body_left_brace; classish_body_elements;
          classish_body_right_brace ]
      | XHPChildrenDeclaration
        { xhp_children; xhp_children_expression; xhp_children_semicolon } ->
        [ xhp_children; xhp_children_expression; xhp_children_semicolon ]
      | XHPCategoryDeclaration
        { xhp_category; xhp_category_list; xhp_category_semicolon } ->
        [ xhp_category; xhp_category_list; xhp_category_semicolon ]
      | XHPEnumType
        { xhp_enum_token; xhp_enum_left_brace; xhp_enum_values;
          xhp_enum_right_brace } ->
        [ xhp_enum_token; xhp_enum_left_brace; xhp_enum_values;
          xhp_enum_right_brace ]
      | XHPRequired
        { xhp_required_at; xhp_required } ->
        [ xhp_required_at; xhp_required ]
      | XHPClassAttributeDeclaration
        { xhp_attr_token; xhp_attr_list; xhp_attr_semicolon } ->
        [ xhp_attr_token; xhp_attr_list; xhp_attr_semicolon ]
      | XHPClassAttribute
        { xhp_attr_decl_type; xhp_attr_decl_name; xhp_attr_decl_init;
          xhp_attr_decl_required } ->
        [ xhp_attr_decl_type; xhp_attr_decl_name; xhp_attr_decl_init;
          xhp_attr_decl_required ]
      | TraitUse
        { trait_use_token; trait_use_name_list; trait_use_semicolon; } ->
        [ trait_use_token; trait_use_name_list; trait_use_semicolon; ]
      | RequireClause
        { require_token; require_kind; require_name; require_semicolon } ->
        [ require_token; require_kind; require_name; require_semicolon ]
      | ConstDeclaration
        { const_abstract; const_token; const_type_specifier;
          const_declarator_list; const_semicolon; } ->
        [ const_abstract; const_token; const_type_specifier;
          const_declarator_list; const_semicolon; ]
      | ConstantDeclarator
        { constant_declarator_name; constant_declarator_initializer; } ->
        [ constant_declarator_name; constant_declarator_initializer; ]
      | TypeConstDeclaration
        { type_const_abstract; type_const_const_token; type_const_type_token;
          type_const_name; type_const_type_constraint; type_const_equal;
          type_const_type_specifier; type_const_semicolon; } ->
        [ type_const_abstract; type_const_const_token; type_const_type_token;
          type_const_name; type_const_type_constraint; type_const_equal;
          type_const_type_specifier; type_const_semicolon; ]
      | DecoratedExpression
        { decorated_expression_decorator; decorated_expression_expression; } ->
        [ decorated_expression_decorator; decorated_expression_expression; ]
      | ParameterDeclaration
        { param_attr; param_visibility; param_type; param_name; param_default }
        ->
        [ param_attr; param_visibility; param_type; param_name; param_default ]
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
      | InclusionExpression
        { inclusion_require; inclusion_filename } ->
        [ inclusion_require; inclusion_filename ]
      | InclusionDirective
        { inclusion_expression; inclusion_semicolon } ->
        [ inclusion_expression; inclusion_semicolon ]
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
      | CatchClause
        { catch_keyword; catch_left_paren; catch_type; catch_variable;
          catch_right_paren; catch_compound_statement} ->
        [ catch_keyword; catch_left_paren; catch_type; catch_variable;
          catch_right_paren; catch_compound_statement]
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
        { break_keyword; break_level; break_semicolon } ->
        [ break_keyword; break_level; break_semicolon ]
      | ContinueStatement
        { continue_keyword; continue_level; continue_semicolon } ->
        [ continue_keyword; continue_level; continue_semicolon ]
      | FunctionStaticStatement
        { static_static; static_declarations; static_semicolon } ->
        [ static_static; static_declarations; static_semicolon ]
      | SimpleInitializer
        { simple_init_equal; simple_init_value } ->
        [ simple_init_equal; simple_init_value ]
      | StaticDeclarator
        { static_name; static_init } ->
        [ static_name; static_init ]
      | EchoStatement
        { echo_token; echo_expression_list; echo_semicolon; } ->
        [ echo_token; echo_expression_list; echo_semicolon; ]
      | PrefixUnaryOperator
        { unary_operator; unary_operand } ->
        [ unary_operator; unary_operand ]
      | PostfixUnaryOperator
        { unary_operand; unary_operator } ->
        [ unary_operand; unary_operator ]
      | BinaryOperator
        { binary_left_operand; binary_operator; binary_right_operand } ->
        [ binary_left_operand; binary_operator; binary_right_operand ]
      | InstanceofExpression
        { instanceof_left_operand; instanceof_operator;
          instanceof_right_operand } ->
        [ instanceof_left_operand; instanceof_operator;
          instanceof_right_operand ]
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
      | CollectionLiteralExpression
        { collection_literal_name; collection_literal_left_brace;
          collection_literal_initialization_list;
          collection_literal_right_brace; } ->
        [ collection_literal_name; collection_literal_left_brace;
          collection_literal_initialization_list;
          collection_literal_right_brace; ]
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
      | ElementInitializer
        { element_key; element_arrow; element_value } ->
        [ element_key; element_arrow; element_value ]
      | SubscriptExpression
        { subscript_receiver; subscript_left;
          subscript_index; subscript_right } ->
        [ subscript_receiver; subscript_left;
          subscript_index; subscript_right ]
      | AwaitableCreationExpression
        { awaitable_async; awaitable_compound_statement; } ->
        [ awaitable_async; awaitable_compound_statement; ]
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
      | SoftTypeSpecifier
        { soft_at; soft_type } ->
        [ soft_at; soft_type ]
      | GenericTypeSpecifier
        { generic_class_type; generic_arguments } ->
        [ generic_class_type; generic_arguments ]
      | TypeArguments
        { type_arguments_left_angle; type_arguments;
          type_arguments_right_angle } ->
        [ type_arguments_left_angle; type_arguments;
          type_arguments_right_angle ]
      | TypeParameters
        { type_parameters_left_angle; type_parameters;
          type_parameters_right_angle } ->
        [ type_parameters_left_angle; type_parameters;
          type_parameters_right_angle ]
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
      | PipeVariableExpression _ -> ["pipe_variable_expression"]
      | Error _ -> []
      | SyntaxList _ -> []
      | MemberSelectionExpression
        { member_object; member_operator; member_name } ->
        [ "member_object"; "member_operator"; "member_name" ]
      | SafeMemberSelectionExpression
        { member_object; member_operator; member_name } ->
        [ "member_object"; "member_operator"; "member_name" ]
      | ScopeResolutionExpression
        { scope_resolution_qualifier; scope_resolution_operator;
          scope_resolution_name } ->
        [ "scope_resolution_qualifier"; "scope_resolution_operator";
          "scope_resolution_name" ]
      | YieldExpression
        { yield_token; yield_operand } ->
        [ "yield_token"; "yield_operand" ]
      | PrintExpression
        { print_token; print_expr; } ->
        [ "print_token"; "print_expr"; ]
      | CastExpression
        { cast_left_paren; cast_type; cast_right_paren; cast_operand } ->
        [ "cast_left_paren"; "cast_type"; "cast_right_paren"; "cast_operand" ]
      | LambdaExpression
        { lambda_async; lambda_signature; lambda_arrow; lambda_body } ->
        [ "lambda_async"; "lambda_signature"; "lambda_arrow"; "lambda_body" ]
      | LambdaSignature
        { lambda_left_paren; lambda_params; lambda_right_paren;
          lambda_colon; lambda_type } ->
        [ "lambda_left_paren"; "lambda_params"; "lambda_right_paren";
          "lambda_colon"; "lambda_type" ]
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
        { enumerator_name; enumerator_equal; enumerator_value;
          enumerator_semicolon } ->
        [ "enumerator_name"; "enumerator_equal"; "enumerator_value";
          "enumerator_semicolon" ]
      | AliasDeclaration
        { alias_attribute_spec; alias_token; alias_name;
          alias_generic_parameter; alias_constraint; alias_equal; alias_type;
          alias_semicolon } ->
        [ "alias_attribute_spec"; "alias_token"; "alias_name";
          "alias_generic_parameter"; "alias_constraint"; "alias_equal";
          "alias_type"; "alias_semicolon" ]
      | PropertyDeclaration
        { prop_modifiers; prop_type; prop_declarators; prop_semicolon } ->
        [ "prop_modifiers"; "prop_type"; "prop_declarators"; "prop_semicolon" ]
      | PropertyDeclarator
        { prop_name; prop_init } ->
        [ "prop_name"; "prop_init" ]
      | NamespaceDeclaration
        { namespace_token; namespace_name; namespace_body } ->
        [ "namespace_token"; "namespace_name"; "namespace_body" ]
      | NamespaceBody
        { namespace_left_brace; namespace_declarations;
          namespace_right_brace } ->
        [ "namespace_left_brace"; "namespace_declarations";
          "namespace_right_brace" ]
      | NamespaceGroupUseDeclaration
        { namespace_group_use; namespace_group_use_kind;
          namespace_group_use_prefix; namespace_group_use_left_brace;
          namespace_group_use_clauses; namespace_group_use_right_brace;
          namespace_group_use_semicolon } ->
        [ "namespace_group_use"; "namespace_group_use_kind";
          "namespace_group_use_prefix"; "namespace_group_use_left_brace";
          "namespace_group_use_clauses"; "namespace_group_use_right_brace";
          "namespace_group_use_semicolon" ]
      | NamespaceUseDeclaration
        { namespace_use; namespace_use_kind; namespace_use_clauses;
          namespace_use_semicolon } ->
        [ "namespace_use"; "namespace_use_kind"; "namespace_use_clauses";
          "namespace_use_semicolon" ]
      | NamespaceUseClause
        { namespace_use_clause_kind; namespace_use_name;
          namespace_use_as; namespace_use_alias } ->
        [ "namespace_use_clause_kind"; "namespace_use_name";
          "namespace_use_as"; "namespace_use_alias" ]
      | FunctionDeclaration
        { function_attribute_spec; function_declaration_header; function_body }
        -> [ "function_attribute_spec"; "function_declaration_header";
        "function_body" ]
      | FunctionDeclarationHeader
        { function_async; function_token; function_name;
          function_type_params; function_left_paren; function_params;
          function_right_paren; function_colon; function_type; } ->
        [ "function_async"; "function_token"; "function_name";
          "function_type_params"; "function_left_paren"; "function_params";
          "function_right_paren"; "function_colon"; "function_type"; ]
      | MethodishDeclaration
        { methodish_attr; methodish_modifiers; methodish_function_decl_header;
          methodish_function_body; methodish_semicolon } ->
        [ "methodish_attr"; "methodish_modifiers";
          "methodish_function_decl_header"; "methodish_function_body";
          "methodish_semicolon" ]
      | ClassishDeclaration
        { classish_attr; classish_modifiers; classish_token;
          classish_name; classish_type_params; classish_extends;
          classish_extends_list; classish_implements; classish_implements_list;
          classish_body } ->
        [ "classish_attr"; "classish_modifiers";
          "classish_token"; "classish_name"; "classish_type_params";
          "classish_extends"; "classish_extends_list"; "classish_implements";
          "classish_implements_list"; "classish_body" ]
      | ClassishBody
        { classish_body_left_brace; classish_body_elements;
          classish_body_right_brace } ->
        [ "classish_body_left_brace"; "classish_body_elements";
          "classish_body_right_brace" ]
      | XHPChildrenDeclaration
        { xhp_children; xhp_children_expression; xhp_children_semicolon } ->
        [ "xhp_children"; "xhp_children_expression"; "xhp_children_semicolon" ]
      | XHPCategoryDeclaration
        { xhp_category; xhp_category_list; xhp_category_semicolon } ->
        [ "xhp_category"; "xhp_category_list"; "xhp_category_semicolon" ]
      | XHPEnumType
        { xhp_enum_token; xhp_enum_left_brace; xhp_enum_values;
          xhp_enum_right_brace } ->
        [ "xhp_enum_token"; "xhp_enum_left_brace"; "xhp_enum_values";
          "xhp_enum_right_brace" ]
      | XHPRequired
        { xhp_required_at; xhp_required } ->
        [ "xhp_required_at"; "xhp_required" ]
      | XHPClassAttributeDeclaration
        { xhp_attr_token; xhp_attr_list; xhp_attr_semicolon } ->
        [ "xhp_attr_token"; "xhp_attr_list"; "xhp_attr_semicolon" ]
      | XHPClassAttribute
        { xhp_attr_decl_type; xhp_attr_decl_name; xhp_attr_decl_init;
          xhp_attr_decl_required } ->
        [ "xhp_attr_decl_type"; "xhp_attr_decl_name"; "xhp_attr_decl_init";
          "xhp_attr_decl_required" ]
      | TraitUse
        { trait_use_token; trait_use_name_list; trait_use_semicolon; } ->
        [ "trait_use_token"; "trait_use_name_list"; "trait_use_semicolon"; ]
      | RequireClause
        { require_token; require_kind; require_name; require_semicolon } ->
        [ "require_token"; "require_kind"; "require_name"; "require_semicolon" ]
      | ConstDeclaration
        { const_abstract; const_token; const_type_specifier;
          const_declarator_list; const_semicolon; } ->
        [ "const_abstract"; "const_token"; "const_type_specifier";
          "const_declarator_list"; "const_semicolon"; ]
      | ConstantDeclarator
        { constant_declarator_name; constant_declarator_initializer; } ->
        [ "constant_declarator_name"; "constant_declarator_initializer"; ]
      | TypeConstDeclaration
        { type_const_abstract; type_const_const_token; type_const_type_token;
          type_const_name; type_const_type_constraint; type_const_equal;
          type_const_type_specifier; type_const_semicolon; } ->
        [ "type_const_abstract"; "type_const_const_token";
          "type_const_type_token"; "type_const_name";
          "type_const_type_constraint"; "type_const_equal";
          "type_const_type_specifier"; "type_const_semicolon"; ]
      | DecoratedExpression
        { decorated_expression_decorator; decorated_expression_expression; } ->
        [ "decorated_expression_decorator"; "decorated_expression_expression"; ]
      | ParameterDeclaration
        { param_attr; param_visibility; param_type; param_name; param_default }
        ->
        [ "param_attr"; "param_visibility"; "param_type"; "param_name";
          "param_default" ]
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
      | InclusionExpression
        { inclusion_require; inclusion_filename } ->
        [ "inclusion_require"; "inclusion_filename" ]
      | InclusionDirective
        { inclusion_expression; inclusion_semicolon } ->
        [ "inclusion_expression"; "inclusion_semicolon" ]
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
      | CatchClause
        { catch_keyword; catch_left_paren; catch_type; catch_variable;
          catch_right_paren; catch_compound_statement} ->
        [ "catch_keyword"; "catch_left_paren"; "catch_type"; "catch_variable";
          "catch_right_paren"; "catch_compound_statement" ]
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
        { break_keyword; break_level; break_semicolon } ->
        [ "break_keyword"; "break_level"; "break_semicolon" ]
      | ContinueStatement
        { continue_keyword; continue_level; continue_semicolon } ->
        [ "continue_keyword"; "continue_level"; "continue_semicolon" ]
      | FunctionStaticStatement
        { static_static; static_declarations; static_semicolon } ->
        [ "static_static"; "static_declarations"; "static_semicolon" ]
      | SimpleInitializer
        { simple_init_equal; simple_init_value } ->
        [ "simple_init_equal"; "simple_init_value" ]
      | StaticDeclarator
        { static_name; static_init } ->
        [ "static_name"; "static_init" ]
      | EchoStatement
        { echo_token; echo_expression_list; echo_semicolon; } ->
        [ "echo_token"; "echo_expression_list"; "echo_semicolon"; ]
      | PrefixUnaryOperator
        { unary_operator; unary_operand } ->
        [ "unary_operator"; "unary_operand" ]
      | PostfixUnaryOperator
        { unary_operand; unary_operator } ->
        [ "unary_operand"; "unary_operator" ]
      | BinaryOperator
        { binary_left_operand; binary_operator; binary_right_operand } ->
        [ "binary_left_operand"; "binary_operator"; "binary_right_operand" ]
      | InstanceofExpression
        { instanceof_left_operand; instanceof_operator;
          instanceof_right_operand } ->
        [ "instanceof_left_operand"; "instanceof_operator";
          "instanceof_right_operand" ]
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
      | CollectionLiteralExpression
        { collection_literal_name; collection_literal_left_brace;
          collection_literal_initialization_list;
          collection_literal_right_brace; } ->
        [ "collection_literal_name"; "collection_literal_left_brace";
          "collection_literal_initialization_list";
          "collection_literal_right_brace"; ]
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
     | ElementInitializer
       { element_key; element_arrow; element_value } ->
       [ "element_key"; "element_arrow"; "element_value" ]
      | SubscriptExpression
        { subscript_receiver; subscript_left;
          subscript_index; subscript_right } ->
        [ "subscript_receiver"; "subscript_left";
          "subscript_index"; "subscript_right" ]
      | AwaitableCreationExpression
        { awaitable_async; awaitable_compound_statement; } ->
        [ "awaitable_async"; "awaitable_compound_statement"; ]
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
      | SoftTypeSpecifier
        { soft_at; soft_type } ->
        [ "soft_at"; "soft_type" ]
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
      | TypeParameters
        { type_parameters_left_angle; type_parameters;
          type_parameters_right_angle } ->
        [ "type_parameters_left_angle"; "type_parameters";
          "type_parameters_right_angle" ]
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
      | (SyntaxKind.MemberSelectionExpression,
        [ member_object; member_operator; member_name ]) ->
        MemberSelectionExpression
        { member_object; member_operator; member_name }
      | (SyntaxKind.SafeMemberSelectionExpression,
        [ member_object; member_operator; member_name ]) ->
        SafeMemberSelectionExpression
        { member_object; member_operator; member_name }
      | (SyntaxKind.ScopeResolutionExpression,
        [ scope_resolution_qualifier; scope_resolution_operator;
          scope_resolution_name ]) ->
        ScopeResolutionExpression
        { scope_resolution_qualifier; scope_resolution_operator;
          scope_resolution_name }
      | (SyntaxKind.YieldExpression,
        [ yield_token; yield_operand ]) ->
        YieldExpression
        { yield_token; yield_operand }
      | (SyntaxKind.PrintExpression,
        [ print_token; print_expr; ]) ->
        PrintExpression { print_token; print_expr; }
      | (SyntaxKind.CastExpression,
        [ cast_left_paren; cast_type; cast_right_paren; cast_operand ]) ->
        CastExpression
        { cast_left_paren; cast_type; cast_right_paren; cast_operand }
      | (SyntaxKind.LambdaExpression,
        [ lambda_async; lambda_signature; lambda_arrow; lambda_body ]) ->
        LambdaExpression
        { lambda_async; lambda_signature; lambda_arrow; lambda_body }
      | (SyntaxKind.LambdaSignature,
        [ lambda_left_paren; lambda_params; lambda_right_paren;
          lambda_colon; lambda_type ]) ->
        LambdaSignature
        { lambda_left_paren; lambda_params; lambda_right_paren;
          lambda_colon; lambda_type }
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
      | (SyntaxKind.PipeVariableExpression, [x]) -> PipeVariableExpression x
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
        [ enumerator_name; enumerator_equal; enumerator_value;
          enumerator_semicolon ]) ->
        Enumerator
        { enumerator_name; enumerator_equal; enumerator_value;
          enumerator_semicolon }
      | (SyntaxKind.AliasDeclaration,
        [ alias_attribute_spec; alias_token; alias_name;
          alias_generic_parameter; alias_constraint; alias_equal; alias_type;
          alias_semicolon ]) ->
        AliasDeclaration
        { alias_attribute_spec; alias_token; alias_name;
          alias_generic_parameter; alias_constraint; alias_equal; alias_type;
          alias_semicolon }
      | (SyntaxKind.PropertyDeclaration,
        [ prop_modifiers; prop_type; prop_declarators; prop_semicolon ]) ->
        PropertyDeclaration
        { prop_modifiers; prop_type; prop_declarators; prop_semicolon }
      | (SyntaxKind.PropertyDeclarator,
        [prop_name; prop_init ]) ->
        PropertyDeclarator
        { prop_name; prop_init }
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
      | (SyntaxKind.NamespaceGroupUseDeclaration,
        [ namespace_group_use; namespace_group_use_kind;
          namespace_group_use_prefix; namespace_group_use_left_brace;
          namespace_group_use_clauses; namespace_group_use_right_brace;
          namespace_group_use_semicolon ]) ->
        NamespaceGroupUseDeclaration
        { namespace_group_use; namespace_group_use_kind;
          namespace_group_use_prefix; namespace_group_use_left_brace;
          namespace_group_use_clauses; namespace_group_use_right_brace;
          namespace_group_use_semicolon }
      | (SyntaxKind.NamespaceUseDeclaration,
        [ namespace_use; namespace_use_kind; namespace_use_clauses;
          namespace_use_semicolon ]) ->
        NamespaceUseDeclaration
        { namespace_use; namespace_use_kind; namespace_use_clauses;
          namespace_use_semicolon }
      | (SyntaxKind.NamespaceUseClause,
        [ namespace_use_clause_kind; namespace_use_name;
          namespace_use_as; namespace_use_alias ]) ->
        NamespaceUseClause
        { namespace_use_clause_kind; namespace_use_name;
          namespace_use_as; namespace_use_alias }
      | (SyntaxKind.FunctionDeclaration,
        [ function_attribute_spec; function_declaration_header; function_body ])
        -> FunctionDeclaration
        { function_attribute_spec; function_declaration_header; function_body }
      | (SyntaxKind.FunctionDeclarationHeader, [ function_async;
        function_token; function_name; function_type_params;
        function_left_paren; function_params; function_right_paren;
        function_colon; function_type ]) ->
            FunctionDeclarationHeader { function_async;
              function_token; function_name; function_type_params;
              function_left_paren; function_params; function_right_paren;
              function_colon; function_type }
      | (SyntaxKind.MethodishDeclaration,
        [ methodish_attr; methodish_modifiers; methodish_function_decl_header;
          methodish_function_body; methodish_semicolon ]) ->
        MethodishDeclaration { methodish_attr; methodish_modifiers;
          methodish_function_decl_header; methodish_function_body;
          methodish_semicolon }
      | (SyntaxKind.ClassishDeclaration,
        [ classish_attr; classish_modifiers; classish_token;
          classish_name; classish_type_params; classish_extends;
          classish_extends_list; classish_implements; classish_implements_list;
          classish_body ]) ->
        ClassishDeclaration {
          classish_attr; classish_modifiers; classish_token;
          classish_name; classish_type_params; classish_extends;
          classish_extends_list; classish_implements; classish_implements_list;
          classish_body }
      | (SyntaxKind.ClassishBody,
        [ classish_body_left_brace; classish_body_elements;
          classish_body_right_brace ]) ->
        ClassishBody {
          classish_body_left_brace; classish_body_elements;
          classish_body_right_brace }
      | (SyntaxKind.XHPChildrenDeclaration,
        [ xhp_children; xhp_children_expression; xhp_children_semicolon ]) ->
        XHPChildrenDeclaration
        { xhp_children; xhp_children_expression; xhp_children_semicolon }
      | (SyntaxKind.XHPCategoryDeclaration,
        [ xhp_category; xhp_category_list; xhp_category_semicolon ]) ->
        XHPCategoryDeclaration
        { xhp_category; xhp_category_list; xhp_category_semicolon }
      | (SyntaxKind.XHPEnumType,
        [ xhp_enum_token; xhp_enum_left_brace; xhp_enum_values;
          xhp_enum_right_brace ]) ->
        XHPEnumType
        { xhp_enum_token; xhp_enum_left_brace; xhp_enum_values;
          xhp_enum_right_brace }
      | (SyntaxKind.XHPRequired,
        [ xhp_required_at; xhp_required ]) ->
        XHPRequired
        { xhp_required_at; xhp_required }
      | (SyntaxKind.XHPClassAttributeDeclaration,
        [ xhp_attr_token; xhp_attr_list; xhp_attr_semicolon ]) ->
        XHPClassAttributeDeclaration
        { xhp_attr_token; xhp_attr_list; xhp_attr_semicolon }
      | (SyntaxKind.XHPClassAttribute,
        [ xhp_attr_decl_type; xhp_attr_decl_name; xhp_attr_decl_init;
          xhp_attr_decl_required ]) ->
        XHPClassAttribute
        { xhp_attr_decl_type; xhp_attr_decl_name; xhp_attr_decl_init;
          xhp_attr_decl_required }
      | (SyntaxKind.TraitUse,
        [ trait_use_token; trait_use_name_list; trait_use_semicolon; ]) ->
        TraitUse { trait_use_token; trait_use_name_list; trait_use_semicolon; }
      | (SyntaxKind.ConstDeclaration,
        [ const_abstract; const_token; const_type_specifier;
          const_declarator_list; const_semicolon; ]) ->
        ConstDeclaration { const_abstract; const_token; const_type_specifier;
          const_declarator_list; const_semicolon; }
      | (SyntaxKind.ConstantDeclarator,
        [ constant_declarator_name; constant_declarator_initializer; ]) ->
        ConstantDeclarator { constant_declarator_name;
          constant_declarator_initializer; }
      | (SyntaxKind.TypeConstDeclaration,
        [ type_const_abstract; type_const_const_token; type_const_type_token;
          type_const_name; type_const_type_constraint; type_const_equal;
          type_const_type_specifier; type_const_semicolon; ]) ->
        TypeConstDeclaration { type_const_abstract; type_const_const_token;
          type_const_type_token; type_const_name; type_const_type_constraint;
          type_const_equal; type_const_type_specifier; type_const_semicolon; }
      | (SyntaxKind.DecoratedExpression,
        [ decorated_expression_decorator; decorated_expression_expression; ]) ->
        DecoratedExpression
        { decorated_expression_decorator; decorated_expression_expression; }
      | (SyntaxKind.ParameterDeclaration, [ param_attr; param_visibility;
        param_type; param_name; param_default ]) ->
        ParameterDeclaration { param_attr; param_visibility; param_type;
          param_name; param_default }
      | (SyntaxKind.RequireClause,
        [ require_token; require_kind; require_name; require_semicolon ]) ->
        RequireClause
        { require_token; require_kind; require_name; require_semicolon }
      | SyntaxKind.AttributeSpecification, [ attribute_spec_left_double_angle;
        attribute_spec_attribute_list; attribute_spec_right_double_angle ] ->
        AttributeSpecification { attribute_spec_left_double_angle;
          attribute_spec_attribute_list; attribute_spec_right_double_angle }
      | SyntaxKind.Attribute, [ attribute_name; attribute_left_paren;
        attribute_values; attribute_right_paren ] ->
        Attribute { attribute_name; attribute_left_paren; attribute_values;
          attribute_right_paren }
      | (SyntaxKind.InclusionExpression,
        [ inclusion_require; inclusion_filename ]) ->
        InclusionExpression
        { inclusion_require; inclusion_filename }
      | (SyntaxKind.InclusionDirective,
        [ inclusion_expression; inclusion_semicolon ]) ->
        InclusionDirective
        { inclusion_expression; inclusion_semicolon }
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
      | SyntaxKind.CatchClause,
        [ catch_keyword; catch_left_paren; catch_type; catch_variable;
          catch_right_paren; catch_compound_statement ] ->
        CatchClause
        { catch_keyword; catch_left_paren; catch_type; catch_variable;
          catch_right_paren; catch_compound_statement }
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
      | (SyntaxKind.BreakStatement,
        [ break_keyword; break_level; break_semicolon ]) ->
        BreakStatement
        { break_keyword; break_level; break_semicolon }
      | (SyntaxKind.ContinueStatement,
        [ continue_keyword; continue_level; continue_semicolon ]) ->
        ContinueStatement
        { continue_keyword; continue_level; continue_semicolon }
      | (SyntaxKind.FunctionStaticStatement,
        [ static_static; static_declarations; static_semicolon ]) ->
        FunctionStaticStatement
        { static_static; static_declarations; static_semicolon }
      | (SyntaxKind.SimpleInitializer,
        [ simple_init_equal; simple_init_value ]) ->
        SimpleInitializer
        { simple_init_equal; simple_init_value }
      | (SyntaxKind.StaticDeclarator,
        [ static_name; static_init ]) ->
        StaticDeclarator
        { static_name; static_init }
      | (SyntaxKind.EchoStatement,
        [ echo_token; echo_expression_list; echo_semicolon; ]) ->
        EchoStatement
        { echo_token; echo_expression_list; echo_semicolon; }
      | (SyntaxKind.PrefixUnaryOperator, [ unary_operator; unary_operand ]) ->
        PrefixUnaryOperator { unary_operator; unary_operand }
      | (SyntaxKind.PostfixUnaryOperator, [ unary_operand; unary_operator ]) ->
        PostfixUnaryOperator { unary_operand; unary_operator }
      | (SyntaxKind.BinaryOperator, [ binary_left_operand; binary_operator;
        binary_right_operand ]) ->
        BinaryOperator { binary_left_operand; binary_operator;
        binary_right_operand }

      | (SyntaxKind.InstanceofExpression,
        [ instanceof_left_operand; instanceof_operator;
          instanceof_right_operand ]) ->
        InstanceofExpression
        { instanceof_left_operand; instanceof_operator;
          instanceof_right_operand }
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
      | (SyntaxKind.CollectionLiteralExpression,
        [ collection_literal_name; collection_literal_left_brace;
          collection_literal_initialization_list;
          collection_literal_right_brace; ]) ->
        CollectionLiteralExpression
        { collection_literal_name; collection_literal_left_brace;
          collection_literal_initialization_list;
          collection_literal_right_brace; }
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
      | (SyntaxKind.ElementInitializer,
        [ element_key; element_arrow; element_value ]) ->
        ElementInitializer
        { element_key; element_arrow; element_value }
      | SyntaxKind.ArrayIntrinsicExpression, [ array_intrinsic_keyword;
        array_intrinsic_left_paren; array_intrinsic_members;
        array_intrinsic_right_paren ] ->
        ArrayIntrinsicExpression { array_intrinsic_keyword;
          array_intrinsic_left_paren; array_intrinsic_members;
          array_intrinsic_right_paren }
      | SyntaxKind.SubscriptExpression,
        [ subscript_receiver; subscript_left;
          subscript_index; subscript_right ] ->
        SubscriptExpression
        { subscript_receiver; subscript_left;
          subscript_index; subscript_right }
      | SyntaxKind.AwaitableCreationExpression,
        [ awaitable_async; awaitable_compound_statement; ] ->
        AwaitableCreationExpression
        { awaitable_async; awaitable_compound_statement; }
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
      | (SyntaxKind.SoftTypeSpecifier,
          [ soft_at; soft_type ]) ->
        SoftTypeSpecifier
          { soft_at; soft_type }
      | (SyntaxKind.TypeArguments, [ type_arguments_left_angle;
          type_arguments; type_arguments_right_angle ]) ->
        TypeArguments { type_arguments_left_angle;
            type_arguments; type_arguments_right_angle }
      | (SyntaxKind.TypeParameters,
        [ type_parameters_left_angle; type_parameters;
          type_parameters_right_angle ]) ->
        TypeParameters
        { type_parameters_left_angle; type_parameters;
          type_parameters_right_angle }
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
          [ unary_operator; unary_operand ]

      let make_binary_operator
        binary_left_operand binary_operator binary_right_operand =
          from_children SyntaxKind.BinaryOperator
            [ binary_left_operand; binary_operator; binary_right_operand ]

      let make_instanceof_expression left op right =
        from_children SyntaxKind.InstanceofExpression [left; op; right ]

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

      let make_member_selection_expression ob op name =
        from_children SyntaxKind.MemberSelectionExpression [ ob; op; name ]

      let make_safe_member_selection_expression ob op name =
        from_children SyntaxKind.SafeMemberSelectionExpression [ ob; op; name ]

      let make_scope_resolution_expression qualifier op name =
        from_children SyntaxKind.ScopeResolutionExpression
          [ qualifier; op; name ]

      let make_yield_expression token operand =
        from_children SyntaxKind.YieldExpression [ token; operand ]

      let make_print_expression print_token print_expr =
        from_children SyntaxKind.PrintExpression
          [ print_token; print_expr; ]

      let make_cast_expression left cast_type right operand =
        from_children SyntaxKind.CastExpression
          [ left; cast_type; right; operand ]

      let make_lambda_expression async signature arrow body =
        from_children SyntaxKind.LambdaExpression
          [async; signature; arrow; body ]

      let make_lambda_signature left params right colon lambda_type =
        from_children SyntaxKind.LambdaSignature
        [ left; params; right; colon; lambda_type ]

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

      let make_collection_literal_expression
        collection_literal_name collection_literal_left_brace
        collection_literal_initialization_list collection_literal_right_brace =
        from_children SyntaxKind.CollectionLiteralExpression
          [ collection_literal_name; collection_literal_left_brace;
          collection_literal_initialization_list;
          collection_literal_right_brace; ]

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

      let make_element_initializer key arrow value =
        from_children SyntaxKind.ElementInitializer [ key; arrow; value ]

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

      let make_subscript_expression receiver left index right =
        from_children SyntaxKind.SubscriptExpression
        [ receiver; left; index; right ]

      let make_awaitable_creation_expression async compound_stmt =
        from_children SyntaxKind.AwaitableCreationExpression
          [ async; compound_stmt; ]

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

      let make_enumerator name equal value semicolon =
        from_children SyntaxKind.Enumerator [ name; equal; value; semicolon ]

      let make_alias attr token name generic constr equal ty semi =
        from_children SyntaxKind.AliasDeclaration
          [ attr; token; name; generic; constr; equal; ty; semi ]

      let make_property_declaration mods ty decls semi =
        from_children SyntaxKind.PropertyDeclaration
        [ mods; ty; decls; semi ]

      let make_property_declarator name init =
        from_children SyntaxKind.PropertyDeclarator
        [ name; init ]

      let make_namespace token name body =
        from_children SyntaxKind.NamespaceDeclaration
          [ token; name; body ]

      let make_namespace_body left decls right =
        from_children SyntaxKind.NamespaceBody
          [ left; decls; right ]

      let make_namespace_group_use token kind prefix left clauses right semi =
        from_children SyntaxKind.NamespaceGroupUseDeclaration
        [ token; kind; prefix; left; clauses; right; semi ]

      let make_namespace_use use use_kind clauses semi =
        from_children SyntaxKind.NamespaceUseDeclaration
          [ use; use_kind; clauses; semi ]

      let make_namespace_use_clause use_kind name as_token alias =
        from_children SyntaxKind.NamespaceUseClause
          [ use_kind; name; as_token; alias ]

      let make_function function_attribute_spec function_declaration_header
        function_body =
        from_children SyntaxKind.FunctionDeclaration
          [ function_attribute_spec; function_declaration_header;
          function_body ]

      let make_function_header function_async function_token
        function_name function_type_params function_left_paren function_params
        function_right_paren function_colon function_type =
      from_children SyntaxKind.FunctionDeclarationHeader [
        function_async; function_token; function_name;
        function_type_params; function_left_paren; function_params;
        function_right_paren; function_colon; function_type ]

      let make_methodish methodish_attr methodish_modifiers
        methodish_function_decl_header methodish_function_body
        methodish_semicolon =
        from_children SyntaxKind.MethodishDeclaration
          [ methodish_attr; methodish_modifiers; methodish_function_decl_header;
            methodish_function_body; methodish_semicolon ]

    let make_classish classish_attr classish_modifiers
      classish_token classish_name classish_type_params classish_extends
      classish_extends_list classish_implements classish_implements_list
      classish_body =
      from_children SyntaxKind.ClassishDeclaration [
        classish_attr; classish_modifiers; classish_token;
        classish_name; classish_type_params; classish_extends;
        classish_extends_list; classish_implements; classish_implements_list;
        classish_body ]

      let make_classish_body classish_body_left_brace classish_body_elements
        classish_body_right_brace =
        from_children SyntaxKind.ClassishBody [
          classish_body_left_brace; classish_body_elements;
          classish_body_right_brace ]

      let make_xhp_enum_type token left items right =
        from_children SyntaxKind.XHPEnumType
        [ token; left; items; right ]

      let make_xhp_category_declaration category items semi =
        from_children SyntaxKind.XHPCategoryDeclaration
        [ category; items; semi ]

      let make_xhp_children_declaration children expr semi =
        from_children SyntaxKind.XHPChildrenDeclaration
          [ children; expr; semi ]

      let make_xhp_required at req =
        from_children SyntaxKind.XHPRequired [ at; req ]

      let make_xhp_class_attribute_declaration attr attrs semi =
        from_children SyntaxKind.XHPClassAttributeDeclaration
          [ attr; attrs; semi ]

      let make_xhp_class_attribute attr_type name init required =
        from_children SyntaxKind.XHPClassAttribute
          [ attr_type; name; init; required ]

      let make_trait_use trait_use_token trait_use_name_list
        trait_use_semicolon =
        from_children SyntaxKind.TraitUse [
          trait_use_token; trait_use_name_list; trait_use_semicolon; ]

      let make_require_clause require kind name semi =
        from_children SyntaxKind.RequireClause [ require; kind; name; semi ]

      let make_const_declaration const_abstract const_token const_type_specifier
        const_declarator_list const_semicolon =
        from_children SyntaxKind.ConstDeclaration [
          const_abstract; const_token; const_type_specifier;
          const_declarator_list; const_semicolon; ]

      let make_constant_declarator constant_declarator_name
        constant_declarator_initializer =
        from_children SyntaxKind.ConstantDeclarator
          [ constant_declarator_name; constant_declarator_initializer; ]

      let make_type_const_declaration type_const_abstract type_const_const_token
        type_const_type_token type_const_name type_const_type_constraint
        type_const_equal type_const_type_specifier type_const_semicolon =
        from_children SyntaxKind.TypeConstDeclaration
          [ type_const_abstract; type_const_const_token; type_const_type_token;
            type_const_name; type_const_type_constraint; type_const_equal;
            type_const_type_specifier; type_const_semicolon; ]

      let make_decorated_expression decorator expression =
        from_children SyntaxKind.DecoratedExpression [ decorator; expression ]

      let make_parameter_declaration
        param_attr param_visibility param_type param_name param_default =
        from_children SyntaxKind.ParameterDeclaration
          [ param_attr; param_visibility; param_type; param_name;
          param_default ]

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

      let make_inclusion_expression require filename =
        from_children SyntaxKind.InclusionExpression [ require; filename ]

      let make_inclusion_directive expr semicolon =
        from_children SyntaxKind.InclusionDirective [ expr; semicolon ]

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

      let make_catch_clause keyword left catch_type catch_var right body =
        from_children SyntaxKind.CatchClause
          [ keyword; left; catch_type; catch_var; right; body ]

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

      let make_break_statement keyword level semi =
        from_children SyntaxKind.BreakStatement
          [ keyword; level; semi ]

      let make_continue_statement keyword level semi =
        from_children SyntaxKind.ContinueStatement
          [ keyword; level; semi ]

      let make_function_static_statement static decls semi =
        from_children SyntaxKind.FunctionStaticStatement [ static; decls; semi ]

      let make_simple_initializer equal value =
        from_children SyntaxKind.SimpleInitializer [ equal; value ]

      let make_static_declarator variable init =
        from_children SyntaxKind.StaticDeclarator [ variable; init ]

      let make_echo_statement
        echo_token echo_expression_list echo_semicolon =
        from_children SyntaxKind.EchoStatement
        [ echo_token; echo_expression_list; echo_semicolon; ]

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

      let make_soft_type_specifier soft_at soft_type =
        from_children SyntaxKind.SoftTypeSpecifier
          [ soft_at; soft_type ]

      let make_nullable_type_specifier nullable_question nullable_type =
        from_children SyntaxKind.NullableTypeSpecifier
          [ nullable_question; nullable_type ]

      let make_generic_type_specifier generic_class_type generic_arguments =
        from_children SyntaxKind.GenericTypeSpecifier
          [ generic_class_type; generic_arguments ]

      let make_type_arguments left items right =
        from_children SyntaxKind.TypeArguments [ left; items; right ]

      let make_type_parameters left items right =
        from_children SyntaxKind.TypeParameters [ left; items; right ]

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

      let make_pipe_variable_expression variable =
        from_children SyntaxKind.PipeVariableExpression [ variable ]

    end (* WithValueBuilder *)
  end (* WithSyntaxValue *)
end (* WithToken *)
