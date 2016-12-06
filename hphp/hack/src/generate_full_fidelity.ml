(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type schema_node = {
  kind_name : string;
  type_name : string;
  description : string;
  prefix : string;
  fields : string list
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
  token_func : token_node -> string
}

type template_file =
{
  filename : string;
  template : string;
  transformations : transformation list;
  token_no_text_transformations : token_transformation list;
  token_variable_text_transformations : token_transformation list;
  token_given_text_transformations : token_transformation list;
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

let schema = List.map from_list [
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
    "type" ];
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
    "body" ];
  [ "CaseStatement";
    "case_statement";
    "case_statement";
    "case";
    "keyword";
    "expression";
    "colon";
    "statement" ];
  [ "DefaultStatement";
    "default_statement";
    "default_statement";
    "default";
    "keyword";
    "colon";
    "statement" ];
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
    "vector";
    "array";
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
  [ "MapTypeSpecifier";
    "map_type_specifier";
    "map_type_specifier";
    "map";
    "array";
    "left_angle";
    "key";
    "comma";
    "value";
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
  [ "SingleQuotedStringLiteral"; "single_quoted_string_literal" ];
  [ "DoubleQuotedStringLiteral"; "double_quoted_string_literal" ];
  [ "DoubleQuotedStringLiteralHead"; "double_quoted_string_literal_head" ];
  [ "DoubleQuotedStringLiteralBody"; "double_quoted_string_literal_body" ];
  [ "DoubleQuotedStringLiteralTail"; "double_quoted_string_literal_tail" ];
  [ "HeredocStringLiteral"; "heredoc_string_literal" ];
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
  [ "Destruct"; "__destruct" ];
  [ "Do"; "do" ];
  [ "Double"; "double" ];
  [ "Echo"; "echo" ];
  [ "Else"; "else" ];
  [ "Elseif"; "elseif" ];
  [ "Empty"; "empty" ];
  [ "Enum"; "enum" ];
  [ "Extends"; "extends" ];
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
  [ "Void"; "void" ];
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

let map_and_concat_separated separator f items =
  String.concat separator (List.map f items)

let map_and_concat f items =
  map_and_concat_separated "" f items

let transform_schema f =
  map_and_concat f schema

let transform_tokens token_list f =
  map_and_concat f token_list

let replace pattern new_text source =
  Str.replace_first (Str.regexp pattern) new_text source

let generate_string template =
  let syntax_folder s x =
    replace x.pattern (transform_schema x.func) s in
  let tokens_folder token_list s x =
    replace x.token_pattern (transform_tokens token_list x.token_func) s in
  let result = List.fold_left
    syntax_folder template.template template.transformations in
  let result = List.fold_left (tokens_folder no_text_tokens)
    result template.token_no_text_transformations in
  let result = List.fold_left (tokens_folder variable_text_tokens)
    result template.token_variable_text_transformations in
  let result = List.fold_left (tokens_folder given_text_tokens)
    result template.token_given_text_transformations in
  result

let generate_file template =
  let file = open_out template.filename in
  let s = generate_string template in
  Printf.fprintf file "%s" s;
  close_out file

module GenerateFFSyntax = struct

  let to_parse_tree x =
    let mapper f = Printf.sprintf "      %s_%s: t;\n" x.prefix f in
    let fields = map_and_concat mapper x.fields in
    Printf.sprintf "    and %s = {\n%s    }\n"
      x.type_name fields

  let to_syntax x =
    Printf.sprintf "    | %s of %s\n"
      x.kind_name x.type_name

  let to_to_kind x =
    Printf.sprintf "      | %s _ ->\n        SyntaxKind.%s\n"
      x.kind_name x.kind_name

  let to_type_tests x =
    Printf.sprintf "    let is_%s node =\n      kind node = SyntaxKind.%s\n"
      x.type_name x.kind_name

  let to_child_list_from_type x =
    let mapper1 f = Printf.sprintf "      %s_%s;\n" x.prefix f in
    let mapper2 f = Printf.sprintf "      %s_%s" x.prefix f in
    let fields1 = map_and_concat mapper1 x.fields in
    let fields2 = String.concat ",\n" (List.map mapper2 x.fields) in
    Printf.sprintf "    let get_%s_children {\n%s    } = (\n%s\n    )\n\n"
      x.type_name fields1 fields2

  let to_children x =
    let mapper f = Printf.sprintf "        %s_%s;\n" x.prefix f in
    let fields = map_and_concat mapper x.fields in
    Printf.sprintf "      | %s {\n%s      } -> [\n%s      ]\n"
      x.kind_name fields fields

  let to_children_names x =
    let mapper1 f = Printf.sprintf "        %s_%s;\n" x.prefix f in
    let mapper2 f = Printf.sprintf "        \"%s_%s\";\n" x.prefix f in
    let fields1 = map_and_concat mapper1 x.fields in
    let fields2 = map_and_concat mapper2 x.fields in
    Printf.sprintf "      | %s {\n%s      } -> [\n%s      ]\n"
      x.kind_name fields1 fields2

  let to_syntax_from_children x =
    let mapper f = Printf.sprintf "          %s_%s;\n" x.prefix f in
    let fields = map_and_concat mapper x.fields in
    Printf.sprintf "      | (SyntaxKind.%s, [
%s        ]) ->
        %s {
%s        }
"
      x.kind_name fields x.kind_name fields

  let to_constructor_methods x =
    let mapper1 f = Printf.sprintf "      %s_%s\n" x.prefix f in
    let mapper2 f = Printf.sprintf "        %s_%s;\n" x.prefix f in
    let fields1 = map_and_concat mapper1 x.fields in
    let fields2 = map_and_concat mapper2 x.fields in
    Printf.sprintf "    let make_%s
%s    =
      from_children SyntaxKind.%s [
%s      ]

"
      x.type_name fields1 x.kind_name fields2

  let full_fidelity_syntax_template =
