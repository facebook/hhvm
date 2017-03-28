(**
* Copyright (c) 2016, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the BSD-style license found in the
* LICENSE file in the "hack" directory of this source tree. An additional grant
* of patent rights can be found in the PATENTS file in the same directory.
*
*)

(* this module defines the grammar of hack using the [grammar] function, which
 * returns all productions for each non-terminal in the grammar. This module
 * specifically generate codes that are friendly to the existing parser *)
include Hack_grammar_descriptor_helper.HackGrammarTermSpec
(***********************production rules in grammar**************************)
let rec p_script = "Script", fun () ->
  [[less_than; question; name; NonTerm p_declaration_list]]

and p_declaration_list = "DeclarationList", fun () -> [
  [NonTerm p_declaration];
  [NonTerm p_declaration_list; NonTerm p_declaration];
  [NonTerm p_declaration_list; NonTerm p_declaration];
  [NonTerm p_declaration_list; NonTerm p_declaration];
  [NonTerm p_declaration_list; NonTerm p_declaration]]

and p_declaration = "Declaration", fun () -> [
  [NonTerm p_trait_declaration];
  [NonTerm p_interface_declaration];
  [NonTerm p_function_definition];
  [NonTerm p_namespace_use_declaration];
  [NonTerm p_namespace_definition];
  [NonTerm p_class_declaration];
  [NonTerm p_alias_declaration];
  [NonTerm p_inclusion_directive];
  [NonTerm p_enum_declaration]]

and p_trait_declaration= "TraitDeclaration", fun () ->
  let attr_opt = power_set [NonTerm p_attribute_spec] in
  let trait_and_name = [[trait; name]] in
  let generic_and_base = power_set [NonTerm p_generic_type_parameter_list;
    NonTerm p_class_interface_clause] in
  let left_brace = [[left_brace]] in
  let right_brace = [[right_brace]] in
  let member =
    power_set [NonTerm p_trait_use_clauses; NonTerm p_trait_member_declarations]
  in
  let (@@) lst1 lst2 = cross lst1 lst2 (@) in
    attr_opt @@ trait_and_name @@ generic_and_base @@ left_brace @@ member @@
    right_brace

and p_trait_member_declarations = "TraitMemberDeclarations", fun () -> [
  [NonTerm p_require_extends_clause];
  [NonTerm p_require_implements_clause];
  [NonTerm p_property_declaration];
  [NonTerm p_method_declaration];
  [NonTerm p_constructor_declaration];
  [NonTerm p_destructor_declaration];
]

and p_interface_declaration= "InterfaceDeclaration", fun () ->
  let attr_opt = power_set [NonTerm p_attribute_spec] in
  let interface_and_name = [[interface; name]] in
  let generic_and_base = power_set [NonTerm p_generic_type_parameter_list;
    NonTerm p_interface_base_clause] in
  let left_brace = [[left_brace]] in
  let right_brace = [[right_brace]] in
  let member = power_set [NonTerm p_interface_member_declarations] in
  let (@@) lst1 lst2 = cross lst1 lst2 (@) in
  attr_opt @@ interface_and_name @@ generic_and_base @@ left_brace @@ member @@
    right_brace

and p_interface_base_clause = "InterfaceBaseClause", fun () -> [
  [extends; qualified_name; NonTerm p_generic_argument];
  [NonTerm p_interface_base_clause; comma; qualified_name;
    NonTerm p_generic_argument];
]

and p_interface_member_declarations = "InterfaceMemberDeclarations", fun () -> [
  [NonTerm p_interface_member_declaration];
  [NonTerm p_interface_member_declarations;
    NonTerm p_interface_member_declaration];
]

and p_interface_member_declaration = "InterfaceMemberDeclaration", fun () -> [
  [NonTerm p_require_extends_clause];
  [NonTerm p_const_declaration];
  [NonTerm p_method_declaration];
  [NonTerm p_type_constant_decl];
]

and p_require_extends_clause = "RequireExtendsClause", fun () ->
  [[require; extends; qualified_name; semicolon]]

and p_require_implements_clause = "RequireImplementsClause", fun () ->
  [[require; implements; qualified_name; semicolon]]

and p_namespace_definition = "NamespaceDefinition", fun () -> [
  (* TODO generate "namespace-name" of the form name\name\name *)
  (* MODIFIED FOR OLD PARSER *)
  (* [namespace; name; semicolon]; *)
  [namespace; name; left_brace; NonTerm p_declaration_list; right_brace];
  [namespace; left_brace; NonTerm p_declaration_list; right_brace]]

and p_enum_declaration = "EnumDeclaration", fun () -> [
  [enum; name; colon; term_int; term_as; NonTerm p_type_spec; left_brace;
  NonTerm p_enumerator_list; right_brace];
  [enum; name; colon; term_string; term_as; NonTerm p_type_spec; left_brace;
  NonTerm p_enumerator_list; right_brace];
  [enum; name; colon; term_int; left_brace; NonTerm p_enumerator_list;
  right_brace];
  [enum; name; colon; term_string; left_brace; NonTerm p_enumerator_list;
  right_brace];
  [enum; name; colon; term_int; term_as; NonTerm p_type_spec; left_brace;
  right_brace];
  [enum; name; colon; term_string; term_as; NonTerm p_type_spec; left_brace;
  right_brace];
  [enum; name; colon; term_int; left_brace; right_brace];
  [enum; name; colon; term_string; left_brace; right_brace]]

and p_enumerator_list = "EnumeratorList", fun () -> [
  [NonTerm p_enumerator];
  [NonTerm p_enumerator; NonTerm p_enumerator_list]]

and p_enumerator = "Enumerator", fun () -> [
  [name; equal; NonTerm p_constant_expression; semicolon]]

and p_inclusion_directive = "InclusionDirective", fun () -> [
  [NonTerm p_require_multiple_directive];
  [NonTerm p_require_once_directive]]

