(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* If you make changes to the schema that cause it to serialize / deserialize
differently, please update this version number *)
let full_fidelity_schema_version_number = "2017-02-09-0001"
(* TODO: Consider basing the version number on an auto-generated
hash of a file rather than relying on people remembering to update it. *)
(* TODO: It may be worthwhile to investigate how Thrift describes data types
and use that standard. *)

type schema_node = {
  kind_name : string;
  type_name : string;
  description : string;
  prefix : string;
  fields : string list
}

type trivia_node = {
  trivia_kind : string;
  trivia_text : string
}

type token_node = {
    token_kind : string;
    token_text : string
}

type transformation =
{
  pattern : string;
  func : schema_node -> string
}

type token_transformation =
{
  token_pattern : string;
  token_func : token_node list -> string
}

type trivia_transformation =
{
  trivia_pattern : string;
  trivia_func : trivia_node list -> string
}

type template_file =
{
  filename : string;
  template : string;
  transformations : transformation list;
  token_no_text_transformations : token_transformation list;
  token_variable_text_transformations : token_transformation list;
  token_given_text_transformations : token_transformation list;
  trivia_transformations : trivia_transformation list;
}

let from_list l =
  match l with
  | kind_name :: type_name :: description :: prefix :: fields ->
    { kind_name; type_name; description; prefix; fields }
  | _ -> failwith "bad schema"

let token_node_from_list l =
  match l with
  | [ token_kind; token_text ] ->
    { token_kind; token_text }
  | _ -> failwith "bad token schema"

let trivia_node_from_list l =
  match l with
  | [ trivia_kind; trivia_text ] ->
    { trivia_kind; trivia_text }
  | _ -> failwith "bad trivia schema"