"(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the \"hack\" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 *)
(* THIS FILE IS GENERATED; DO NOT EDIT IT *)
(* @" ^ "generated *)
(**
  To regenerate this file build hphp/hack/src:generate_full_fidelity and run
  the binary.
  buck build hphp/hack/src:generate_full_fidelity
  buck-out/bin/hphp/hack/src/generate_full_fidelity/generate_full_fidelity.opt
*)
(**
 * This module contains the code describing the structure of a syntax tree.
 *
 * The relationships between the various functors and signatures here needs
 * some explanation.
 *
 * First off, the structure of the syntax tree is described by the collection
 * of recursive types that makes up the bulk of this file. The type \"t\" is
 * the type of a node in the syntax tree; each node has associated with it
 * an arbitrary value of type SyntaxValue.t, and syntax node proper, which
 * has structure given by the \"syntax\" type.
 *
 * Note that every child in the syntax tree is of type t, except for the
 * \"Token\" type. This should be the *only* child of a type other than t.
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

    type t = {
      syntax : syntax ;
      value : SyntaxValue.t
    }
PARSE_TREE
    and syntax =
    | Token of Token.t
    | Missing
    | SyntaxList of t list
SYNTAX

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
      | SyntaxList _ -> SyntaxKind.SyntaxList
TO_KIND

    let kind node =
      to_kind (syntax node)

    let is_missing node =
      kind node = SyntaxKind.Missing

    let is_list node =
      kind node = SyntaxKind.SyntaxList

TYPE_TESTS

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

CHILD_LIST_FROM_TYPE

    let children node =
      match node.syntax with
      | Missing -> []
      | Token _ -> []
      | SyntaxList x -> x
CHILDREN

    let children_names node =
      match node.syntax with
      | Missing -> []
      | Token _ -> []
      | SyntaxList _ -> []
CHILDREN_NAMES

    let rec to_json node =
      let open Hh_json in
      let ch = match node.syntax with
      | Token t -> [ \"token\", Token.to_json t ]
      | SyntaxList x -> [ (\"elements\", JSON_Array (List.map to_json x)) ]
      | _ ->
        let rec aux acc c n =
          match c, n with
          | ([], []) -> acc
          | ((hc :: tc), (hn :: tn)) ->
            aux ((hn, to_json hc) :: acc) tc tn
          | _ -> failwith \"mismatch between children and names\" in
        List.rev (aux [] (children node) (children_names node)) in
      let k = (\"kind\", JSON_String (SyntaxKind.to_string (kind node))) in
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
SYNTAX_FROM_CHILDREN      | (SyntaxKind.Missing, []) -> Missing
      | (SyntaxKind.SyntaxList, items) -> SyntaxList items
      | _ -> failwith
        \"syntax_from_children called with wrong number of children\"

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

      let make_list items =
        match items with
        | [] -> make_missing()
        | h :: [] -> h
        | _ -> from_children SyntaxKind.SyntaxList items

CONSTRUCTOR_METHODS

    end (* WithValueBuilder *)
  end (* WithSyntaxValue *)
end (* WithToken *)
"

  let full_fidelity_syntax =
  {
    filename = "hphp/hack/src/full_fidelity/full_fidelity_syntax.ml";
    template = full_fidelity_syntax_template;
    transformations = [
      { pattern = "PARSE_TREE"; func = to_parse_tree };
      { pattern = "SYNTAX"; func = to_syntax };
      { pattern = "TO_KIND"; func = to_to_kind };
      { pattern = "TYPE_TESTS"; func = to_type_tests };
      { pattern = "CHILD_LIST_FROM_TYPE"; func = to_child_list_from_type };
      { pattern = "CHILDREN"; func = to_children };
      { pattern = "CHILDREN_NAMES"; func = to_children_names };
      { pattern = "SYNTAX_FROM_CHILDREN"; func = to_syntax_from_children };
      { pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods };
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = []
  }

end (* GenerateFFSyntax *)

module GenerateFFSyntaxKind = struct

  let to_tokens x =
    Printf.sprintf "| %s\n" x.kind_name

  let to_to_string x =
    Printf.sprintf "  | %s -> \"%s\"\n" x.kind_name x.description

  let full_fidelity_syntax_kind_template =
"(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the \"hack\" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 *)
(* THIS FILE IS GENERATED; DO NOT EDIT IT *)
(* @" ^ "generated *)
(**
  To regenerate this file build hphp/hack/src:generate_full_fidelity and run
  the binary.
  buck build hphp/hack/src:generate_full_fidelity
  buck-out/bin/hphp/hack/src/generate_full_fidelity/generate_full_fidelity.opt
*)
type t =
| Token
| Missing
| SyntaxList
TOKENS

let to_string kind =
  match kind with
  | Missing -> \"missing\"
  | Token -> \"token\"
  | SyntaxList -> \"list\"
TO_STRING"

let full_fidelity_syntax_kind =
{
  filename = "hphp/hack/src/full_fidelity/full_fidelity_syntax_kind.ml";
  template = full_fidelity_syntax_kind_template;
  transformations = [
    { pattern = "TOKENS"; func = to_tokens };
    { pattern = "TO_STRING"; func = to_to_string };
  ];
  token_no_text_transformations = [];
  token_given_text_transformations = [];
  token_variable_text_transformations = []
}

end (* GenerateFFSyntaxKind *)

module GenerateFFJavaScript = struct

  let to_from_json x =
    Printf.sprintf "    case '%s':
      return %s.from_json(json, position, source);
" x.description x.kind_name

  let to_editable_syntax x =
    let ctor_mapper f = f in
    let ctor = map_and_concat_separated ",\n    " ctor_mapper x.fields in
    let ctor2 = map_and_concat_separated ",\n        " ctor_mapper x.fields in
    let children_mapper f = Printf.sprintf "%s: %s" f f in
    let children =
      map_and_concat_separated ",\n      " children_mapper x.fields in
    let props_mapper f =
      Printf.sprintf "get %s() { return this.children.%s; }" f f in
    let props = map_and_concat_separated "\n  " props_mapper x.fields in
    let withs_mapper f =
      let inner_mapper f_inner =
        let prefix = if f_inner = f then "" else "this." in
        Printf.sprintf "%s%s" prefix f_inner in
      let inner = map_and_concat_separated ",\n      " inner_mapper x.fields in
      Printf.sprintf "with_%s(%s){\n    return new %s(\n      %s);\n  }"
        f f x.kind_name inner in
    let withs = map_and_concat_separated "\n  " withs_mapper x.fields in
    let rewriter_mapper f =
      Printf.sprintf "var %s = this.%s.rewrite(rewriter, new_parents);" f f in
    let rewriter =
      map_and_concat_separated "\n    " rewriter_mapper x.fields in
    let condition_mapper f = Printf.sprintf "%s === this.%s" f f in
    let condition =
      map_and_concat_separated " &&\n      " condition_mapper x.fields in
    let json_mapper f = Printf.sprintf
      "let %s = EditableSyntax.from_json(
      json.%s_%s, position, source);
    position += %s.width;" f x.prefix f f in
    let json = map_and_concat_separated "\n    " json_mapper x.fields in
    let keys_mapper f = Printf.sprintf "'%s'" f in
    let keys = map_and_concat_separated ",\n        " keys_mapper x.fields in
    Printf.sprintf
"class %s extends EditableSyntax
{
  constructor(
    %s)
  {
    super('%s', {
      %s });
  }
  %s
  %s
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    %s
    if (
      %s)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new %s(
        %s), parents);
    }
  }
  static from_json(json, position, source)
  {
    %s
    return new %s(
        %s);
  }
  get children_keys()
  {
    if (%s._children_keys == null)
      %s._children_keys = [
        %s];
    return %s._children_keys;
  }
}
" x.kind_name ctor x.description children props withs rewriter condition
  x.kind_name ctor2 json x.kind_name ctor2 x.kind_name x.kind_name keys
  x.kind_name

  let to_exports_syntax x =
    Printf.sprintf "exports.%s = %s;\n" x.kind_name x.kind_name

  let to_editable_no_text x =
    Printf.sprintf "class %sToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('%s', leading, trailing, '');
  }
}
" x.token_kind x.token_text