and p_require_multiple_directive = "RequireMultipleDirective", fun () -> [
  [require; left_paren; NonTerm p_string_literal; right_paren; semicolon];
  [require; NonTerm p_string_literal; semicolon]]

and p_require_once_directive = "RequireOnceDirective", fun () -> [
  [require_once; left_paren; NonTerm p_string_literal; right_paren;
  semicolon];
  [require_once; NonTerm p_string_literal; semicolon]]

and p_alias_declaration = "AliasDeclaration", fun () -> [
  [term_type; name; equal; NonTerm p_type_spec; semicolon];
  [term_type; name; equal; qualified_name; semicolon];
  [newtype; name; term_as; NonTerm p_type_spec; equal; NonTerm p_type_spec;
  semicolon];
  [newtype; name; term_as; NonTerm p_type_spec; equal; qualified_name;
  semicolon];
  [newtype; name; equal; NonTerm p_type_spec; semicolon];
  [newtype; name; equal; qualified_name; semicolon]]

and p_namespace_use_declaration = "NamespaceUseDeclaration", fun () -> [
  [use; NonTerm p_namespace_use_clauses; semicolon]]

and p_namespace_use_clauses = "NamespaceUseClauses", fun () -> [
  [NonTerm p_namespace_use_clauses; comma; NonTerm p_namespace_use_clause];
  [NonTerm p_namespace_use_clause]]

and p_namespace_use_clause = "NamespaceUseClause", fun () -> [
  [qualified_name; term_as; name];
  [qualified_name]]

and p_class_declaration = "ClassDeclaration", fun () ->
  let modifiers = [[abstract]; [final]; [abstract; final]] in
  let prefixes =
  List.map (fun e -> (NonTerm p_attribute_spec) :: e) modifiers in
  let prefixes = List.flatten (List.map power_set prefixes) in
  let class_and_name = [term_class; name] in
  let after_name = power_set [NonTerm p_generic_type_parameter_list;
    NonTerm p_class_base_clause] in
  let without_prefix = List.map (fun x -> class_and_name @ x) after_name in
  let without_interface = cross prefixes without_prefix (@) in
  let body_members =
    power_set [NonTerm p_trait_use_clauses;
    NonTerm p_class_member_declarations ] in
  let body_with_closing =
    List.map (fun x -> x @ [right_brace]) body_members in
  let before_body =
    [[NonTerm p_class_interface_clause; left_brace]; [left_brace]] in
  let with_interface = cross before_body body_with_closing (@) in
  cross without_interface with_interface (@)

and p_trait_use_clauses = "TraitUseClauses", fun () -> [
  [NonTerm p_trait_use_clause];
  [NonTerm p_trait_use_clauses; NonTerm p_trait_use_clause]]

and p_trait_use_clause = "TraitUseClause", fun () ->
  [[use; NonTerm p_trait_name_list; semicolon]]

and p_trait_name_list = "TraitNameList", fun () -> [
  [qualified_name; NonTerm p_generic_argument];
  [qualified_name];
  [NonTerm p_trait_name_list; comma; qualified_name;
    NonTerm p_generic_argument];
  [NonTerm p_trait_name_list; comma; qualified_name]]

and p_class_member_declarations = "ClassMemberDeclarations", fun () -> [
  [NonTerm p_class_member_declaration];
  [NonTerm p_class_member_declarations; NonTerm p_class_member_declaration]]

and p_class_member_declaration = "ClassMemberDeclaration", fun () ->
  [
  [NonTerm p_const_declaration];
  [NonTerm p_property_declaration];
  [NonTerm p_method_declaration];
  [NonTerm p_constructor_declaration];
  [NonTerm p_destructor_declaration];
  [NonTerm p_type_constant_decl]]

and p_property_declaration = "PropertyDeclaration", fun () -> [
  [NonTerm p_property_modifier; NonTerm p_type_spec;
    NonTerm p_property_declarator_list; semicolon];
]

and p_property_declarator_list = "PropertyDeclaratorList", fun () -> [
  [NonTerm p_property_declarator_list; comma; NonTerm p_property_declarator];
  [NonTerm p_property_declarator];
]

and p_property_declarator = "PropertyDeclarator", fun () -> [
  [NonTerm p_variable_name; equal; NonTerm p_expression];
  [NonTerm p_variable_name];
]

and p_property_modifier = "PropertyModifier", fun () -> [
  [NonTerm p_visibility_modifier];
  [NonTerm p_visibility_modifier; static];
  [static; NonTerm p_visibility_modifier];
]

and p_method_declaration = "MethodDeclaration", fun () -> [
  [NonTerm p_attribute_spec; NonTerm p_method_modifiers;
    NonTerm p_function_definition_header; NonTerm p_compound_statement];
  [NonTerm p_method_modifiers; NonTerm p_function_definition_header;
    NonTerm p_compound_statement];
  [NonTerm p_attribute_spec; NonTerm p_method_modifiers;
    NonTerm p_function_definition_header; semicolon];
  [NonTerm p_method_modifiers; NonTerm p_function_definition_header; semicolon];
]

and p_method_modifiers = "MethodModifiers", fun () ->
  let visi = [[NonTerm p_visibility_modifier]] in
  let other = power_set [static; abstract; final] in
  cross visi other (@)

and p_constructor_declaration = "ConstructorDeclaration", fun () -> [
  [NonTerm p_attribute_spec; NonTerm p_constructor_modifiers; term_function;
    construct; left_paren; NonTerm p_constructor_param_decl_list; right_paren;
    NonTerm p_compound_statement];
  [NonTerm p_attribute_spec; NonTerm p_constructor_modifiers; term_function;
    construct; left_paren; right_paren; NonTerm p_compound_statement];
  [NonTerm p_constructor_modifiers; term_function;
    construct; left_paren; NonTerm p_constructor_param_decl_list; right_paren;
    NonTerm p_compound_statement];
  [NonTerm p_constructor_modifiers; term_function; construct; left_paren;
    right_paren; NonTerm p_compound_statement];
]