let schema = List.map from_list [
  [ "EndOfFile";
    "end_of_file";
    "end_of_file";
    "end_of_file";
    "token" ];
  [ "ScriptHeader";
    "script_header";
    "header";
    "header";
    "less_than";
    "question";
    "language" ];
  [ "Script";
    "script";
    "script";
    "script";
    "header";
    "declarations" ];
  [ "ScriptFooter";
    "script_footer";
    "footer";
    "footer";
    "question_greater_than" ];
  [ "SimpleTypeSpecifier";
    "simple_type_specifier";
    "simple_type_specifier";
    "simple_type";
    "specifier" ];
  [ "LiteralExpression";
    "literal_expression";
    "literal";
    "literal";
    "expression" ];
  [ "VariableExpression";
    "variable_expression";
    "variable";
    "variable";
    "expression" ];
  [ "QualifiedNameExpression";
    "qualified_name_expression";
    "qualified_name";
    "qualified_name";
    "expression" ];
  [ "PipeVariableExpression";
    "pipe_variable_expression";
    "pipe_variable";
    "pipe_variable";
    "expression" ];
  [ "EnumDeclaration";
    "enum_declaration";
    "enum_declaration";
    "enum";
    "attribute_spec";
    (* TODO: Make all uses of attribute_spec consistent in the API. *)
    "keyword";
    "name";
    "colon";
    "base";
    "type";
    "left_brace";
    "enumerators";
    "right_brace" ];
  [ "Enumerator";
    "enumerator";
    "enumerator";
    "enumerator";
    "name";
    "equal";
    "value";
    "semicolon" ];
  [ "AliasDeclaration";
    "alias_declaration";
    "alias_declaration";
    "alias";
    "attribute_spec";
    "keyword";
    "name";
    "generic_parameter";
    "constraint";
    "equal";
    "type";
    "semicolon" ];
  [ "PropertyDeclaration";
    "property_declaration";
    "property_declaration";
    "property";
    "modifiers";
    "type";
    "declarators";
    "semicolon" ];
  [ "PropertyDeclarator";
    "property_declarator";
    "property_declarator";
    "property";
    "name";
    "initializer" ];
  [ "NamespaceDeclaration";
    "namespace_declaration";
    "namespace_declaration";
    "namespace";
    "keyword";
    "name";
    "body" ];
  [ "NamespaceBody";
    "namespace_body";
    "namespace_body";
    "namespace";
    "left_brace";
    "declarations";
    "right_brace" ];
  [ "NamespaceUseDeclaration";
    "namespace_use_declaration";
    "namespace_use_declaration";
    "namespace_use";
    "keyword";
    "kind";
    "clauses";
    "semicolon"];
  [ "NamespaceGroupUseDeclaration";
    "namespace_group_use_declaration";
    "namespace_group_use_declaration";
    "namespace_group_use";
    "keyword";
    "kind";
    "prefix";
    "left_brace";
    "clauses";
    "right_brace";
    "semicolon" ];
  [ "NamespaceUseClause";
    "namespace_use_clause";
    "namespace_use_clause";
    "namespace_use";
    "clause_kind";
    "name";
    "as";
    "alias" ];
  [ "FunctionDeclaration";
    "function_declaration";
    "function_declaration";
    "function";
    "attribute_spec";
    "declaration_header";
    "body" ];
  [ "FunctionDeclarationHeader";
    "function_declaration_header";
    "function_declaration_header";
    "function";
    "async";
    "keyword";
    "ampersand";
    "name";
    "type_parameter_list";
    "left_paren";
    "parameter_list";
    "right_paren";
    "colon";
    "type";
    "where_clause" ];
  [ "WhereClause";
    "where_clause";
    "where_clause";
    "where_clause";
    "keyword";
    "constraints" ];
  [ "WhereConstraint";
    "where_constraint";
    "where_constraint";
    "where_constraint";
    "left_type";
    "operator";
    "right_type" ];
  [ "MethodishDeclaration";
    "methodish_declaration";
    "methodish_declaration";
    "methodish";
    "attribute";
    "modifiers";
    "function_decl_header";
    "function_body";
    "semicolon" ];
  [ "ClassishDeclaration";
    "classish_declaration";
    "classish_declaration";
    "classish";
    "attribute";
    "modifiers";
    "keyword";
    "name";
    "type_parameters";
    "extends_keyword";
    "extends_list";
    "implements_keyword";
    "implements_list";
    "body" ];
  [ "ClassishBody";
    "classish_body";
    "classish_body";
    "classish_body";
    "left_brace";
    "elements";
    "right_brace" ];
  [ "TraitUse";
    "trait_use";
    "trait_use";
    "trait_use";
    "keyword";
    "names";
    "semicolon" ];
  [ "RequireClause";
    "require_clause";
    "require_clause";
    "require";
    "keyword";
    "kind";
    "name";
    "semicolon" ];
  [ "ConstDeclaration";
    "const_declaration";
    "const_declaration";
    "const";
    "abstract";
    "keyword";
    "type_specifier";
    "declarators";
    "semicolon" ];
  [ "ConstantDeclarator";
    "constant_declarator";
    "constant_declarator";
    "constant_declarator";
    "name";
    "initializer" ];
  [ "TypeConstDeclaration";
    "type_const_declaration";
    "type_const_declaration";
    "type_const";
    "abstract";
    "keyword";
    "type_keyword";
    "name";
    "type_constraint";
    "equal";
    "type_specifier";
    "semicolon" ];
  [ "DecoratedExpression";
    "decorated_expression";
    "decorated_expression";
    "decorated_expression";
    "decorator";
    "expression" ];
  [ "ParameterDeclaration";
    "parameter_declaration";
    "parameter_declaration";
    "parameter";
    "attribute";
    "visibility";
    "type";
    "name";
    "default_value" ];
  [ "VariadicParameter";
    "variadic_parameter";
    "variadic_parameter";
    "variadic_parameter";
    "ellipsis" ];
  [ "AttributeSpecification";
    "attribute_specification";
    "attribute_specification";
    "attribute_specification";
    "left_double_angle";
    "attributes";
    "right_double_angle" ];
  [ "Attribute";
    "attribute";
    "attribute";
    "attribute";
    "name";
    "left_paren";
    "values";
    "right_paren" ];
  [ "InclusionExpression";
    "inclusion_expression";
    "inclusion_expression";
    "inclusion";
    "require";
    "filename" ];
  [ "InclusionDirective";
    "inclusion_directive";
    "inclusion_directive";
    "inclusion";
    "expression";
    "semicolon" ];
  [ "CompoundStatement";
    "compound_statement";
    "compound_statement";
    "compound";
    "left_brace";
    "statements";
    "right_brace" ];
  [ "ExpressionStatement";
    "expression_statement";
    "expression_statement";
    "expression_statement";
    "expression";
    "semicolon" ];
  [ "UnsetStatement";
    "unset_statement";
    "unset_statement";
    "unset";
    "keyword";
    "left_paren";
    "variables";
    "right_paren";
    "semicolon" ];
  [ "WhileStatement";
    "while_statement";
    "while_statement";
    "while";
    "keyword";
    "left_paren";
    "condition";
    "right_paren";
    "body" ];
  [ "IfStatement";
    "if_statement";
    "if_statement";
    "if";
    "keyword";
    "left_paren";
    "condition";
    "right_paren";
    "statement";
    "elseif_clauses";
    "else_clause" ];
  [ "ElseifClause";
    "elseif_clause";
    "elseif_clause";
    "elseif";
    "keyword";
    "left_paren";
    "condition";
    "right_paren";
    "statement" ];
  [ "ElseClause";
    "else_clause";
    "else_clause";
    "else";
    "keyword";
    "statement" ];
  [ "TryStatement";
    "try_statement";
    "try_statement";
    "try";
    "keyword";
    "compound_statement";
    "catch_clauses";
    "finally_clause" ];
  [ "CatchClause";
    "catch_clause";
    "catch_clause";
    "catch";
    "keyword";
    "left_paren";
    "type";
    "variable";
    "right_paren";
    "body" ];
  [ "FinallyClause";
    "finally_clause";
    "finally_clause";
    "finally";
    "keyword";
    "body" ];
  [ "DoStatement";
    "do_statement";
    "do_statement";
    "do";
    "keyword";
    "body";
    "while_keyword";
    "left_paren";
    "condition";
    "right_paren";
    "semicolon" ];
  [ "ForStatement";
    "for_statement";
    "for_statement";
    "for";
    "keyword";
    "left_paren";
    "initializer";
    "first_semicolon";
    "control";
    "second_semicolon";
    "end_of_loop";
    "right_paren";
    "body" ];
  [ "ForeachStatement";
    "foreach_statement";
    "foreach_statement";
    "foreach";
    "keyword";
    "left_paren";
    "collection";
    "await_keyword";
    "as";
    "key";
    "arrow";
    "value";
    "right_paren";
    "body" ];
  [ "SwitchStatement";
    "switch_statement";
    "switch_statement";
    "switch";
    "keyword";
    "left_paren";
    "expression";
    "right_paren";
    "left_brace";
    "sections";
    "right_brace"];
  [ "SwitchSection";
    "switch_section";
    "switch_section";
    "switch_section";
    "labels";
    "statements";
    "fallthrough" ];
  [ "SwitchFallthrough";
    "switch_fallthrough";
    "switch_fallthrough";
    "fallthrough";
    "keyword";
    "semicolon" ];
  [ "CaseLabel";
    "case_label";
    "case_label";
    "case";
    "keyword";
    "expression";
    "colon" ];
  [ "DefaultLabel";
    "default_label";
    "default_label";
    "default";
    "keyword";
    "colon" ];
  [ "ReturnStatement";
    "return_statement";
    "return_statement";
    "return";
    "keyword";
    "expression";
    "semicolon" ];
  [ "ThrowStatement";
    "throw_statement";
    "throw_statement";
    "throw";
    "keyword";
    "expression";
    "semicolon" ];
  [ "BreakStatement";
    "break_statement";
    "break_statement";
    "break";
    "keyword";
    "level";
    "semicolon" ];
  [ "ContinueStatement";
    "continue_statement";
    "continue_statement";
    "continue";
    "keyword";
    "level";
    "semicolon" ];
  [ "FunctionStaticStatement";
    "function_static_statement";
    "function_static_statement";
    "static";
    "static_keyword";
    "declarations";
    "semicolon" ];
  [ "StaticDeclarator";
    "static_declarator";
    "static_declarator";
    "static";
    "name";
    "initializer" ];
  [ "EchoStatement";
    "echo_statement";
    "echo_statement";
    "echo";
    "keyword";
    "expressions";
    "semicolon" ];
  [ "GlobalStatement";
    "global_statement";
    "global_statement";
    "global";
    "keyword";
    "variables";
    "semicolon" ];
  [ "SimpleInitializer";
    "simple_initializer";
    "simple_initializer";
    "simple_initializer";
    "equal";
    "value" ];
  [ "AnonymousFunction";
    "anonymous_function";
    "anonymous_function";
    "anonymous";
    "async_keyword";
    "function_keyword";
    "left_paren";
    "parameters";
    "right_paren";
    "colon";
    "type";
    "use";
    "body" ];
  [ "AnonymousFunctionUseClause";
    "anonymous_function_use_clause";
    "anonymous_function_use_clause";
    "anonymous_use";
    "keyword";
    "left_paren";
    "variables";
    "right_paren" ];
  [ "LambdaExpression";
    "lambda_expression";
    "lambda_expression";
    "lambda";
    "async";
    "signature";
    "arrow";
    "body" ];
  [ "LambdaSignature";
    "lambda_signature";
    "lambda_signature";
    "lambda";
    "left_paren";
    "parameters";
    "right_paren";
    "colon";
    "type" ];
  [ "CastExpression";
    "cast_expression";
    "cast_expression";
    "cast";
    "left_paren";
    "type";
    "right_paren";
    "operand" ];
  [ "ScopeResolutionExpression";
    "scope_resolution_expression";
    "scope_resolution_expression";
    "scope_resolution";
    "qualifier";
    "operator";
    "name" ];
  [ "MemberSelectionExpression";
    "member_selection_expression";
    "member_selection_expression";
    "member";
    "object";
    "operator";
    "name" ];
  [ "SafeMemberSelectionExpression";
    "safe_member_selection_expression";
    "safe_member_selection_expression";
    "safe_member";
    "object";
    "operator";
    "name" ];
  [ "YieldExpression";
    "yield_expression";
    "yield_expression";
    "yield";
    "keyword";
    "operand" ];
  [ "PrintExpression";
    "print_expression";
    "print_expression";
    "print";
    "keyword";
    "expression" ];
  [ "PrefixUnaryExpression";
    "prefix_unary_expression";
    "prefix_unary_expression";
    "prefix_unary";
    "operator";
    "operand" ];
  [ "PostfixUnaryExpression";
    "postfix_unary_expression";
    "postfix_unary_expression";
    "postfix_unary";
    "operand";
    "operator" ];
  [ "BinaryExpression";
    "binary_expression";
    "binary_expression";
    "binary";
    "left_operand";
    "operator";
    "right_operand" ];
  [ "InstanceofExpression";
    "instanceof_expression";
    "instanceof_expression";
    "instanceof";
    "left_operand";
    "operator";
    "right_operand" ];
  [ "ConditionalExpression";
    "conditional_expression";
    "conditional_expression";
    "conditional";
    "test";
    "question";
    "consequence";
    "colon";
    "alternative" ];
  [ "EvalExpression";
    "eval_expression";
    "eval_expression";
    "eval";
    "keyword";
    "left_paren";
    "argument";
    "right_paren" ];
  [ "EmptyExpression";
    "empty_expression";
    "empty_expression";
    "empty";
    "keyword";
    "left_paren";
    "argument";
    "right_paren" ];
  [ "DefineExpression";
    "define_expression";
    "define_expression";
    "define";
    "keyword";
    "left_paren";
    "argument_list";
    "right_paren" ];
  [ "IssetExpression";
    "isset_expression";
    "isset_expression";
    "isset";
    "keyword";
    "left_paren";
    "argument_list";
    "right_paren" ];
  [ "FunctionCallExpression";
    "function_call_expression";
    "function_call_expression";
    "function_call";
    "receiver";
    "left_paren";
    "argument_list";
    "right_paren" ];
  [ "ParenthesizedExpression";
    "parenthesized_expression";
    "parenthesized_expression";
    "parenthesized_expression";
    "left_paren";
    "expression";
    "right_paren" ];
  [ "BracedExpression";
    "braced_expression";
    "braced_expression";
    "braced_expression";
    "left_brace";
    "expression";
    "right_brace" ];
  [ "ListExpression";
    "list_expression";
    "list_expression";
    "list";
    "keyword";
    "left_paren";
    "members";
    "right_paren" ];
  [ "CollectionLiteralExpression";
    "collection_literal_expression";
    "collection_literal_expression";
    "collection_literal";
    "name";
    "left_brace";
    "initializers";
    "right_brace" ];
  [ "ObjectCreationExpression";
    "object_creation_expression";
    "object_creation_expression";
    "object_creation";
    "new_keyword";
    "type";
    "left_paren";
    "argument_list";
    "right_paren" ];
  [ "ArrayCreationExpression";
    "array_creation_expression";
    "array_creation_expression";
    "array_creation";
    "left_bracket";
    "members";
    "right_bracket" ];
  [ "ArrayIntrinsicExpression";
    "array_intrinsic_expression";
    "array_intrinsic_expression";
    "array_intrinsic";
    "keyword";
    "left_paren";
    "members";
    "right_paren" ];
  [ "DictionaryIntrinsicExpression";
    "dictionary_intrinsic_expression";
    "dictionary_intrinsic_expression";
    "dictionary_intrinsic";
    "keyword";
    "left_bracket";
    "members";
    "right_bracket" ];
  [ "KeysetIntrinsicExpression";
    "keyset_intrinsic_expression";
    "keyset_intrinsic_expression";
    "keyset_intrinsic";
    "keyword";
    "left_bracket";
    "members";
    "right_bracket" ];
  [ "VectorIntrinsicExpression";
    "vector_intrinsic_expression";
    "vector_intrinsic_expression";
    "vector_intrinsic";
    "keyword";
    "left_bracket";
    "members";
    "right_bracket" ];
  [ "ElementInitializer";
    "element_initializer";
    "element_initializer";
    "element";
    "key";
    "arrow";
    "value" ];
  [ "SubscriptExpression";
    "subscript_expression";
    "subscript_expression";
    "subscript";
    "receiver";
    "left_bracket";
    "index";
    "right_bracket" ];
  [ "AwaitableCreationExpression";
    "awaitable_creation_expression";
    "awaitable_creation_expression";
    "awaitable";
    "async";
    "compound_statement" ];
  [ "XHPChildrenDeclaration";
    "xhp_children_declaration";
    "xhp_children_declaration";
    "xhp_children";
    "keyword";
    "expression";
    "semicolon" ];
  [ "XHPCategoryDeclaration";
    "xhp_category_declaration";
    "xhp_category_declaration";
    "xhp_category";
    "keyword";
    "categories";
    "semicolon" ];
  [ "XHPEnumType";
    "xhp_enum_type";
    "xhp_enum_type";
    "xhp_enum";
    "keyword";
    "left_brace";
    "values";
    "right_brace" ];
  [ "XHPRequired";
    "xhp_required";
    "xhp_required";
    "xhp_required";
    "at";
    "keyword" ];
  [ "XHPClassAttributeDeclaration";
    "xhp_class_attribute_declaration";
    "xhp_class_attribute_declaration";
    "xhp_attribute";
    "keyword";
    "attributes";
    "semicolon" ];
  [ "XHPClassAttribute";
    "xhp_class_attribute";
    "xhp_class_attribute";
    "xhp_attribute_decl";
    "type";
    "name";
    "initializer";
    "required" ];
  [ "XHPSimpleClassAttribute";
    "xhp_simple_class_attribute";
    "xhp_simple_class_attribute";
    "xhp_simple_class_attribute";
    "type" ];
  [ "XHPAttribute";
    "xhp_attribute";
    "xhp_attribute";
    "xhp_attribute";
    "name";
    "equal";
    "expression" ];
  [ "XHPOpen";
    "xhp_open";
    "xhp_open";
    "xhp_open";
    "name";
    "attributes";
    "right_angle" ];
  [ "XHPExpression";
    "xhp_expression";
    "xhp_expression";
    "xhp";
    "open";
    "body";
    "close" ];
  [ "XHPClose";
    "xhp_close";
    "xhp_close";
    "xhp_close";
    "left_angle";
    "name";
    "right_angle" ];
  [ "TypeConstant";
    "type_constant";
    "type_constant";
    "type_constant";
    "left_type";
    "separator";
    "right_type" ];
  [ "VectorTypeSpecifier";
    "vector_type_specifier";
    "vector_type_specifier";
    "vector_type";
    "keyword";
    "left_angle";
    "type";
    "right_angle" ];
  [ "KeysetTypeSpecifier";
    "keyset_type_specifier";
    "keyset_type_specifier";
    "keyset_type";
    "keyword";
    "left_angle";
    "type";
    "right_angle" ];
  [ "VectorArrayTypeSpecifier";
    "vector_array_type_specifier";
    "vector_array_type_specifier";
    "vector_array";
    "keyword";
    "left_angle";
    "type";
    "right_angle" ];
  [ "TypeParameter";
    "type_parameter";
    "type_parameter";
    "type";
    "variance";
    "name";
    "constraints" ];
  [ "TypeConstraint";
    "type_constraint";
    "type_constraint";
    "constraint";
    "keyword";
    "type" ];
  [ "MapArrayTypeSpecifier";
    "map_array_type_specifier";
    "map_array_type_specifier";
    "map_array";
    "keyword";
    "left_angle";
    "key";
    "comma";
    "value";
    "right_angle" ];
  [ "DictionaryTypeSpecifier";
    "dictionary_type_specifier";
    "dictionary_type_specifier";
    "dictionary_type";
    "keyword";
    "left_angle";
    "members";
    "right_angle" ];
  [ "ClosureTypeSpecifier";
    "closure_type_specifier";
    "closure_type_specifier";
    "closure";
    "outer_left_paren";
    "function_keyword";
    "inner_left_paren";
    "parameter_types";
    "inner_right_paren";
    "colon";
    "return_type";
    "outer_right_paren" ];
  [ "ClassnameTypeSpecifier";
    "classname_type_specifier";
    "classname_type_specifier";
    "classname";
    "keyword";
    "left_angle";
    "type";
    "right_angle" ];
  [ "FieldSpecifier";
    "field_specifier";
    "field_specifier";
    "field";
    "name";
    "arrow";
    "type" ];
  [ "FieldInitializer";
    "field_initializer";
    "field_initializer";
    "field_initializer";
    "name";
    "arrow";
    "value" ];
  [ "ShapeTypeSpecifier";
    "shape_type_specifier";
    "shape_type_specifier";
    "shape_type";
    "keyword";
    "left_paren";
    "fields";
    "right_paren" ];
  [ "ShapeExpression";
    "shape_expression";
    "shape_expression";
    "shape_expression";
    "keyword";
    "left_paren";
    "fields";
    "right_paren" ];
  [ "TupleExpression";
    "tuple_expression";
    "tuple_expression";
    "tuple_expression";
    "keyword";
    "left_paren";
    "items";
    "right_paren" ];
  [ "GenericTypeSpecifier";
    "generic_type_specifier";
    "generic_type_specifier";
    "generic";
    "class_type";
    "argument_list" ];
  [ "NullableTypeSpecifier";
    "nullable_type_specifier";
    "nullable_type_specifier";
    "nullable";
    "question";
    "type" ];
  [ "SoftTypeSpecifier";
    "soft_type_specifier";
    "soft_type_specifier";
    "soft";
    "at";
    "type" ];
  [ "TypeArguments";
    "type_arguments";
    "type_arguments";
    "type_arguments";
    "left_angle";
    "types";
    "right_angle" ];
  [ "TypeParameters";
    "type_parameters";
    "type_parameters";
    "type_parameters";
    "left_angle";
    "parameters";
    "right_angle" ];
  [ "TupleTypeSpecifier";
    "tuple_type_specifier";
    "tuple_type_specifier";
    "tuple";
    "left_paren";
    "types";
    "right_paren" ];
  [ "ErrorSyntax";
    "error";
    "error";
    "error";
    "error" ];
  [ "ListItem";
    "list_item";
    "list_item";
    "list";
    "item";
    "separator" ]]