let to_editable_given_text x =
  Printf.sprintf "class %sToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('%s', leading, trailing, '%s');
  }
}
" x.token_kind x.token_text x.token_text

  let to_editable_variable_text x =
    Printf.sprintf "class %sToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('%s', leading, trailing, text);
  }
  with_text(text)
  {
    return new %sToken(this.leading, this.trailing, text);
  }

}
" x.token_kind x.token_text x.token_kind

  let to_factory_no_text x =
    Printf.sprintf
"    case '%s':
       return new %sToken(leading, trailing);
"
      x.token_text x.token_kind

  let to_factory_given_text = to_factory_no_text

  let to_factory_variable_text x =
    Printf.sprintf
"    case '%s':
       return new %sToken(leading, trailing, token_text);
"
      x.token_text x.token_kind

  let to_export_token x =
    Printf.sprintf "exports.%sToken = %sToken;\n" x.token_kind x.token_kind

  let full_fidelity_javascript_template =
"/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the 'hack' directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 */
/* THIS FILE IS GENERATED; DO NOT EDIT IT */
/* @" ^ "generated */
/**
  To regenerate this file build hphp/hack/src:generate_full_fidelity and run
  the binary.
  buck build hphp/hack/src:generate_full_fidelity
  buck-out/bin/hphp/hack/src/generate_full_fidelity/generate_full_fidelity.opt
*/

\"use strict\";

let utils = require('./full_fidelity_utils.js');
let array_map_reduce = utils.array_map_reduce;
let array_sum = utils.array_sum;

class EditableSyntax
{
  constructor(syntax_kind, children)
  {
    this._syntax_kind = syntax_kind;
    this._children = children;
    let width = 0;
    for(let child in children)
      width += children[child].width;
    this._children_width = width;
  }
  get syntax_kind() { return this._syntax_kind; }
  get children() { return this._children; }
  get is_token() { return false; }
  get is_trivia() { return false; }
  get is_list() { return false; }
  get is_missing() { return false; }

  get width() { return this._children_width; }

  get full_text()
  {
    let s = '';
    for(let key of this.children_keys)
      s += this.children[key].full_text;
    return s;
  }

  static from_json(json, position, source)
  {
    switch(json.kind)
    {
    case 'token':
      return EditableToken.from_json(json.token, position, source);
    case 'list':
      return EditableList.from_json(json, position, source);
    case 'whitespace':
      return Whitespace.from_json(json, position, source);
    case 'end_of_line':
      return EndOfLine.from_json(json, position, source);
    case 'delimited_comment':
      return DelimitedComment.from_json(json, position, source);
    case 'single_line_comment':
      return SingleLineComment.from_json(json, position, source);
    case 'missing':
      return Missing.missing;
FROM_JSON_SYNTAX
    default:
      throw 'unexpected json kind: ' + json.kind; // TODO: Better exception
    }
  }

  reduce(reducer, accumulator, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    for(let key of this.children_keys)
    {
      accumulator = this.children[key].reduce(
        reducer, accumulator, new_parents);
    }
    return reducer(this, accumulator, parents);
  }

  // Returns all the parents (and the node itself) of the first node
  // that matches a predicate, or [] if there is no such node.
  find(predicate, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    if (predicate(this))
      return new_parents;
    for(let key of this.children_keys)
    {
      let result = this.children[key].find(predicate, new_parents);
      if (result.length != 0)
        return result;
    }
    return [];
  }

  // Returns a list of nodes that match a predicate.
  filter(predicate)
  {
    let reducer = (node, acc, parents) => {
      if (predicate(node))
        acc.push(node);
      return acc;
    };
    return this.reduce(reducer, []);
  }

  of_syntax_kind(kind)
  {
    return this.filter((node) => node.syntax_kind == kind);
  }

  remove_where(predicate)
  {
    return this.rewrite(
      (node, parents) => predicate(node) ? Missing.missing : node);
  }

  without(target)
  {
    return this.remove_where((node) => node === target);
  }

  replace(new_node, target)
  {
    return this.rewrite((node, parents) => node === target ? new_node : node);
  }

  pre_order(action, parents)
  {
    if (parents == undefined)
      parents = [];
    action(this, parents);
    let new_parents = parents.slice();
    new_parents.push(this);
    for(let key of this.children_keys)
      this.children[key].pre_order(action, new_parents);
  }

  get leftmost_token()
  {
    if (this.is_token)
      return this;

    for(let key of this.children_keys)
    {
      if (!this.children[key].is_missing)
        return this.children[key].leftmost_token;
    }
    return null;
  }

  get rightmost_token()
  {
    if (this.is_token)
      return this;

    for (let i = this.children_keys.length - 1; i >= 0; i--)
    {
      if (!this.children[this.children_keys[i]].is_missing)
        return this.children[key].rightmost_token;
    }
    return null;
  }

  insert_before(new_node, target)
  {
    // Inserting before missing is an error.
    if (target.is_missing)
      throw 'Target must not be missing in insert_before.';

    // Inserting missing is a no-op
    if (new_node.is_missing)
      return this;

    if (new_node.is_trivia && !target.is_trivia)
    {
      let token = target.is_token ? target : target.leftmost_token;
      if (token == null)
        throw 'Unable to find token to insert trivia.';

      // Inserting trivia before token is inserting to the right end of
      // the leading trivia.
      let new_leading = EditableSyntax.concatenate_lists(
        token.leading, new_node);
      let new_token = token.with_leading(new_leading);
      return this.replace(new_token, token);
    }

    return this.replace(
      EditableSyntax.concatenate_lists(new_node, target), target);
  }

  insert_after(new_node, target)
  {
    // Inserting after missing is an error.
    if (target.is_missing)
      throw 'Target must not be missing in insert_after.';

    // Inserting missing is a no-op
    if (new_node.is_missing)
      return this;

    if (new_node.is_trivia && !target.is_trivia)
    {
      let token = target.is_token ? target : target.rightmost_token;
      if (token == null)
        throw 'Unable to find token to insert trivia.';

      // Inserting trivia after token is inserting to the left end of
      // the trailing trivia.
      let new_trailing = EditableSyntax.concatenate_lists(
        new_node, token.trailing);
      let new_token = token.with_trailing(new_trailing);
      return this.replace(new_token, token);
    }

    return this.replace(
      EditableSyntax.concatenate_lists(target, new_node), target);
  }