and p_constructor_param_decl_list = "ConstructorParameterDeclList", fun () -> [
  [NonTerm p_constructor_param_decl];
  [NonTerm p_constructor_param_decl_list; comma;
    NonTerm p_constructor_param_decl];
]

and p_constructor_param_decl = "ConstructorParameterDeclaration", fun () -> [
  [NonTerm p_visibility_modifier; NonTerm p_type_spec; NonTerm p_variable_name;
    NonTerm p_default_arg_spec];
  [NonTerm p_visibility_modifier; NonTerm p_type_spec; NonTerm p_variable_name];
  [NonTerm p_type_spec; NonTerm p_variable_name; NonTerm p_default_arg_spec];
  [NonTerm p_type_spec; NonTerm p_variable_name];
]

and p_constructor_modifiers = "ConstructorModifiers", fun () -> [
  [NonTerm p_constructor_modifier];
  [NonTerm p_constructor_modifiers; NonTerm p_constructor_modifier];
]

and p_constructor_modifier = "ConstructorModifier", fun () ->
  [[NonTerm p_visibility_modifier]; [abstract]; [final]]

and p_destructor_declaration = "DestructorDeclaration", fun () -> [
  [NonTerm p_attribute_spec; NonTerm p_visibility_modifier; term_function;
    destruct; left_paren; right_paren; NonTerm p_compound_statement];
  [NonTerm p_visibility_modifier; term_function;
    destruct; left_paren; right_paren; NonTerm p_compound_statement];
]

and p_visibility_modifier = "VisibilityModifier", fun () ->
  [[term_public]; [term_private]; [term_protected]]

and p_const_declaration = "ConstDeclaration", fun () -> [
  [abstract; const; NonTerm p_type_spec; NonTerm p_const_decl_list; semicolon];
  [abstract; const; NonTerm p_const_decl_list; semicolon];
  [const; NonTerm p_type_spec; NonTerm p_const_decl_list; semicolon];
  [const; NonTerm p_const_decl_list; semicolon];
]

and p_const_decl_list = "ConstDeclaratorList", fun () -> [
  [name; equal; NonTerm p_constant_expression];
  [name];
  [NonTerm p_const_decl_list; comma; name; equal;
    NonTerm p_constant_expression];
  [NonTerm p_const_decl_list; comma; name];
]

and p_type_constant_decl = "TypeConstantDeclaration", fun () -> [
  [abstract; const; term_type; name; NonTerm p_type_constraint; semicolon];
  [abstract; const; term_type; name; semicolon];
  [const; term_type; name; NonTerm p_type_constraint; equal;
    NonTerm p_type_spec; semicolon];
  [const; term_type; name; equal; NonTerm p_type_spec; semicolon]]

and p_class_interface_clause = "ClassInterfaceClause", fun () -> [
  [implements; qualified_name; NonTerm p_generic_argument];
  [implements; qualified_name];
  [NonTerm p_class_interface_clause; comma; qualified_name;
    NonTerm p_generic_argument];
  [NonTerm p_class_interface_clause; comma; qualified_name]]

and p_class_base_clause = "ClassBaseClause", fun () -> [
  [extends; qualified_name; NonTerm p_generic_argument];
  [extends; qualified_name]]

and p_function_definition = "FunctionDefinition", fun () -> [
  [NonTerm p_attribute_spec; NonTerm p_function_definition_header;
    NonTerm p_compound_statement];
  [NonTerm p_function_definition_header; NonTerm p_compound_statement];
]

and p_function_definition_header = "FunctionDefinitionHeader", fun () ->
  let headers = power_set [async] in
  let parameters = [
    [left_paren; right_paren];
    [left_paren; NonTerm p_parameter_list; right_paren];
    [NonTerm p_generic_type_parameter_list; left_paren;
      right_paren];
    [NonTerm p_generic_type_parameter_list; left_paren;
        NonTerm p_parameter_list; right_paren]]
  in
  let postfix =
    List.map (fun x -> [term_function; name]
      @ x @ [colon; NonTerm p_type_spec]) parameters
  in
  List.map (fun head -> List.map (fun tl -> head @ tl) postfix) headers
  |> List.flatten

and p_parameter_list = "ParameterList", fun () -> [
  [dot_dot_dot];
  [NonTerm p_parameter_declaration_list];
  [NonTerm p_parameter_declaration_list; comma; dot_dot_dot]]

and p_generic_type_parameter_list = "GenericTypeParameterList", fun () ->
  [[less_than; NonTerm p_generic_type_parameters; greater_than]]

and p_generic_type_parameters = "GenericTypeParameters", fun () -> [
  [NonTerm p_generic_type_parameter];
  [NonTerm p_generic_type_parameters; comma;
    NonTerm p_generic_type_parameter]]

and p_generic_type_parameter = "GenericTypeParameter", fun () -> [
  [plus; name; NonTerm p_type_constraint];
  [minus; name; NonTerm p_type_constraint];
  [name; NonTerm p_type_constraint];
  [plus; name];
  [minus; name];
  [name]]

and p_type_constraint = "TypeConstraint", fun () ->
  [[term_as; NonTerm p_type_spec]]

and p_statement = "Statement", fun () -> [
(* TODO *)
  (* [function_static_declaration]; *)
  [NonTerm p_compound_statement];
  (* MODIFIED FOR OLD PARSER *)
  (* [NonTerm p_labeled_statement]; *)
  [NonTerm p_expression_statement];
  [NonTerm p_if_statement];
  [NonTerm p_switch_statement];
  [NonTerm p_while_statement];
  [NonTerm p_do_statement];
  [NonTerm p_for_statement];
  [NonTerm p_foreach_statement];
  [NonTerm p_jump_statement];
  [NonTerm p_try_statement]]

and p_function_static_declaration = "FunctionStaticDeclaration", fun () ->
  [[static; NonTerm p_static_declaration_list; semicolon]]