let variable_text_tokens = List.map token_node_from_list [
  [ "ErrorToken"; "error_token" ];
  [ "Name"; "name" ];
  [ "QualifiedName"; "qualified_name" ];
  [ "Variable"; "variable" ];
  [ "NamespacePrefix"; "namespace_prefix" ];
  [ "DecimalLiteral"; "decimal_literal" ];
  [ "OctalLiteral"; "octal_literal" ];
  [ "HexadecimalLiteral"; "hexadecimal_literal" ];
  [ "BinaryLiteral"; "binary_literal" ];
  [ "FloatingLiteral"; "floating_literal" ];
  [ "ExecutionString"; "execution_string" ];
  [ "SingleQuotedStringLiteral"; "single_quoted_string_literal" ];
  [ "DoubleQuotedStringLiteral"; "double_quoted_string_literal" ];
  [ "DoubleQuotedStringLiteralHead"; "double_quoted_string_literal_head" ];
  [ "StringLiteralBody"; "string_literal_body" ];
  [ "DoubleQuotedStringLiteralTail"; "double_quoted_string_literal_tail" ];
  [ "HeredocStringLiteral"; "heredoc_string_literal" ];
  [ "HeredocStringLiteralHead"; "heredoc_string_literal_head" ];
  [ "HeredocStringLiteralTail"; "heredoc_string_literal_tail" ];
  [ "NowdocStringLiteral"; "nowdoc_string_literal" ];
  [ "BooleanLiteral"; "boolean_literal" ];
  [ "XHPCategoryName"; "XHP_category_name" ];
  [ "XHPElementName"; "XHP_element_name" ];
  [ "XHPClassName"; "XHP_class_name" ];
  [ "XHPStringLiteral"; "XHP_string_literal" ];
  [ "XHPBody"; "XHP_body" ];
  [ "XHPComment"; "XHP_comment" ]]