  static to_list(syntax_list)
  {
    if (syntax_list.length == 0)
      return Missing.missing;
    else if (syntax_list.length == 1)
      return syntax_list[0];
    else
      return new EditableList(syntax_list);
  }

  static concatenate_lists(left, right)
  {
    if (left.is_missing)
      return right;
    if (right.is_missing)
      return left;
    if (left.is_list && right.is_list)
      return new EditableList(left.children.concat(right.children));
    if (left.is_list)
      return new EditableList(left.children.splice().push(right));
    if (right.is_list)
      return new EditableList([right].concat(left.children));
    return new EditableList([left, right]);
  }
}

class EditableList extends EditableSyntax
{
  constructor(children)
  {
    super('list', children);
  }
  get is_list() { return true; }

  static from_json(json, position, source)
  {
    let children = [];
    let current_position = position;
    for(let element of json.elements)
    {
      let child = EditableSyntax.from_json(element, current_position, source);
      children.push(child);
      current_position += child.width;
    }
    return new EditableList(children);
  }

  rewrite(rewriter, parents)
  {
    let dirty = false;
    let new_children = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    for (let key of this.children_keys)
    {
      let child = this.children[key];
      let new_child = child.rewrite(rewriter, new_parents);
      if (new_child != child)
        dirty = true;
      if (new_child != null)
      {
        if (new_child.is_list)
        {
          for(let n of new_child.children)
            new_children.push(n);
        }
        else
          new_children.push(new_child);
      }
    }
    let result = this;
    if (dirty)
    {
      if (new_children.length === 0)
        result = Missing.missing;
      else if (new_children.length === 1)
        result = new_children[0];
      else
        result = new EditableList(new_children);
    }
    return rewriter(result, parents);
  }
  get children_keys()
  {
    return Object.keys(this.children);
  }
}

class EditableToken extends EditableSyntax
{
  constructor(token_kind, leading, trailing, text)
  {
    super('token', { leading : leading, trailing : trailing });
    this._token_kind = token_kind;
    this._text = text;
  }

  get token_kind() { return this._token_kind; }
  get text() { return this._text; }
  get leading() { return this.children.leading; }
  get trailing() { return this.children.trailing; }
  get width()
  {
    return this.text.length + this.leading.width + this.trailing.width;
  }
  get is_token() { return true; }
  get full_text()
  {
    return this.leading.full_text + this.text + this.trailing.full_text;
  }
  with_leading(leading)
  {
    return EditableToken.factory(
      this.token_kind, leading, this.trailing, this.text);
  }
  with_trailing(trailing)
  {
    return EditableToken.factory(
      this.token_kind, this.leading, trailing, this.text);
  }
  static factory(token_kind, leading, trailing, token_text)
  {
    switch(token_kind)
    {
FACTORY_NO_TEXT_TOKENS
FACTORY_GIVEN_TEXT_TOKENS
FACTORY_VARIABLE_TEXT_TOKENS
      default: throw 'unexpected token kind; ' + token_kind;
      // TODO: Better error
    }
  }

  rewrite(rewriter, parents)
  {
    let new_parents = parents.slice();
    new_parents.push(this);
    let leading = this.leading.rewrite(rewriter, new_parents);
    let trailing = this.trailing.rewrite(rewriter, new_parents);
    if (leading === this.leading && trailing === this.trailing)
      return rewriter(this, parents);
    else
      return rewriter(EditableToken.factory(
        this.token_kind, leading, trailing, this.text), parents);
  }

  reduce(reducer, accumulator)
  {
    accumulator = this.leading.reduce(reducer, accumulator);
    accumulator = reducer(this, accumulator);
    accumulator = this.trailing.reduce(reducer, accumulator);
    return accumulator;
  }

  static from_json(json, position, source)
  {
    let leading_list = array_map_reduce(
      json.leading,
      (json, position) => EditableSyntax.from_json(json, position, source),
      (json, position) => json.width + position,
      position);
    let leading = EditableSyntax.to_list(leading_list);
    let token_position = position + leading.width;
    let token_text = source.substring(
      token_position, token_position + json.width);
    let trailing_position = token_position + json.width;
    let trailing_list = array_map_reduce(
      json.trailing,
      (json, position) => EditableSyntax.from_json(json, position, source),
      (json, position) => json.width + position,
      trailing_position);
    let trailing = EditableSyntax.to_list(trailing_list);
    return EditableToken.factory(json.kind, leading, trailing, token_text);
  }

  get children_keys()
  {
    if (EditableToken._children_keys == null)
      EditableToken._children_keys = ['leading', 'trailing'];
    return EditableToken._children_keys;
  }
}

EDITABLE_NO_TEXT_TOKENS
EDITABLE_GIVEN_TEXT_TOKENS
EDITABLE_VARIABLE_TEXT_TOKENS

class EditableTrivia extends EditableSyntax
{
  constructor(trivia_kind, text)
  {
    super(trivia_kind, {});
    this._text = text;
  }
  get text() { return this._text; }
  get full_text() { return this.text; }
  get width() { return this.text.length; }
  get is_trivia() { return true; }

  static from_json(json, position, source)
  {
    let trivia_text = source.substring(position, position + json.width);
    switch(json.kind)
    {
      case 'whitespace': return new Whitespace(trivia_text);
      case 'end_of_line': return new EndOfLine(trivia_text);
      case 'single_line_comment': return new SingleLineComment(trivia_text);
      case 'delimited_comment': return new DelimitedComment(trivia_text);
      default: throw 'unexpected json kind: ' + json.kind; // TODO: Better error
    }
  }

  rewrite(rewriter, parents)
  {
    return rewriter(this, parents);
  }
  get children_keys()
  {
    return [];
  }
}

class Whitespace extends EditableTrivia
{
  constructor(text) { super('whitespace', text); }
  with_text(text)
  {
    return new Whitespace(text);
  }
}

class EndOfLine extends EditableTrivia
{
  constructor(text) { super('end_of_line', text); }
  with_text(text)
  {
    return new EndOfLine(text);
  }
}

class SingleLineComment extends EditableTrivia
{
  constructor(text) { super('single_line_comment', text); }
  with_text(text)
  {
    return new SingleLineComment(text);
  }
}

class DelimitedComment extends EditableTrivia
{
  constructor(text) { super('delimited_comment', text); }
  with_text(text)
  {
    return new DelimitedComment(text);
  }
}

class Missing extends EditableSyntax
{
  constructor()
  {
    super('missing', {});
  }
  get is_missing() { return true; }
  static get missing() { return Missing._missing; }
  static from_json(json, position, source)
  {
    return Missing._missing;
  }
  rewrite(rewriter, parents)
  {
    return rewriter(this, parents);
  }
  get children_keys()
  {
    return [];
  }
}
Missing._missing = new Missing();

EDITABLE_SYNTAX

function from_json(json)
{
  return EditableSyntax.from_json(json.parse_tree, 0, json.program_text);
}