and p_static_declaration_list = "StaticDeclarationList", fun () -> [
  [NonTerm p_static_declarator];
  [NonTerm p_static_declaration_list; comma; NonTerm p_static_declarator]]

and p_static_declarator = "StaticDeclarator", fun () -> [
  [NonTerm p_variable_name];
  [NonTerm p_variable_name; equal; NonTerm p_expression]]

and p_variable_name = "VariableName", fun () -> [[dollar]]

and p_compound_statement = "CompoundStatement", fun () -> [
  [left_brace; right_brace];
  [left_brace; NonTerm p_statement; right_brace];
  [left_brace; NonTerm p_statement_list; right_brace]]

and p_statement_list = "StatementList", fun () -> [
  [NonTerm p_statement];
  [NonTerm p_statement; NonTerm p_statement];
  [NonTerm p_statement; NonTerm p_statement_list; NonTerm p_statement_list];
  [NonTerm p_statement_list; NonTerm p_statement];
  [NonTerm p_statement_list; NonTerm p_statement];
  [NonTerm p_statement_list; NonTerm p_statement];
  [NonTerm p_statement_list; NonTerm p_statement]]

and p_labeled_statement = "LabeledStatement", fun () -> [
  [case; NonTerm p_expression; colon; NonTerm p_statement];
  [default; colon; NonTerm p_statement]]

and p_jump_statement = "JumpStatement", fun () -> [
  [continue; semicolon]; [break; semicolon];
  [return; semicolon];
  [return; NonTerm p_expression; semicolon];
  [throw; NonTerm p_expression; semicolon]]

and p_try_statement = "TryStatement", fun () -> [
  [term_try; NonTerm p_compound_statement; NonTerm p_catch_clauses];
  [term_try; NonTerm p_compound_statement; finally;
   NonTerm p_compound_statement];
  [term_try; NonTerm p_compound_statement; NonTerm p_catch_clauses; finally;
   NonTerm p_compound_statement]]

and p_catch_clauses = "CatchClauses", fun () -> [
  [NonTerm p_catch_clause];
  [NonTerm p_catch_clauses; NonTerm p_catch_clause]]

and p_catch_clause = "CatchClause", fun () -> [
  [catch; left_paren; NonTerm p_parameter_declaration_list;
    right_paren; NonTerm p_compound_statement]]

and p_if_statement = "IfStatement", fun () -> [
  [term_if; left_paren; NonTerm p_expression; right_paren;
    NonTerm p_statement];
  [term_if; left_paren; NonTerm p_expression; right_paren;
    NonTerm p_statement; NonTerm p_elseif_clauses];
  [term_if; left_paren; NonTerm p_expression; right_paren;
    NonTerm p_statement; NonTerm p_elseif_clauses; term_else;
    NonTerm p_statement]]

and p_elseif_clauses = "ElseifClauses", fun () -> [
  [NonTerm p_elseif_clause];
  [NonTerm p_elseif_clauses; NonTerm p_elseif_clause]]

and p_elseif_clause = "ElseifClause", fun () -> [
  [elseif; left_paren; NonTerm p_expression; right_paren;
    NonTerm p_statement]]

and p_switch_statement = "SwitchStatement", fun () -> [
  (* MODIFIED FOR OLD PARSER *)
  [switch; left_paren; NonTerm p_expression; right_paren;
    left_brace; NonTerm p_case_list; right_brace]]

(* MODIFIED FOR OLD PARSER *)
and p_case_list = "CaseList", fun () -> [
  [NonTerm p_labeled_statement];
  [NonTerm p_labeled_statement; NonTerm p_statement_list];
  [NonTerm p_case_list; NonTerm p_labeled_statement];
  [NonTerm p_case_list; NonTerm p_labeled_statement; NonTerm p_statement_list]
]

and p_while_statement = "WhileStatement", fun () -> [
  [term_while; left_paren; NonTerm p_expression; right_paren;
    NonTerm p_statement]]

and p_do_statement = "DoStatement", fun () -> [
  [term_do; NonTerm p_statement; term_while; left_paren;
    NonTerm p_expression; right_paren; semicolon]]

and p_for_statement = "ForStatement", fun () ->
  let end_part = [right_paren; NonTerm p_statement] in
  let for_expr_g = NonTerm p_for_expression_group in
  let middle_parts = [
    [for_expr_g; semicolon; for_expr_g; semicolon; for_expr_g];
    [for_expr_g; semicolon; for_expr_g; semicolon];
    [for_expr_g; semicolon; semicolon; for_expr_g];
    [semicolon; for_expr_g; semicolon; for_expr_g];
    [for_expr_g; semicolon; semicolon];
    [semicolon; for_expr_g; semicolon];
    [semicolon; semicolon; for_expr_g];
    [semicolon; semicolon]]
  in
  List.map (fun x -> [term_for; left_paren] @ x @ end_part) middle_parts

and p_for_expression_group = "ForExpressionGroup", fun () -> [
  [NonTerm p_expression];
  [NonTerm p_for_expression_group; comma; NonTerm p_expression]]

and p_foreach_statement = "ForeachStatement", fun () ->
  let front_part = [foreach; left_paren; NonTerm p_expression] in
  let end_part = [NonTerm p_foreach_value; right_paren;
    NonTerm p_statement] in
  let middle_parts = [
    [term_as];
    [term_as; NonTerm p_expression; equal_greater_than];
    [await; term_as];
    [await; term_as; NonTerm p_expression; equal_greater_than]] in
  List.map (fun x -> front_part @ x @ end_part) middle_parts

and p_foreach_value = "ForeachValue", fun () -> [
  (* MODIFIED FOR OLD PARSER *)
  [NonTerm p_variable_name];
  [NonTerm p_list_intrinsic]]

and p_parameter_declaration_list = "ParameterDeclarationList", fun () -> [
  [NonTerm p_parameter_declaration];
  [NonTerm p_parameter_declaration_list; comma;
    NonTerm p_parameter_declaration]]