let no_text_tokens = List.map token_node_from_list [
  [ "EndOfFile"; "end_of_file" ]]

let given_text_tokens = List.map token_node_from_list [
  [ "Abstract"; "abstract" ];
  [ "And"; "and" ];
  [ "Array"; "array" ];
  [ "Arraykey"; "arraykey" ];
  [ "As"; "as" ];
  [ "Async"; "async" ];
  [ "Attribute"; "attribute" ];
  [ "Await"; "await" ];
  [ "Bool"; "bool" ];
  [ "Break"; "break" ];
  [ "Case"; "case" ];
  [ "Catch"; "catch" ];
  [ "Category"; "category" ];
  [ "Children"; "children" ];
  [ "Class"; "class" ];
  [ "Classname"; "classname" ];
  [ "Clone"; "clone" ];
  [ "Const"; "const" ];
  [ "Construct"; "__construct" ];
  [ "Continue"; "continue" ];
  [ "Default"; "default" ];
  [ "Define"; "define"];
  [ "Destruct"; "__destruct" ];
  [ "Dict"; "dict" ];
  [ "Do"; "do" ];
  [ "Double"; "double" ];
  [ "Echo"; "echo" ];
  [ "Else"; "else" ];
  [ "Elseif"; "elseif" ];
  [ "Empty"; "empty" ];
  [ "Enum"; "enum" ];
  [ "Eval"; "eval" ];
  [ "Extends"; "extends" ];
  [ "Fallthrough"; "fallthrough" ];
  [ "Float"; "float" ];
  [ "Final"; "final" ];
  [ "Finally"; "finally" ];
  [ "For"; "for" ];
  [ "Foreach"; "foreach" ];
  [ "Function"; "function" ];
  [ "Global"; "global" ];
  [ "If"; "if" ];
  [ "Implements"; "implements" ];
  [ "Include"; "include" ];
  [ "Include_once"; "include_once" ];
  [ "Instanceof"; "instanceof" ];
  [ "Insteadof"; "insteadof" ];
  [ "Int"; "int" ];
  [ "Interface"; "interface" ];
  [ "Isset"; "isset" ];
  [ "Keyset"; "keyset" ];
  [ "List"; "list" ];
  [ "Mixed"; "mixed" ];
  [ "Namespace"; "namespace" ];
  [ "New"; "new" ];
  [ "Newtype"; "newtype" ];
  [ "Noreturn"; "noreturn" ];
  [ "Num"; "num" ];
  [ "Object"; "object" ];
  [ "Or"; "or" ];
  [ "Parent"; "parent" ];
  [ "Print"; "print" ];
  [ "Private"; "private" ];
  [ "Protected"; "protected" ];
  [ "Public"; "public" ];
  [ "Require"; "require" ];
  [ "Require_once"; "require_once" ];
  [ "Required"; "required" ];
  [ "Resource"; "resource" ];
  [ "Return"; "return" ];
  [ "Self"; "self" ];
  [ "Shape"; "shape" ];
  [ "Static"; "static" ];
  [ "String"; "string" ];
  [ "Super"; "super" ];
  [ "Switch"; "switch" ];
  [ "This"; "this" ];
  [ "Throw"; "throw" ];
  [ "Trait"; "trait" ];
  [ "Try"; "try" ];
  [ "Tuple"; "tuple" ];
  [ "Type"; "type" ];
  [ "Unset"; "unset" ];
  [ "Use"; "use" ];
  [ "Var"; "var" ];
  [ "Vec"; "vec" ];
  [ "Void"; "void" ];
  [ "Where"; "where" ];
  [ "While"; "while" ];
  [ "Xor"; "xor" ];
  [ "Yield"; "yield" ];
  [ "LeftBracket"; "[" ];
  [ "RightBracket"; "]" ];
  [ "LeftParen"; "(" ];
  [ "RightParen"; ")" ];
  [ "LeftBrace"; "{" ];
  [ "RightBrace"; "}" ];
  [ "Dot"; "." ];
  [ "QuestionGreaterThan"; "?>" ];
  [ "MinusGreaterThan"; "->" ];
  [ "PlusPlus"; "++" ];
  [ "MinusMinus"; "--" ];
  [ "StarStar"; "**" ];
  [ "Star"; "*" ];
  [ "Plus"; "+" ];
  [ "Minus"; "-" ];
  [ "Tilde"; "~" ];
  [ "Exclamation"; "!" ];
  [ "Dollar"; "$" ];
  [ "Slash"; "/" ];
  [ "Percent"; "%" ];
  [ "LessThanGreaterThan"; "<>"];
  [ "LessThanEqualGreaterThan"; "<=>"];
  [ "LessThanLessThan"; "<<" ];
  [ "GreaterThanGreaterThan"; ">>" ];
  [ "LessThan"; "<" ];
  [ "GreaterThan"; ">" ];
  [ "LessThanEqual"; "<=" ];
  [ "GreaterThanEqual"; ">=" ];
  [ "EqualEqual"; "==" ];
  [ "EqualEqualEqual"; "===" ];
  [ "ExclamationEqual"; "!=" ];
  [ "ExclamationEqualEqual"; "!==" ];
  [ "Carat"; "^" ];
  [ "Bar"; "|" ];
  [ "Ampersand"; "&" ];
  [ "AmpersandAmpersand"; "&&" ];
  [ "BarBar"; "||" ];
  [ "Question"; "?" ];
  [ "QuestionQuestion"; "??" ];
  [ "Colon"; ":" ];
  [ "Semicolon"; ";" ];
  [ "Equal"; "=" ];
  [ "StarStarEqual"; "**=" ];
  [ "StarEqual"; "*=" ];
  [ "SlashEqual"; "/=" ];
  [ "PercentEqual"; "%=" ];
  [ "PlusEqual"; "+=" ];
  [ "MinusEqual"; "-=" ];
  [ "DotEqual"; ".=" ];
  [ "LessThanLessThanEqual"; "<<=" ];
  [ "GreaterThanGreaterThanEqual"; ">>=" ];
  [ "AmpersandEqual"; "&=" ];
  [ "CaratEqual"; "^=" ];
  [ "BarEqual"; "|=" ];
  [ "Comma"; "," ];
  [ "At"; "@" ];
  [ "ColonColon"; "::" ];
  [ "EqualGreaterThan"; "=>" ];
  [ "EqualEqualGreaterThan"; "==>" ];
  [ "QuestionMinusGreaterThan"; "?->" ];
  [ "DotDotDot"; "..." ];
  [ "DollarDollar"; "$$" ];
  [ "BarGreaterThan"; "|>" ];
  [ "NullLiteral"; "null" ];
  [ "SlashGreaterThan"; "/>" ];
  [ "LessThanSlash"; "</" ]]