exports.from_json = from_json;
exports.EditableSyntax = EditableSyntax;
exports.EditableList = EditableList;
exports.EditableToken = EditableToken;
EXPORTS_NO_TEXT_TOKENS
EXPORTS_GIVEN_TEXT_TOKENS
EXPORTS_VARIABLE_TEXT_TOKENS
exports.EditableTrivia = EditableTrivia;
exports.Whitespace = Whitespace;
exports.EndOfLine = EndOfLine;
exports.DelimitedComment = DelimitedComment;
exports.SingleLineComment = SingleLineComment;
EXPORTS_SYNTAX"

  let full_fidelity_javascript =
  {
    filename = "hphp/hack/src/full_fidelity/js/full_fidelity_editable.js";
    template = full_fidelity_javascript_template;
    transformations = [
      { pattern = "FROM_JSON_SYNTAX"; func = to_from_json };
      { pattern = "EDITABLE_SYNTAX"; func = to_editable_syntax };
      { pattern = "EXPORTS_SYNTAX"; func = to_exports_syntax };
    ];
    token_no_text_transformations = [
      { token_pattern = "EDITABLE_NO_TEXT_TOKENS";
        token_func = to_editable_no_text };
      { token_pattern = "FACTORY_NO_TEXT_TOKENS";
        token_func = to_factory_no_text };
      { token_pattern = "EXPORTS_NO_TEXT_TOKENS";
        token_func = to_export_token }];
    token_given_text_transformations = [
      { token_pattern = "EDITABLE_GIVEN_TEXT_TOKENS";
        token_func = to_editable_given_text };
      { token_pattern = "FACTORY_GIVEN_TEXT_TOKENS";
        token_func = to_factory_given_text };
      { token_pattern = "EXPORTS_GIVEN_TEXT_TOKENS";
        token_func = to_export_token }];
    token_variable_text_transformations = [
      { token_pattern = "EDITABLE_VARIABLE_TEXT_TOKENS";
        token_func = to_editable_variable_text };
      { token_pattern = "FACTORY_VARIABLE_TEXT_TOKENS";
        token_func = to_factory_variable_text };
      { token_pattern = "EXPORTS_VARIABLE_TEXT_TOKENS";
        token_func = to_export_token }]
  }

end (* GenerateFFJavaScript *)

module GenerateFFHack = struct

  let to_from_json x =
    Printf.sprintf "    case '%s':
      return %s::from_json($json, $position, $source);
" x.description x.kind_name

  let to_editable_syntax x =
    let ctor_mapper f = Printf.sprintf "EditableSyntax $%s" f in
    let ctor = map_and_concat_separated ",\n    " ctor_mapper x.fields in
    let ctor2_mapper f = Printf.sprintf "$%s" f in
    let ctor2 = map_and_concat_separated ",\n        " ctor2_mapper x.fields in
    let props_mapper f =
      Printf.sprintf "private EditableSyntax $_%s;" f in
    let props = map_and_concat_separated "\n  " props_mapper x.fields in
    let getters_mapper f =
      Printf.sprintf "public function %s(): EditableSyntax {
    return $this->_%s;
  }" f f in
    let getters = map_and_concat_separated "\n  " getters_mapper x.fields in

    let assignments_mapper f =
      Printf.sprintf "$this->_%s = $%s;" f f in
    let assignments = map_and_concat_separated
      "\n    " assignments_mapper x.fields in
    let withs_mapper f =
      let inner_mapper f_inner =
        let prefix = if f_inner = f then "$" else "$this->_" in
        Printf.sprintf "%s%s" prefix f_inner in
      let inner = map_and_concat_separated ",\n      " inner_mapper x.fields in
      Printf.sprintf "public function with_%s(EditableSyntax $%s): %s {
    return new %s(
      %s);
  }"
        f f x.kind_name x.kind_name inner in
    let withs = map_and_concat_separated "\n  " withs_mapper x.fields in
    let rewriter_mapper f =
      Printf.sprintf "$%s = $this->%s()->rewrite($rewriter, $new_parents);" f f in
    let rewriter =
      map_and_concat_separated "\n    " rewriter_mapper x.fields in
    let condition_mapper f = Printf.sprintf "$%s === $this->%s()" f f in
    let condition =
      map_and_concat_separated " &&\n      " condition_mapper x.fields in
    let json_mapper f = Printf.sprintf
      "$%s = EditableSyntax::from_json(
      $json->%s_%s, $position, $source);
    $position += $%s->width();" f x.prefix f f in
    let json = map_and_concat_separated "\n    " json_mapper x.fields in
    let children_mapper f = Printf.sprintf "yield $this->_%s;" f in
    let children =
      map_and_concat_separated "\n    " children_mapper x.fields in
    Printf.sprintf
"final class %s extends EditableSyntax {
  %s
  public function __construct(
    %s) {
    parent::__construct('%s');
    %s
  }
  %s
  %s

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    %s
    if (
      %s) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new %s(
        %s), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    %s
    return new %s(
        %s);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    %s
    yield break;
  }
}
" x.kind_name props ctor x.description assignments getters withs rewriter
  condition x.kind_name ctor2 json x.kind_name ctor2 children

  let to_editable_no_text x =
    Printf.sprintf "final class %sToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('%s', $leading, $trailing, '');
  }

  public function with_leading(EditableSyntax $leading): %sToken {
    return new %sToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): %sToken {
    return new %sToken($this->leading(), $trailing);
  }
}
" x.token_kind x.token_text x.token_kind x.token_kind x.token_kind x.token_kind

let to_editable_given_text x =
  Printf.sprintf "final class %sToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('%s', $leading, $trailing, '%s');
  }

  public function with_leading(EditableSyntax $leading): %sToken {
    return new %sToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): %sToken {
    return new %sToken($this->leading(), $trailing);
  }
}
" x.token_kind x.token_text x.token_text x.token_kind x.token_kind
  x.token_kind x.token_kind

  let to_editable_variable_text x =
    Printf.sprintf "final class %sToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('%s', $leading, $trailing, $text);
  }

  public function with_text(string $text): %sToken {
    return new %sToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): %sToken {
    return new %sToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): %sToken {
    return new %sToken($this->leading(), $trailing, $this->text());
  }
}
" x.token_kind x.token_text
  x.token_kind x.token_kind
  x.token_kind x.token_kind
  x.token_kind x.token_kind

  let to_factory_no_text x =
    Printf.sprintf
"    case '%s':
       return new %sToken($leading, $trailing);
"
      x.token_text x.token_kind

  let to_factory_given_text = to_factory_no_text

  let to_factory_variable_text x =
    Printf.sprintf
"    case '%s':
       return new %sToken($leading, $trailing, $token_text);
"
      x.token_text x.token_kind

  let full_fidelity_hack_template =