and p_parameter_declaration = "ParameterDeclaration", fun () -> (* TODO *)
  let middle_part = [NonTerm p_type_spec; NonTerm p_variable_name] in
  [middle_part;
   NonTerm p_attribute_spec :: middle_part;
   middle_part @ [NonTerm p_default_arg_spec];
   NonTerm p_attribute_spec :: middle_part @ [NonTerm p_default_arg_spec]]

and p_type_spec = "TypeSpec", fun () -> [
  [void]; [term_int]; [term_bool]; [term_float]; [arraykey]; [num];
  [resource]; [term_string]; [this]; [qualified_name]; [name]; [mixed];
  [qualified_name; less_than; NonTerm p_generic_type_arguments; greater_than];
  [classname; less_than; qualified_name; greater_than];
  [left_paren; term_function; left_paren; NonTerm p_type_spec_list;
  right_paren; colon; NonTerm p_type_spec; right_paren];
  [left_paren; term_function; left_paren; right_paren; colon;
  NonTerm p_type_spec; right_paren];
  [term_array; less_than; NonTerm p_type_spec; comma; NonTerm p_type_spec;
  greater_than];
  [question; NonTerm p_type_spec];
  [shape; left_paren; NonTerm p_field_spec_list; right_paren];
  [shape; left_paren; right_paren];
  [left_paren; NonTerm p_type_spec; comma; NonTerm p_type_spec_list;
  right_paren];
  [NonTerm p_type_constant_type_name];
  [term_array; less_than; NonTerm p_type_spec; greater_than]]

and p_generic_argument = "GenericArgument", fun () ->
  [[less_than; NonTerm p_generic_type_arguments; greater_than]]

and p_generic_type_arguments = "GenericTypeArguments", fun () -> [
  [name];
  [NonTerm p_type_spec];
  [NonTerm p_generic_type_arguments; comma; name];
  [NonTerm p_generic_type_arguments; comma; NonTerm p_type_spec]]

and p_type_spec_list = "TypeSpecList", fun () -> [
  [NonTerm p_type_spec];
  [NonTerm p_type_spec_list; comma; NonTerm p_type_spec]]

and p_field_spec_list = "FieldSpecList", fun () -> [
  [NonTerm p_field_spec];
  [NonTerm p_field_spec_list; comma; NonTerm p_field_spec]]

and p_field_spec = "FieldSpec", fun () -> [
  (* MODIFIED FOR OLD PARSER *)
  [single_quoted_string_literal; equal_greater_than; NonTerm p_type_spec];
  (* [qualified_name; equal_greater_than; NonTerm p_type_spec] *)]

and p_type_constant_type_name = "TypeConstTypeName", fun () -> [
  [name; colon_colon; name];
  [self; colon_colon; name];
  [this; colon_colon; name];
  [NonTerm p_type_constant_type_name; colon_colon; name]]

and p_default_arg_spec = "DefaultArgumentSpec", fun () ->
  [[equal; NonTerm p_expression]]

and p_attribute_spec = "AttributeSpec", fun () ->
  [[less_than_less_than; NonTerm p_attribute_list; greater_than_greater_than]]

and p_attribute_list = "AttributeList", fun () -> [
  [NonTerm p_attribute];
  [NonTerm p_attribute_list; comma; NonTerm p_attribute]]

and p_attribute = "Attribute", fun () -> [
  [name];
  [name; left_paren; right_paren];
  [name; left_paren; NonTerm p_attribute_values; right_paren]]

and p_attribute_values = "AttributeValues", fun () -> [
  [NonTerm p_expression];
  [NonTerm p_attribute_values; comma ; NonTerm p_expression]]

and p_expression_statement = "ExpressionStatement", fun () -> [
  [semicolon];
  [NonTerm p_expression; semicolon]]

and p_expression = "Expression", fun () -> [
  (* DEVIATION non-assignment options are pre-expanded to increase their
   * likelihood *)
  [NonTerm p_assignment_expression];
  [NonTerm p_primary_expression];
  [NonTerm p_unary_expression];
  (* DEVIATION: complex hierarchy is flattened here *)
  [NonTerm p_binary_expression]
  (* [yield; array_element_initializer] *)
  ]

and p_xhp_expression = "XHPExpression", fun () -> [
  (* Note: 1. opening and closing names are not made the same
   *       2. opening is used as xhp_element_name which comes with a < token
   *       3. closing is just used as a Hack name token
   *)

  [xhp_element_name; NonTerm p_xhp_attributes; slash_greater_than];
  [xhp_element_name; slash_greater_than];
  [xhp_element_name; NonTerm p_xhp_attributes; greater_than;
  NonTerm p_xhp_body_elements; less_than_slash; name; greater_than];
  [xhp_element_name; greater_than; NonTerm p_xhp_body_elements;
  less_than_slash; name; greater_than];
  [xhp_element_name; NonTerm p_xhp_attributes; greater_than;
  less_than_slash; name; greater_than];
  [xhp_element_name; greater_than; less_than_slash; name;
  greater_than]]

and p_xhp_attributes = "XHPAttributes", fun () -> [
  [NonTerm p_xhp_attribute];
  [NonTerm p_xhp_attribute; NonTerm p_xhp_attributes]]

and p_xhp_attribute = "XHPAttribute", fun () -> [
  [name; equal; left_brace; NonTerm p_expression; right_brace];
  [name; equal; xhp_string_literal]]

and p_xhp_body_elements = "XHPBodyElements", fun () -> [
  [NonTerm p_xhp_body_element];
  [NonTerm p_xhp_body_element; NonTerm p_xhp_body_elements]]

and p_xhp_body_element = "XHPBodyElement", fun () -> [
  [NonTerm p_xhp_expression];
  [xhp_body_text];
  [xhp_comment];
  [left_brace; NonTerm p_expression; right_brace]]