let trivia_kinds = List.map trivia_node_from_list [
  [ "WhiteSpace"; "whitespace" ];
  [ "EndOfLine"; "end_of_line" ];
  [ "DelimitedComment"; "delimited_comment" ];
  [ "SingleLineComment"; "single_line_comment" ];
  [ "Unsafe"; "unsafe" ];
  [ "UnsafeExpression"; "unsafe_expression" ];
  [ "FixMe"; "fix_me" ];
  [ "IgnoreError"; "ignore_error" ];
  [ "FallThrough"; "fall_through" ]]

let map_and_concat_separated separator f items =
  String.concat separator (List.map f items)

let map_and_concat f items =
  map_and_concat_separated "" f items

let transform_schema f =
  map_and_concat f schema

let transform_tokens token_list f =
  map_and_concat f token_list

let transform_trivia trivia_list f =
  map_and_concat f trivia_list

let replace pattern new_text source =
  Str.replace_first (Str.regexp pattern) new_text source

let generate_string template =
  let syntax_folder s x =
    replace x.pattern (transform_schema x.func) s in
  let tokens_folder token_list s x =
    replace x.token_pattern (x.token_func token_list) s in
  let trivia_folder trivia_list s x =
    replace x.trivia_pattern (x.trivia_func trivia_list) s in
  let result = List.fold_left
    syntax_folder template.template template.transformations in

  let result = List.fold_left (tokens_folder no_text_tokens)
    result template.token_no_text_transformations in
  let result = List.fold_left (tokens_folder variable_text_tokens)
    result template.token_variable_text_transformations in
  let result = List.fold_left (tokens_folder given_text_tokens)
    result template.token_given_text_transformations in
  let result = List.fold_left (trivia_folder trivia_kinds)
    result template.trivia_transformations in

  result