"<?hh // strict
/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the 'hack' directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 */
/* THIS FILE IS GENERATED; DO NOT EDIT IT */
/* @" ^ "generated */
/**
  To regenerate this file build hphp/hack/src:generate_full_fidelity and run
  the binary.
  buck build hphp/hack/src:generate_full_fidelity
  buck-out/bin/hphp/hack/src/generate_full_fidelity/generate_full_fidelity.opt
*/

require_once 'full_fidelity_parser.php';

abstract class EditableSyntax implements ArrayAccess {
  private string $_syntax_kind;
  protected ?int $_width;
  public function __construct(string $syntax_kind) {
    $this->_syntax_kind = $syntax_kind;
  }

  public function offsetExists (mixed $offset): bool {
    return $offset === 0;
  }

  public function offsetGet (mixed $offset): mixed {
    return $this;
  }

  public function offsetSet (mixed $offset, mixed $value): void {
  }

  public function offsetUnset (mixed $offset): void {
  }

  public function syntax_kind(): string {
    return $this->_syntax_kind;
  }

  public abstract function children():
    Generator<string, EditableSyntax, void>;

  public function preorder(): Continuation<EditableSyntax> {
    yield $this;
    foreach($this->children() as $name => $child)
      foreach($child->preorder() as $descendant)
        yield $descendant;
  }

  private function _parented_preorder(array<EditableSyntax> $parents):
    Continuation<(EditableSyntax, array<EditableSyntax>)> {
    $new_parents = $parents;
    array_push($new_parents, $this);
    yield tuple($this, $parents);
    foreach($this->children() as $name => $child)
      foreach($child->_parented_preorder($new_parents) as $descendant)
        yield $descendant;
  }

  public function parented_preorder():
    Continuation<(EditableSyntax, array<EditableSyntax>)> {
    return $this->_parented_preorder([]);
  }

  public function postorder(): Continuation<EditableSyntax> {
    foreach($this->children() as $name => $child)
      foreach($child->preorder() as $descendant)
        yield $descendant;
    yield $this;
  }

  public function is_token(): bool {
    return false;
  }

  public function is_trivia(): bool {
    return false;
  }

  public function is_list(): bool {
    return false;
  }

  public function is_missing(): bool {
    return false;
  }

  public function width(): int {
    if ($this->_width === null) {
      $width = 0;
      /* TODO: Make an accumulation sequence operator */
      foreach ($this->children() as $name => $node) {
        $width += $node->width();
      }
      $this->_width = $width;
      return $width;
    } else {
      return $this->_width;
    }
  }

  public function full_text(): string {
    /* TODO: Make an accumulation sequence operator */
    $s = '';
    foreach ($this->children() as $name => $node) {
      $s .= $node->full_text();
    }
    return $s;
  }

  public static function from_json(mixed $json, int $position, string $source) {
    switch($json->kind) {
    case 'token':
      return EditableToken::from_json($json->token, $position, $source);
    case 'list':
      return EditableList::from_json($json, $position, $source);
    case 'whitespace':
      return Whitespace::from_json($json, $position, $source);
    case 'end_of_line':
      return EndOfLine::from_json($json, $position, $source);
    case 'delimited_comment':
      return DelimitedComment::from_json($json, $position, $source);
    case 'single_line_comment':
      return SingleLineComment::from_json($json, $position, $source);
    case 'missing':
      return Missing::missing();
FROM_JSON_SYNTAX
    default:
      throw new Exception('unexpected json kind: ' . $json->kind);
      // TODO: Better exception
    }
  }

  public function to_array(): array<EditableSyntax> {
    return [$this];
  }

  public function reduce<TAccumulator>(
    (function
      ( EditableSyntax,
        TAccumulator,
        array<EditableSyntax>): TAccumulator) $reducer,
    TAccumulator $accumulator,
    ?array<EditableSyntax> $parents = null): TAccumulator {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    foreach($this->children() as $child) {
      $accumulator = $child->reduce($reducer, $accumulator, $new_parents);
    }
    return $reducer($this, $accumulator, $parents ?? []);
  }

  // Returns all the parents (and the node itself) of the first node
  // that matches a predicate, or [] if there is no such node.
  public function find_with_parents(
    (function(EditableSyntax):bool) $predicate,
    ?array<EditableSyntax> $parents = null): array<EditableSyntax> {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    if ($predicate($this))
      return $new_parents;
    foreach($this->children() as $child) {
      $result = $child->find_with_parents($predicate, $new_parents);
      if (count($result) != 0)
        return $result;
    }
    return [];
  }

  // Returns a list of nodes that match a predicate.
  public function filter(
    (function(EditableSyntax, ?array<EditableSyntax>):bool) $predicate):
      array<EditableSyntax> {
    $reducer = ($node, $acc, $parents) ==> {
      if ($predicate($node, $parents))
        array_push($acc, $node);
      return $acc;
    };
    return $this->reduce($reducer, []);
  }

  public function of_syntax_kind(string $kind): Continuation<EditableSyntax> {
    foreach($this->preorder() as $child)
      if ($child->syntax_kind() === $kind)
        yield $child;
  }

  public function remove_where(
    (function(EditableSyntax, ?array<EditableSyntax>):bool) $predicate):
      array<EditableSyntax> {
    return $this->rewrite(
      ($node, $parents) ==>
        $predicate($node, $parents) ? Missing::missing() : $node);
  }

  public function without(EditableSyntax $target): EditableSyntax {
    return $this->remove_where(($node, $parents) ==> $node === $target);
  }

  public function replace(
    EditableSyntax $new_node,
    EditableSyntax $target): EditableSyntax {
    return $this->rewrite(
      ($node, $parents) ==> $node === $target ? $new_node : $node);
  }

  public function leftmost_token(): ?EditableSyntax {
    if ($this->is_token())
      return $this;
    foreach($this->children() as $child)
    {
      if (!$child->is_missing())
        return $child->leftmost_token();
    }
    return null;
  }

  public function rightmost_token(): ?EditableSyntax {
    if ($this->is_token())
      return $this;

    // TODO: Better way to reverse a sequence?
    foreach (array_reverse(iterator_to_array($this->children())) as $child) {
      if (!$child->is_missing())
        return $child->rightmost_token();
    }
    return null;
  }

  public function insert_before(
    EditableSyntax $new_node,
    EditableSyntax $target): EditableSyntax {
    // Inserting before missing is an error.
    if ($target->is_missing())
      throw new Exception('Target must not be missing in insert_before.');

    // Inserting missing is a no-op
    if ($new_node->is_missing())
      return $this;

    if ($new_node->is_trivia() && !$target->is_trivia()) {
      $token = $target->is_token() ? $target : $target->leftmost_token();
      if ($token === null)
        throw new Exception('Unable to find token to insert trivia.');

      // Inserting trivia before token is inserting to the right end of
      // the leading trivia.
      $new_leading = EditableList::concatenate_lists(
        $token->leading(), $new_node);
      $new_token = $token->with_leading($new_leading);
      return $this->replace($new_token, $token);
    }

    return $this->replace(
      EditableList::concatenate_lists($new_node, $target), $target);
  }