and p_binary_expression = "BinaryExpression", fun () -> [
(* TODO this exludes assignments.
 * Should assignements appear in binary expr? *)
  (* MODIFIED FOR OLD PARSER *)
  [left_paren; NonTerm p_unary_expression; right_paren];
  [left_paren; NonTerm p_binary_expression; right_paren; NonTerm p_binary_op;
    left_paren; NonTerm p_binary_expression; right_paren];
  [left_paren; NonTerm p_conditional_expression; right_paren];
  [left_paren; NonTerm p_instanceof_expression; right_paren]]

and p_conditional_expression = "ConditionalExpression", fun () -> [
  [NonTerm p_binary_expression; question; left_paren; NonTerm p_expression;
    right_paren; colon; NonTerm p_binary_expression;];
  [NonTerm p_binary_expression; question; colon; NonTerm p_binary_expression]]

and p_binary_op = "BinaryOperator", fun () -> [
  (* MODIFIED FOR OLD PARSER *)
  (* [star_star];*) [star]; [slash]; [percent]; [plus]; [minus]; [dot];
  [less_than_less_than]; [greater_than_greater_than]; [less_than];
  [greater_than]; [less_than_equal]; [greater_than_equal]; [equal_equal];
  [exclamation_equal]; [equal_equal_equal]; [exclamation_equal_equal];
  [ampersand]; [carat]; [bar]; [ampersand_ampersand]; [bar_bar]]

and p_instanceof_expression = "InstanceofExpression", fun () -> [
  (* MODIFIED FOR OLD PARSER *)
  [NonTerm p_unary_expression];
  [left_paren; NonTerm p_expression; instanceof; NonTerm p_instance_designator;
    right_paren]]

and p_instance_designator = "InstanceTypeDesignator", fun () -> [
  [qualified_name]; [NonTerm p_variable_name]]

and p_primary_expression = "PrimaryExpression", fun () -> [
  (* TODO *)
  [NonTerm p_variable_name];
  (* MODIFIED FOR OLD PARSER *)
  (* [qualified_name]; *)
  [NonTerm p_literal];
  [NonTerm p_intrinsic];
  (* [NonTerm p_collection_literal]; *)
  (* [NonTerm p_tuple_literal]; *)
  [NonTerm p_shape_literal];
  [NonTerm p_anonymous_fun_creation];
  (* [NonTerm p_awaitable_creation]; *)
  [left_paren; NonTerm p_expression; right_paren];
  (*[dollar_dollar]*)]

and p_list_intrinsic = "ListIntrinsic", fun () -> [
  [term_list; left_paren; right_paren];
  [term_list; left_paren; NonTerm p_list_expression_list; right_paren]]

and p_list_expression_list = "ListExpressionList", fun () -> [
  (* MODIFIED FOR OLD PARSER *)
  [NonTerm p_variable_name];
  (* [comma];
  [NonTerm p_list_expression_list; comma]; *)
  [NonTerm p_list_expression_list; comma; NonTerm p_variable_name]]

and p_literal = "Literal", fun () -> [
  (* MODIFIED FOR OLD PARSER *)
  (* [boolean_literal]; *)
  [NonTerm p_int_literal];
  [floating_literal];
  [NonTerm p_string_literal];
  (* [null_literal] *)]

and p_int_literal = "IntLiteral", fun () -> [
  [decimal_literal];
  [octal_literal];
  [hexadecimal_literal];
  [binary_literal]]

and p_string_literal = "StringLiteral", fun () -> [
  [single_quoted_string_literal];
  [double_quoted_string_literal];
  [heredoc_string_literal];
  [nowdoc_string_literal]]

and p_constant_expression = "ConstantExpression", fun () -> [
  [NonTerm p_array_creation_expression];
  [NonTerm p_literal];
  [name];
  (* [NonTerm p_collection_literal]; *)
  (* [NonTerm p_tuple_literal]; *)
  [NonTerm p_shape_literal];
]

and p_intrinsic = "Intrinsic", fun () -> [
  [NonTerm p_array_intrinsic];
  (* [NonTerm p_echo_intrinsic]; *)
  (* [NonTerm p_exit_intrinsic]; *)
  (* [invariant_intrinsic]; *) (* TODO get this back into the code *)
  [NonTerm p_list_intrinsic]]

and p_array_intrinsic = "ArrayIntrinsic", fun () -> [
  [term_array; left_paren; right_paren];
  [term_array; left_paren; NonTerm p_array_initializer; right_paren]]

and p_echo_intrinsic = "EchoIntrinsic", fun () -> [
  [echo; NonTerm p_expression];
  [echo; left_paren; NonTerm p_expression; right_paren];
  [echo; NonTerm p_expression_list_two_or_more]]

and p_exit_intrinsic = "ExitIntrinsic", fun () -> [
  [term_exit];
  [term_exit; NonTerm p_expression];
  [term_exit; left_paren; right_paren];
  [term_exit; left_paren; NonTerm p_expression; right_paren]]

(* TODO grammar here looks weird: no explanation on format and condition *)
(* and p_invariant_intrinsic = "", fun () -> [
  [invariant; left_paren; condition; comma; format; right_paren];
  [invariant; left_paren; condition; comma; format; comma; values;
    right_paren]] *)

and p_collection_literal = "CollectionLiteral", fun () -> [
  [qualified_name; left_brace; right_brace];
  [qualified_name; left_brace; NonTerm p_cl_initializer_list_no_keys;
    right_brace];
  [qualified_name; left_brace; right_brace];
  [qualified_name; left_brace; NonTerm p_cl_initializer_list_keys;
    right_brace];
  [qualified_name; left_brace; NonTerm p_expression; comma;
    NonTerm p_expression; right_brace]]

and p_cl_initializer_list_keys = "CLInitializerListKeys", fun () -> [
  [NonTerm p_expression; equal_greater_than; NonTerm p_expression];
  [NonTerm p_cl_initializer_list_keys; comma; NonTerm p_expression;
    equal_greater_than; NonTerm p_expression]]