let generate_file template =
  let file = open_out template.filename in
  let s = generate_string template in
  Printf.fprintf file "%s" s;
  close_out file


module GenerateFFJSONSchema = struct
  let to_json_trivia { trivia_kind; trivia_text } =
    Printf.sprintf
"    { \"trivia_kind_name\" : \"%s\",
      \"trivia_type_name\" : \"%s\" }"
    trivia_kind trivia_text

  let to_json_given_text x =
    Printf.sprintf
"    { \"token_kind\" : \"%s\",
      \"token_text\" : \"%s\" },
"
    x.token_kind x.token_text

  let to_json_variable_text x =
    Printf.sprintf
"    { \"token_kind\" : \"%s\",
      \"token_text\" : null },
"
    x.token_kind

  let to_json_ast_nodes x =
    let mapper f = Printf.sprintf
"{ \"field_name\" : \"%s\" }" f in
    let fields = String.concat ",\n        " (List.map mapper x.fields) in
    Printf.sprintf
"    { \"kind_name\" : \"%s\",
      \"type_name\" : \"%s\",
      \"description\" : \"%s\",
      \"prefix\" : \"%s\",
      \"fields\" : [
        %s
      ] },
"  x.kind_name x.type_name x.description x.prefix fields

  let full_fidelity_json_schema_template =