  public function insert_after(
    EditableSyntax $new_node,
    EditableSyntax $target): EditableSyntax {

    // Inserting after missing is an error.
    if ($target->is_missing())
      throw new Exception('Target must not be missing in insert_after.');

    // Inserting missing is a no-op
    if ($new_node->is_missing())
      return $this;

    if ($new_node->is_trivia() && !$target->is_trivia()) {
      $token = $target->is_token() ? $target : $target->rightmost_token();
      if ($token === null)
        throw new Exception('Unable to find token to insert trivia.');

      // Inserting trivia after token is inserting to the left end of
      // the trailing trivia.
      $new_trailing = EditableList::concatenate_lists(
        $new_node, $token->trailing());
      $new_token = $token->with_trailing($new_trailing);
      return $this->replace($new_token, $token);
    }

    return $this->replace(
      EditableSyntax::concatenate_lists($target, $new_node), $target);
  }
}

final class EditableList extends EditableSyntax implements ArrayAccess {
  private array<EditableSyntax> $_children;
  public function __construct(array<EditableSyntax> $children) {
    parent::__construct('list');
    $this->_children = $children;
  }

  public function offsetExists(mixed $offset): bool {
    return array_key_exists($offset, $this->_children);
  }

  public function offsetGet(mixed $offset): mixed {
    return $this->_children[$offset];
  }

  public function offsetSet(mixed $offset, mixed $value): void {
  }

  public function offsetUnset(mixed $offset): void {
  }

  public function is_list(): bool {
    return true;
  }

  public function to_array(): array<EditableSyntax> {
    return $this->_children;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    foreach($this->_children as $key => $node)
      yield $key => $node;
  }

  /* TODO: Getter by index? */

  public static function to_list(
    array<EditableSyntax> $syntax_list): EditableSyntax {
    if (count($syntax_list) === 0)
      return Missing::missing();
    else if (count($syntax_list) === 1)
      return $syntax_list[0];
    else
      return new EditableList($syntax_list);
  }

  public static function concatenate_lists(
    EditableSyntax $left,
    EditableSyntax $right): EditableSyntax {
    if ($left->is_missing())
      return $right;
    if ($right->is_missing())
      return $left;
    return new EditableList(
      array_merge($left->to_array(), $right->to_array()));
  }

  public static function from_json(mixed $json, int $position, string $source) {
    // TODO Implement array map
    $children = [];
    $current_position = $position;
    foreach($json->elements as $element)
    {
      $child = EditableSyntax::from_json($element, $current_position, $source);
      array_push($children, $child);
      $current_position += $child->width();
    }
    return new EditableList($children);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): EditableSyntax {
    $dirty = false;
    $new_children = [];
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    foreach ($this->children() as $child)
    {
      $new_child = $child->rewrite($rewriter, $new_parents);
      if ($new_child != $child)
        $dirty = true;
      if ($new_child != null)
      {
        if ($new_child->is_list())
        {
          foreach($new_child->children() as $n)
            array_push($new_children, $n);
        }
        else
          array_push($new_children, $new_child);
      }
    }
    $result = $this;
    if ($dirty) {
      if (count($new_children) === 0)
        $result = Missing::missing();
      else if (count($new_children) === 1)
        $result = $new_children[0];
      else
        $result = new EditableList($new_children);
    }
    return $rewriter($result, $parents ?? []);
  }
}

abstract class EditableToken extends EditableSyntax {
  private string $_token_kind;
  private EditableSyntax $_leading;
  private EditableSyntax $_trailing;

  public function __construct(
    string $token_kind,
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('token');
    $this->_token_kind = $token_kind;
    $this->_text = $text;
    $this->_leading = $leading;
    $this->_trailing = $trailing;
    $this->_width = strlen($text) +
      $leading->width() + $trailing->width();
  }

  public function token_kind(): string {
    return $this->_token_kind;
  }

  public function text(): string {
    return $this->_text;
  }

  public function leading(): EditableSyntax {
    return $this->_leading;
  }

  public function trailing(): EditableSyntax {
    return $this->_trailing;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    yield 'leading' => $this->leading();
    yield 'trailing' => $this->trailing();
  }

  public function is_token(): bool {
    return true;
  }

  public function full_text(): string {
    return $this->leading()->full_text() .
      $this->text() .
      $this->trailing()->full_text();
  }

  public abstract function with_leading(
    EditableSyntax $leading): EditableToken;

  public abstract function with_trailing(
    EditableSyntax $trailing): EditableToken;

  private static function factory(
    string $token_kind,
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $token_text) {
    switch($token_kind) {
FACTORY_NO_TEXT_TOKENS
FACTORY_GIVEN_TEXT_TOKENS
FACTORY_VARIABLE_TEXT_TOKENS
      default:
        throw new Exception('unexpected token kind: ' . $token_kind);
        // TODO: Better error
    }
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $leading = $this->leading()->rewrite($rewriter, $new_parents);
    $trailing = $this->trailing()->rewrite($rewriter, $new_parents);
    if ($leading === $this->leading() && $trailing === $this->trailing())
      return $rewriter($this, $parents ?? []);
    else
      return $rewriter(EditableToken::factory(
        $this->token_kind(), $leading, $trailing,
        $this->text()), $parents ?? []);
  }

  public function reduce<TAccumulator>(
    (function
      ( EditableSyntax,
        TAccumulator,
        array<EditableSyntax>): TAccumulator) $reducer,
    TAccumulator $accumulator,
    ?array<EditableSyntax> $parents = null): TAccumulator {
    $accumulator = $this->leading()->reduce($reducer, $accumulator);
    $accumulator = $reducer($this, $accumulator, $parents ?? []);
    $accumulator = $this->trailing()->reduce($reducer, $accumulator);
    return $accumulator;
  }

  public static function from_json(
    mixed $json,
    int $position,
    string $source): EditableToken {
    $leading_list = fold_map(
      $json->leading,
      ($j, $p) ==> EditableSyntax::from_json($j, $p, $source),
      ($j, $p) ==> $j->width + $p,
      $position);

    $leading = EditableList::to_list($leading_list);
    $token_position = $position + $leading->width();
    $token_width = $json->width;
    $token_text = substr($source, $token_position, $token_width);
    $trailing_position = $token_position + $token_width;
    $trailing_list = fold_map(
      $json->trailing,
      ($j, $p) ==> EditableSyntax::from_json($j, $p, $source),
      ($j, $p) ==> $j->width + $p,
      $trailing_position);
    $trailing = EditableList::to_list($trailing_list);
    return EditableToken::factory(
      $json->kind, $leading, $trailing, $token_text);
  }
}