and p_cl_initializer_list_no_keys = "CLInitializerListNoKeys", fun () -> [
  [NonTerm p_expression];
  [NonTerm p_cl_initializer_list_no_keys; comma; NonTerm p_expression]]

and p_tuple_literal = "TupleLiteral", fun () -> [
  [tuple; left_paren; NonTerm p_expression_list_two_or_more; right_paren]]

and p_shape_literal = "ShapeLiteral", fun () -> [
  [shape; left_paren; right_paren];
  [shape; left_paren; NonTerm p_field_initializer_list; right_paren]]

and p_field_initializer_list = "FieldInitializerList", fun () -> [
  [NonTerm p_field_initializer];
  [NonTerm p_field_initializer_list; comma; NonTerm p_field_initializer]]

and p_field_initializer = "FieldInitializer", fun () -> [
  [single_quoted_string_literal; equal_greater_than; NonTerm p_expression];
  (* MODIFIED FOR OLD PARSER *)
  (* [NonTerm p_int_literal; equal_greater_than; NonTerm p_expression]; *)
  (* [qualified_name; equal_greater_than; NonTerm p_expression]; *)
]

and p_anonymous_fun_creation = "AnonymousFunctionCreation", fun () ->
  let before_body = [
    (* TODO *)
    (* [async; term_function; left_paren;
    NonTerm p_anonymous_fun_param_decl_list; right_paren];
    [async; term_function; left_paren; right_paren]; *)
    [term_function; left_paren; NonTerm p_anonymous_fun_param_decl_list;
    right_paren];
    [term_function; left_paren; right_paren]] in
  let return_and_use =
    power_set [NonTerm p_anonymous_fun_return; NonTerm p_anonymous_fun_use] in
  let before_statement = cross before_body return_and_use (@) in
  let compound_stmt = [NonTerm p_compound_statement] in
  List.map (fun x -> x @ compound_stmt) before_statement

and p_anonymous_fun_return = "AnonymousFunReturn", fun () ->
  [[colon; NonTerm p_type_spec]]

and p_anonymous_fun_use = "AnonymousFunUseClause", fun () ->
  [[use; left_paren; NonTerm p_use_variable_name_list; right_paren]]

and p_use_variable_name_list = "UseVariableNameList", fun () -> [
  [NonTerm p_variable_name];
  [NonTerm p_use_variable_name_list; comma; NonTerm p_variable_name]]

and p_anonymous_fun_param_decl_list = "AnonymousFunParamDeclList", fun () -> [
  [NonTerm p_anonymous_fun_param_decl];
  [NonTerm p_anonymous_fun_param_decl_list; comma;
    NonTerm p_anonymous_fun_param_decl]]

and p_anonymous_fun_param_decl = "AnonymousFunParamDecl", fun () -> [
  [NonTerm p_variable_name];
  [NonTerm p_variable_name; NonTerm p_default_arg_spec];
  [NonTerm p_attribute_spec; NonTerm p_variable_name];
  [NonTerm p_type_spec; NonTerm p_variable_name];
  [NonTerm p_type_spec; NonTerm p_variable_name; NonTerm p_default_arg_spec];
  [NonTerm p_attribute_spec; NonTerm p_variable_name;
    NonTerm p_default_arg_spec];
  [NonTerm p_attribute_spec; NonTerm p_type_spec; NonTerm p_variable_name];
  [NonTerm p_attribute_spec; NonTerm p_type_spec; NonTerm p_variable_name;
    NonTerm p_default_arg_spec];
]

and p_awaitable_creation = "AwaitableCreation", fun () -> [
  [async; left_brace; right_brace];
  [async; left_brace; NonTerm p_async_statement_list; right_brace]]

and p_async_statement_list = "AsyncStatementList", fun () -> [
  [NonTerm p_statement];
  [NonTerm p_async_statement_list; NonTerm p_statement]]

and p_assignment_expression = "AssignmentExpression", fun () -> [
  (* [lambda_expression]; *)
  [NonTerm p_simple_assignment_expression];
  [NonTerm p_compound_assignment_expression]]

and p_lambda_expression = "LambdaExpression", fun () -> [
(* [piped_expression]; *) (* TODO removed since coalesce is not ready *)
[async; NonTerm p_lambda_function_signature; equal_equal_greater_than;
  NonTerm p_anonymous_fun_body];
[NonTerm p_lambda_function_signature; equal_equal_greater_than;
  NonTerm p_anonymous_fun_body]]

and p_lambda_function_signature = "LambdaFunctionSignature", fun () -> [
  [NonTerm p_variable_name];
  [left_paren; NonTerm p_anonymous_fun_param_decl_list; right_paren;
    NonTerm p_anonymous_fun_return];
  [left_paren; right_paren; NonTerm p_anonymous_fun_return];
  [left_paren; NonTerm p_anonymous_fun_param_decl_list; right_paren];
  [left_paren; right_paren]]

and p_anonymous_fun_body = "AnonymousFunBody", fun () -> [
  [NonTerm p_expression];
  [NonTerm p_compound_statement]]

and p_piped_expression = "PipedExpression", fun () -> [
  [NonTerm p_coalesce_expression];
  [NonTerm p_piped_expression; bar_greater_than;
    NonTerm p_coalesce_expression]]

(* TODO grammar did not explain what coalesce is *)
and p_coalesce_expression = "CoalesceExpression", fun () ->
  [[question_question; NonTerm p_expression]]

and p_simple_assignment_expression = "SimpleAssignmentExpression", fun () ->
  (* MODIFIED FOR OLD PARSER *)
  [[dollar; equal; left_paren; NonTerm p_assignment_expression; right_paren];
  (* this does not appear in the grammar.md but this is necessary for the
   * grammar to terminate *)
  [dollar; equal; NonTerm p_unary_expression];
  [dollar; equal; NonTerm p_primary_expression]]

and p_unary_expression = "UnaryExpression", fun () -> [
  [NonTerm p_postfix_expression];
  [NonTerm p_cast_expression];
  [NonTerm p_unary_prefix_expression]
  ]