"{ \"description\" :
  \"Auto-generated JSON schema of the Hack Full Fidelity Parser AST\",
  \"version\" : \"" ^ full_fidelity_schema_version_number ^ "\",
  \"trivia\" : [
TRIVIA_KINDS ],
  \"tokens\" : [
GIVEN_TEXT_TOKENS
VARIABLE_TEXT_TOKENS
    { \"token_kind\" : \"EndOfFile\",
      \"token_text\" : null } ],
  \"AST\" : [
AST_NODES
    { \"kind_name\" : \"Token\",
      \"type_name\" : \"token\",
      \"description\" : \"token\",
      \"prefix\" : \"\",
      \"fields\" : [
        { \"field_name\" : \"leading\" },
        { \"field_name\" : \"trailing\" } ] },
    { \"kind_name\" : \"Missing\",
      \"type_name\" : \"missing\",
      \"description\" : \"missing\",
      \"prefix\" : \"\",
      \"fields\" : [ ] },
    { \"kind_name\" : \"SyntaxList\",
      \"type_name\" : \"syntax_list\",
      \"description\" : \"syntax_list\",
      \"prefix\" : \"\",
      \"fields\" : [ ] } ] }"

  let full_fidelity_json_schema =
  {
    filename = "hphp/hack/src/full_fidelity/js/full_fidelity_schema.json";
    template = full_fidelity_json_schema_template;
    transformations = [
      { pattern = "AST_NODES"; func = to_json_ast_nodes }
    ];
    token_no_text_transformations = [ ];
    token_given_text_transformations = [
      { token_pattern = "GIVEN_TEXT_TOKENS";
        token_func = map_and_concat to_json_given_text } ];
    token_variable_text_transformations = [
      { token_pattern = "VARIABLE_TEXT_TOKENS";
        token_func = map_and_concat to_json_variable_text }];
    trivia_transformations = [
      { trivia_pattern = "TRIVIA_KINDS";
        trivia_func = map_and_concat_separated ",\n" to_json_trivia }
    ]
  }

end (* GenerateFFJSONSchema *)

let schema_as_json () =
  generate_string GenerateFFJSONSchema.full_fidelity_json_schema