EDITABLE_NO_TEXT_TOKENS
EDITABLE_GIVEN_TEXT_TOKENS
EDITABLE_VARIABLE_TEXT_TOKENS

abstract class EditableTrivia extends EditableSyntax {
  private string $_text;
  public function __construct(string $trivia_kind , string $text) {
    parent::__construct($trivia_kind);
    $this->_text = $text;
  }

  public function text(): string {
    return $this->_text;
  }

  public function full_text() {
    return $this->_text;
  }

  public function width() {
    return strlen($this->_text);
  }

  public function is_trivia() {
    return true;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    yield break;
  }

  public static function from_json(
    mixed $json,
    int $position,
    string $source) {
    $trivia_text = substr($source, $position, $json->width);
    switch($json->kind) {
      case 'whitespace': return new Whitespace($trivia_text);
      case 'end_of_line': return new EndOfLine($trivia_text);
      case 'single_line_comment': return new SingleLineComment($trivia_text);
      case 'delimited_comment': return new DelimitedComment($trivia_text);
      default:
        throw new Exception('unexpected json kind: ' . $json->kind);
        // TODO: Better error
    }
  }

public function rewrite(
  ( function
    (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
  ?array<EditableSyntax> $parents = null): EditableSyntax {
    return $rewriter($this, $parents ?? []);
  }
}
class Whitespace extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('whitespace', $text);
  }
  public function with_text(string $text): Whitespace {
    return new Whitespace($text);
  }
}

class EndOfLine extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('end_of_line', $text);
  }
  public function with_text(string $text): Whitespace {
    return new EndOfLine($text);
  }
}

class SingleLineComment extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('single_line_comment', $text);
  }
  public function with_text(string $text): Whitespace {
    return new SingleLineComment($text);
  }
}

class DelimitedComment extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('delimited_comment', $text);
  }
  public function with_text(string $text): Whitespace {
    return new DelimitedComment($text);
  }
}

final class Missing extends EditableSyntax {
  private static ?Missing $_missing = null;

  public function __construct() {
    parent::__construct('missing');
  }

  public function is_missing(): bool {
    return true;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    yield break;
  }

  public static function missing(): Missing {
    if (Missing::$_missing === null) {
      $m = new Missing();
      Missing::$_missing = $m;
      return $m;
    } else {
      return Missing::$_missing;
    }
  }

  public static function from_json(
    mixed $json,
    int $position,
    string $source) {
    return Missing::missing();
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): EditableSyntax {
      return $rewriter($this, $parents ?? []);
  }

  public function to_array(): array<EditableSyntax> {
    return [];
  }
}

EDITABLE_SYNTAX

function from_json(mixed $json): EditableSyntax {
  return EditableSyntax::from_json($json->parse_tree, 0, $json->program_text);
}
/* End full_fidelity_editable.php */"

  let full_fidelity_hack =
  {
    filename = "hphp/hack/src/full_fidelity/php/full_fidelity_editable.php";
    template = full_fidelity_hack_template;
    transformations = [
      { pattern = "FROM_JSON_SYNTAX"; func = to_from_json };
      { pattern = "EDITABLE_SYNTAX"; func = to_editable_syntax };
    ];
    token_no_text_transformations = [
      { token_pattern = "EDITABLE_NO_TEXT_TOKENS";
        token_func = to_editable_no_text };
      { token_pattern = "FACTORY_NO_TEXT_TOKENS";
        token_func = to_factory_no_text }];
    token_given_text_transformations = [
      { token_pattern = "EDITABLE_GIVEN_TEXT_TOKENS";
        token_func = to_editable_given_text };
      { token_pattern = "FACTORY_GIVEN_TEXT_TOKENS";
        token_func = to_factory_given_text }];
    token_variable_text_transformations = [
      { token_pattern = "EDITABLE_VARIABLE_TEXT_TOKENS";
        token_func = to_editable_variable_text };
      { token_pattern = "FACTORY_VARIABLE_TEXT_TOKENS";
        token_func = to_factory_variable_text }]
  }

end (* GenerateFFHack *)


module GenerateFFTokenKind = struct

  let to_kind_declaration x =
    Printf.sprintf "  | %s\n" x.token_kind

  let to_from_string x =
    Printf.sprintf "  | \"%s\" -> Some %s\n" x.token_text x.token_kind

  let to_to_string x =
    Printf.sprintf "  | %s -> \"%s\"\n" x.token_kind x.token_text

  let full_fidelity_token_kind_template = "(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the \"hack\" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 *)
(* THIS FILE IS GENERATED; DO NOT EDIT IT *)
(* @" ^ "generated *)
(**
  To regenerate this file build hphp/hack/src:generate_full_fidelity and run
  the binary.
  buck build hphp/hack/src:generate_full_fidelity
  buck-out/bin/hphp/hack/src/generate_full_fidelity/generate_full_fidelity.opt
*)

type t =
KIND_DECLARATIONS_NO_TEXT
KIND_DECLARATIONS_GIVEN_TEXT
KIND_DECLARATIONS_VARIABLE_TEXT

let from_string keyword =
  match keyword with
  | \"true\" -> Some BooleanLiteral
  | \"false\" -> Some BooleanLiteral
FROM_STRING_GIVEN_TEXT
  | _ -> None

let to_string kind =
match kind with
| EndOfFile -> \"end of file\"
TO_STRING_GIVEN_TEXT
TO_STRING_VARIABLE_TEXT
"
  let full_fidelity_token_kind =
  {
    filename = "hphp/hack/src/full_fidelity/full_fidelity_token_kind.ml";
    template = full_fidelity_token_kind_template;
    transformations = [];
    token_no_text_transformations = [
      { token_pattern = "KIND_DECLARATIONS_NO_TEXT";
        token_func = to_kind_declaration }];
    token_given_text_transformations = [
      { token_pattern = "KIND_DECLARATIONS_GIVEN_TEXT";
        token_func = to_kind_declaration };
      { token_pattern = "FROM_STRING_GIVEN_TEXT";
        token_func = to_from_string };
      { token_pattern = "TO_STRING_GIVEN_TEXT";
        token_func = to_to_string }];
    token_variable_text_transformations = [
      { token_pattern = "KIND_DECLARATIONS_VARIABLE_TEXT";
        token_func = to_kind_declaration };
      { token_pattern = "TO_STRING_VARIABLE_TEXT";
        token_func = to_to_string }]
  }

end (* GenerateFFTokenKind *)

let () =
  generate_file GenerateFFSyntax.full_fidelity_syntax;
  generate_file GenerateFFSyntaxKind.full_fidelity_syntax_kind;
  generate_file GenerateFFJavaScript.full_fidelity_javascript;
  generate_file GenerateFFHack.full_fidelity_hack;
  generate_file GenerateFFTokenKind.full_fidelity_token_kind