and p_unary_prefix_expression = "UnaryPrefixExpression", fun () -> [
  (* MODIFIED FOR OLD PARSER *)
  (* [plus_plus; NonTerm p_unary_expression]; *)
  (* [minus_minus; NonTerm p_unary_expression]; *)
  [NonTerm p_unary_operator; NonTerm p_cast_expression];
  [at; NonTerm p_expression];
  [await; NonTerm p_expression]]


and p_postfix_expression = "PostfixExpression", fun () -> [
  [NonTerm p_primary_expression];
  [clone; NonTerm p_expression];
  [NonTerm p_object_creation_expression];
  [NonTerm p_array_creation_expression];
  [NonTerm p_subscript_expression];
  [NonTerm p_function_call_expression];
  [NonTerm p_postfix_expression; minus_greater_than; name];
  (* MODIFIED FOR OLD PARSER *)
  [NonTerm p_postfix_expression; minus_greater_than; left_paren;
    NonTerm p_expression; right_paren];
  (* MODIFIED FOR OLD PARSER *)
  (* [NonTerm p_postfix_expression; question_minus_greater_than; name]; *)
  [left_paren; NonTerm p_variable_name; plus_plus; right_paren];
  [left_paren; NonTerm p_variable_name; minus_minus; right_paren];
  (* [NonTerm p_scope_resolution_quantifier; colon_colon; name]; *)
  (* [NonTerm p_scope_resolution_quantifier; colon_colon; term_class];
  [NonTerm p_expression; star_star; NonTerm p_expression] *)]

and p_unary_operator = "UnaryOperator", fun () ->
  [[plus]; [minus]; [exclamation]; [tilde]]

and p_object_creation_expression = "ObjectCreationExpression", fun () -> [
  [term_new; static; left_paren; NonTerm p_argument_expression_list;
    right_paren];
  [term_new; static; left_paren; right_paren];
  [term_new; qualified_name; left_paren; NonTerm p_argument_expression_list;
    right_paren];
  [term_new; qualified_name; left_paren; right_paren];
  [term_new; NonTerm p_variable_name; left_paren;
    NonTerm p_argument_expression_list; right_paren];
  [term_new; NonTerm p_variable_name; left_paren; right_paren]]

and p_argument_expression_list = "ArgumentExpressionList", fun () -> [
  [NonTerm p_expression];
  [NonTerm p_argument_expression_list; comma; NonTerm p_expression]]

and p_array_creation_expression = "ArrayCreationExpression", fun () -> [
  [term_array; left_paren; NonTerm p_array_initializer; right_paren];
  [term_array; left_paren; right_paren];
  [left_bracket; NonTerm p_array_initializer; right_bracket];
  [left_bracket; right_bracket]]

and p_subscript_expression = "SubscriptExpression", fun () -> [
  [NonTerm p_postfix_expression; left_bracket; right_bracket];
  [NonTerm p_postfix_expression; left_bracket; NonTerm p_expression;
    right_bracket];
  (* MODIFIED FOR OLD PARSER *)
  (* [NonTerm p_postfix_expression; left_brace; right_brace];
  [NonTerm p_postfix_expression; left_brace; NonTerm p_expression;
    right_brace] *)
]

and p_function_call_expression = "FunctionCallExpression", fun () -> [
  [NonTerm p_postfix_expression; left_paren;
    NonTerm p_argument_expression_list; right_paren];
  [NonTerm p_postfix_expression; left_paren; right_paren]]

and p_scope_resolution_quantifier = "ScopeResolutionQuantifier", fun () -> [
  [qualified_name];
  [NonTerm p_variable_name];
  [self];
  [parent];
  [static]]

and p_cast_expression = "CastExpression", fun () -> [
  [left_paren; term_bool; right_paren; NonTerm p_expression];
  [left_paren; term_int; right_paren; NonTerm p_expression];
  [left_paren; term_float; right_paren; NonTerm p_expression];
  [left_paren; term_string; right_paren; NonTerm p_expression]]

and p_compound_assignment_operator = "CompoundAssignmentOperator", fun () -> [
  (* MODIFIED FOR OLD PARSER *)
  (* [star_star_equal];*)
  [star_equal];[slash_equal];[percent_equal];[plus_equal];
  [minus_equal];[dot_equal];[less_less_equal];[greater_greater_equal];
  [ampersand_equal];[carat_equal];[bar_equal]]

and p_compound_assignment_expression = "CompoundAssignmentExpression",
  fun () -> [
  (* MODIFIED FOR OLD PARSER *)
  [dollar; NonTerm p_compound_assignment_operator; left_paren;
    NonTerm p_assignment_expression; right_paren];
  (* this does not appear in the grammar.md but this is necessary for the
   * grammar to terminate *)
  [dollar; NonTerm p_compound_assignment_operator;
    NonTerm p_primary_expression];
  [dollar; NonTerm p_compound_assignment_operator;
    NonTerm p_unary_expression]]

and p_array_element_initializer = "ArrayElementInitializer", fun () -> [
  [NonTerm p_expression];
  [NonTerm p_expression; equal_greater_than; NonTerm p_expression]]

and p_array_initializer = "ArrayInitializer", fun () -> [
  [NonTerm p_array_initializer_list];
  [NonTerm p_array_initializer_list; comma]]

and p_array_initializer_list = "ArrayInitializerList", fun () -> [
  [NonTerm p_array_element_initializer];
  [NonTerm p_array_element_initializer; comma;
    NonTerm p_array_initializer_list]]

and p_expression_list_two_or_more = "ExpressionListTwoOrMore", fun () -> [
  [NonTerm p_expression; comma; NonTerm p_expression];
  [NonTerm p_expression_list_two_or_more; comma; NonTerm p_expression]]

(***********************interface defined functions**************************)

let grammar (_, nonterm_fun) = nonterm_fun ()

let start = p_declaration

let nonterm_to_string (name, _) = name
