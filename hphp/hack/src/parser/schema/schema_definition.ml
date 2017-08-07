(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type aggregate_type =
  | TopLevelDeclaration
  | Expression
  | Specifier
  | Parameter
  | ClassBodyDeclaration
  | Statement
  | SwitchLabel
  | LambdaBody
  | ConstructorExpression
  | NamespaceInternals
  | TODO

type child_spec =
  | Token (* Special case, since it's so common, synonym of `Just "Token"` *)
  | Just       of string
  | Aggregate  of aggregate_type
  | ZeroOrMore of child_spec
  | ZeroOrOne  of child_spec

type schema_node =
  { kind_name   : string
  ; type_name   : string
  ; func_name   : string
  ; description : string
  ; prefix      : string
  ; aggregates  : aggregate_type list
  ; fields      : (string * child_spec) list
  }

let schema : schema_node list =
  [ { kind_name   = "EndOfFile"
    ; type_name   = "end_of_file"
    ; func_name   = "end_of_file"
    ; description = "end_of_file"
    ; prefix      = "end_of_file"
    ; aggregates  = [ TopLevelDeclaration; TODO ]
    ; fields      = [ "token", Token ]
    }
  ; { kind_name   = "Script"
    ; type_name   = "script"
    ; func_name   = "script"
    ; description = "script"
    ; prefix      = "script"
    ; aggregates  = []
    ; fields = ["declarations", ZeroOrMore (Aggregate TopLevelDeclaration)]
    }
  ; { kind_name   = "SimpleTypeSpecifier"
    ; type_name   = "simple_type_specifier"
    ; func_name   = "simple_type_specifier"
    ; description = "simple_type_specifier"
    ; prefix      = "simple_type"
    ; aggregates  = [ Specifier ]
    ; fields      = [ "specifier", Token ]
    }
  ; { kind_name   = "LiteralExpression"
    ; type_name   = "literal_expression"
    ; func_name   = "literal_expression"
    ; description = "literal"
    ; prefix      = "literal"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields      = [ "expression", ZeroOrMore (Aggregate Expression) ]
    }
  ; { kind_name   = "VariableExpression"
    ; type_name   = "variable_expression"
    ; func_name   = "variable_expression"
    ; description = "variable"
    ; prefix      = "variable"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields      = [ "expression", Token ]
    }
  ; { kind_name   = "QualifiedNameExpression"
    ; type_name   = "qualified_name_expression"
    ; func_name   = "qualified_name_expression"
    ; description = "qualified_name"
    ; prefix      = "qualified_name"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields      = [ "expression", Token ]
    }
  ; { kind_name   = "PipeVariableExpression"
    ; type_name   = "pipe_variable_expression"
    ; func_name   = "pipe_variable_expression"
    ; description = "pipe_variable"
    ; prefix      = "pipe_variable"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields      = [ "expression", Token ]
    }
  (* TODO: Make all uses of attribute_spec consistent in the API. *)
  ; { kind_name   = "EnumDeclaration"
    ; type_name   = "enum_declaration"
    ; func_name   = "enum_declaration"
    ; description = "enum_declaration"
    ; prefix      = "enum"
    ; aggregates  = [ TopLevelDeclaration ]
    ; fields =
      [ "attribute_spec", ZeroOrOne (Just "AttributeSpecification")
      ; "keyword", Token
      ; "name", Token
      ; "colon", Token
      ; "base", Aggregate Specifier
      ; "type", ZeroOrOne (Just "TypeConstraint")
      ; "left_brace", Token
      ; "enumerators", ZeroOrMore (Just "Enumerator")
      ; "right_brace", Token
      ]
    }
  ; { kind_name   = "Enumerator"
    ; type_name   = "enumerator"
    ; func_name   = "enumerator"
    ; description = "enumerator"
    ; prefix      = "enumerator"
    ; aggregates  = []
    ; fields =
      [ "name", Token
      ; "equal", Token
      ; "value", Aggregate Expression
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "AliasDeclaration"
    ; type_name   = "alias_declaration"
    ; func_name   = "alias_declaration"
    ; description = "alias_declaration"
    ; prefix      = "alias"
    ; aggregates  = [ TopLevelDeclaration ]
    ; fields =
      [ "attribute_spec", ZeroOrOne (Just "AttributeSpecification")
      ; "keyword", Token
      ; "name", ZeroOrOne Token
      ; "generic_parameter", ZeroOrOne (Just "TypeParameters")
      ; "constraint", ZeroOrOne (Just "TypeConstraint")
      ; "equal", ZeroOrOne Token
      ; "type", Aggregate Specifier
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "PropertyDeclaration"
    ; type_name   = "property_declaration"
    ; func_name   = "property_declaration"
    ; description = "property_declaration"
    ; prefix      = "property"
    ; aggregates  = [ ClassBodyDeclaration ]
    ; fields =
      [ "modifiers", ZeroOrMore Token
      ; "type", ZeroOrOne (Aggregate Specifier)
      ; "declarators", ZeroOrMore (Just "PropertyDeclarator")
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "PropertyDeclarator"
    ; type_name   = "property_declarator"
    ; func_name   = "property_declarator"
    ; description = "property_declarator"
    ; prefix      = "property"
    ; aggregates  = []
    ; fields =
      [ "name", Token
      ; "initializer", ZeroOrOne (Just "SimpleInitializer")
      ]
    }
  ; { kind_name   = "NamespaceDeclaration"
    ; type_name   = "namespace_declaration"
    ; func_name   = "namespace_declaration"
    ; description = "namespace_declaration"
    ; prefix      = "namespace"
    ; aggregates  = [ TopLevelDeclaration ]
    ; fields =
      [ "keyword", Token
      ; "name", ZeroOrOne Token
      ; "body", Aggregate NamespaceInternals
      ]
    }
  ; { kind_name   = "NamespaceBody"
    ; type_name   = "namespace_body"
    ; func_name   = "namespace_body"
    ; description = "namespace_body"
    ; prefix      = "namespace"
    ; aggregates  = [ NamespaceInternals ]
    ; fields =
      [ "left_brace", Token
      ; "declarations", ZeroOrMore (Aggregate TopLevelDeclaration)
      ; "right_brace", Token
      ]
    }
  ; { kind_name   = "NamespaceEmptyBody"
    ; type_name   = "namespace_empty_body"
    ; func_name   = "namespace_empty_body"
    ; description = "namespace_empty_body"
    ; prefix      = "namespace"
    ; aggregates  = [ NamespaceInternals ]
    ; fields      = [ "semicolon", Token ]
    }
  ; { kind_name   = "NamespaceUseDeclaration"
    ; type_name   = "namespace_use_declaration"
    ; func_name   = "namespace_use_declaration"
    ; description = "namespace_use_declaration"
    ; prefix      = "namespace_use"
    ; aggregates  = [ TopLevelDeclaration ]
    ; fields =
      [ "keyword", Token
      ; "kind", ZeroOrOne Token
      ; "clauses", ZeroOrMore (ZeroOrOne (Just "NamespaceUseClause"))
      ; "semicolon", ZeroOrOne Token
      ]
    }
  ; { kind_name   = "NamespaceGroupUseDeclaration"
    ; type_name   = "namespace_group_use_declaration"
    ; func_name   = "namespace_group_use_declaration"
    ; description = "namespace_group_use_declaration"
    ; prefix      = "namespace_group_use"
    ; aggregates  = [ TopLevelDeclaration ]
    ; fields =
      [ "keyword", Token
      ; "kind", ZeroOrOne Token
      ; "prefix", Token
      ; "left_brace", Token
      ; "clauses", ZeroOrMore (Just "NamespaceUseClause")
      ; "right_brace", Token
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "NamespaceUseClause"
    ; type_name   = "namespace_use_clause"
    ; func_name   = "namespace_use_clause"
    ; description = "namespace_use_clause"
    ; prefix      = "namespace_use"
    ; aggregates  = []
    ; fields =
      [ "clause_kind", ZeroOrOne Token
      ; "name", Token
      ; "as", ZeroOrOne Token
      ; "alias", ZeroOrOne Token
      ]
    }
  ; { kind_name   = "FunctionDeclaration"
    ; type_name   = "function_declaration"
    ; func_name   = "function_declaration"
    ; description = "function_declaration"
    ; prefix      = "function"
    ; aggregates  = [ TopLevelDeclaration ]
    ; fields =
      [ "attribute_spec", ZeroOrOne (Just "AttributeSpecification")
      ; "declaration_header", Just "FunctionDeclarationHeader"
      ; "body", Just "CompoundStatement"
      ]
    }
  ; { kind_name   = "FunctionDeclarationHeader"
    ; type_name   = "function_declaration_header"
    ; func_name   = "function_declaration_header"
    ; description = "function_declaration_header"
    ; prefix      = "function"
    ; aggregates  = []
    ; fields =
      [ "async", ZeroOrOne Token
      ; "coroutine", ZeroOrOne Token
      ; "keyword", Token
      ; "ampersand", ZeroOrOne Token
      ; "name", Token
      ; "type_parameter_list", ZeroOrOne (Just "TypeParameters")
      ; "left_paren", Token
      ; "parameter_list", ZeroOrMore (ZeroOrOne (Aggregate Parameter))
      ; "right_paren", Token
      ; "colon", ZeroOrOne Token
      ; "type", ZeroOrOne (Aggregate Specifier)
      ; "where_clause", ZeroOrOne (Just "WhereClause")
      ]
    }
  ; { kind_name   = "WhereClause"
    ; type_name   = "where_clause"
    ; func_name   = "where_clause"
    ; description = "where_clause"
    ; prefix      = "where_clause"
    ; aggregates  = []
    ; fields =
      [ "keyword", Token
      ; "constraints", ZeroOrMore (Just "WhereConstraint")
      ]
    }
  ; { kind_name   = "WhereConstraint"
    ; type_name   = "where_constraint"
    ; func_name   = "where_constraint"
    ; description = "where_constraint"
    ; prefix      = "where_constraint"
    ; aggregates  = []
    ; fields =
      [ "left_type", Aggregate Specifier
      ; "operator", Token
      ; "right_type", Aggregate Specifier
      ]
    }
  ; { kind_name   = "MethodishDeclaration"
    ; type_name   = "methodish_declaration"
    ; func_name   = "methodish_declaration"
    ; description = "methodish_declaration"
    ; prefix      = "methodish"
    ; aggregates  = [ ClassBodyDeclaration ]
    ; fields =
      [ "attribute", ZeroOrOne (Just "AttributeSpecification")
      ; "modifiers", ZeroOrMore Token
      ; "function_decl_header", Just "FunctionDeclarationHeader"
      ; "function_body", ZeroOrOne (Just "CompoundStatement")
      ; "semicolon", ZeroOrOne Token
      ]
    }
  ; { kind_name   = "ClassishDeclaration"
    ; type_name   = "classish_declaration"
    ; func_name   = "classish_declaration"
    ; description = "classish_declaration"
    ; prefix      = "classish"
    ; aggregates  = [ TopLevelDeclaration ]
    ; fields =
      [ "attribute", ZeroOrOne (Just "AttributeSpecification")
      ; "modifiers", ZeroOrMore Token
      ; "keyword", Token
      ; "name", Token
      ; "type_parameters", ZeroOrOne (Just "TypeParameters")
      ; "extends_keyword", ZeroOrOne Token
      ; "extends_list", ZeroOrMore (Aggregate Specifier)
      ; "implements_keyword", ZeroOrOne Token
      ; "implements_list", ZeroOrMore (Aggregate Specifier)
      ; "body", Just "ClassishBody"
      ]
    }
  ; { kind_name   = "ClassishBody"
    ; type_name   = "classish_body"
    ; func_name   = "classish_body"
    ; description = "classish_body"
    ; prefix      = "classish_body"
    ; aggregates  = []
    ; fields =
      [ "left_brace", Token
      ; "elements", ZeroOrMore (Aggregate ClassBodyDeclaration)
      ; "right_brace", Token
      ]
    }
  ; { kind_name   = "TraitUsePrecedenceItem"
    ; type_name   = "trait_use_precedence_item"
    ; func_name   = "trait_use_precedence_item"
    ; description = "trait_use_precedence_item"
    ; prefix      = "trait_use_precedence_item"
    ; aggregates  = []
    ; fields =
      [ "name", Aggregate Specifier
      ; "keyword", Token
      ; "removed_names", ZeroOrMore (Aggregate Specifier)
      ]
    }
  ; { kind_name   = "TraitUseAliasItem"
    ; type_name   = "trait_use_alias_item"
    ; func_name   = "trait_use_alias_item"
    ; description = "trait_use_alias_item"
    ; prefix      = "trait_use_alias_item"
    ; aggregates  = []
    ; fields =
      [ "aliasing_name", Aggregate Specifier
      ; "keyword", Token
      ; "visibility", ZeroOrOne Token
      ; "aliased_name", ZeroOrOne (Aggregate Specifier)
      ]
    }
  ; { kind_name   = "TraitUseConflictResolution"
    ; type_name   = "trait_use_conflict_resolution"
    ; func_name   = "trait_use_conflict_resolution"
    ; description = "trait_use_conflict_resolution"
    ; prefix      = "trait_use_conflict_resolution"
    ; aggregates  = []
    ; fields =
      [ "keyword", Token
      ; "names", ZeroOrMore (Aggregate Specifier)
      ; "left_brace", Token
      ; "clauses", ZeroOrMore (Aggregate Specifier)
      ; "right_brace", Token
      ]
    }
  ; { kind_name   = "TraitUse"
    ; type_name   = "trait_use"
    ; func_name   = "trait_use"
    ; description = "trait_use"
    ; prefix      = "trait_use"
    ; aggregates  = []
    ; fields =
      [ "keyword", Token
      ; "names", ZeroOrMore (Aggregate Specifier)
      ; "semicolon", ZeroOrOne Token
      ]
    }
  ; { kind_name   = "RequireClause"
    ; type_name   = "require_clause"
    ; func_name   = "require_clause"
    ; description = "require_clause"
    ; prefix      = "require"
    ; aggregates  = [ ClassBodyDeclaration ]
    ; fields =
      [ "keyword", Token
      ; "kind", Token
      ; "name", Aggregate Specifier
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "ConstDeclaration"
    ; type_name   = "const_declaration"
    ; func_name   = "const_declaration"
    ; description = "const_declaration"
    ; prefix      = "const"
    ; aggregates  = [ ClassBodyDeclaration; TopLevelDeclaration ]
    ; fields =
      [ "abstract", ZeroOrOne Token
      ; "keyword", Token
      ; "type_specifier", ZeroOrOne (Aggregate Specifier)
      ; "declarators", ZeroOrMore (Just "ConstantDeclarator")
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "ConstantDeclarator"
    ; type_name   = "constant_declarator"
    ; func_name   = "constant_declarator"
    ; description = "constant_declarator"
    ; prefix      = "constant_declarator"
    ; aggregates  = []
    ; fields =
      [ "name", Token
      ; "initializer", ZeroOrOne (Just "SimpleInitializer")
      ]
    }
  ; { kind_name   = "TypeConstDeclaration"
    ; type_name   = "type_const_declaration"
    ; func_name   = "type_const_declaration"
    ; description = "type_const_declaration"
    ; prefix      = "type_const"
    ; aggregates  = [ ClassBodyDeclaration ]
    ; fields =
      [ "abstract", ZeroOrOne Token
      ; "keyword", Token
      ; "type_keyword", Token
      ; "name", Token
      ; "type_constraint", ZeroOrOne (Just "TypeConstraint")
      ; "equal", ZeroOrOne Token
      ; "type_specifier", ZeroOrOne (Aggregate Specifier)
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "DecoratedExpression"
    ; type_name   = "decorated_expression"
    ; func_name   = "decorated_expression"
    ; description = "decorated_expression"
    ; prefix      = "decorated_expression"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "decorator", Token
      ; "expression", Aggregate Expression
      ]
    }
  ; { kind_name   = "ParameterDeclaration"
    ; type_name   = "parameter_declaration"
    ; func_name   = "parameter_declaration"
    ; description = "parameter_declaration"
    ; prefix      = "parameter"
    ; aggregates  = [ Parameter ]
    ; fields =
      [ "attribute", ZeroOrOne (Just "AttributeSpecification")
      ; "visibility", ZeroOrOne Token
      ; "type", ZeroOrOne (Aggregate Specifier)
      ; "name", Aggregate Expression
      ; "default_value", ZeroOrOne (Just "SimpleInitializer")
      ]
    }
  ; { kind_name   = "VariadicParameter"
    ; type_name   = "variadic_parameter"
    ; func_name   = "variadic_parameter"
    ; description = "variadic_parameter"
    ; prefix      = "variadic_parameter"
    ; aggregates  = [ Specifier; Parameter ]
    ; fields      = [ "ellipsis", Token ]
    }
  ; { kind_name   = "AttributeSpecification"
    ; type_name   = "attribute_specification"
    ; func_name   = "attribute_specification"
    ; description = "attribute_specification"
    ; prefix      = "attribute_specification"
    ; aggregates  = []
    ; fields =
      [ "left_double_angle", Token
      ; "attributes", ZeroOrMore (Just "Attribute")
      ; "right_double_angle", Token
      ]
    }
  ; { kind_name   = "Attribute"
    ; type_name   = "attribute"
    ; func_name   = "attribute"
    ; description = "attribute"
    ; prefix      = "attribute"
    ; aggregates  = []
    ; fields =
      [ "name", Token
      ; "left_paren", ZeroOrOne Token
      ; "values", ZeroOrMore (Aggregate Expression)
      ; "right_paren", ZeroOrOne Token
      ]
    }
  ; { kind_name   = "InclusionExpression"
    ; type_name   = "inclusion_expression"
    ; func_name   = "inclusion_expression"
    ; description = "inclusion_expression"
    ; prefix      = "inclusion"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "require", Token
      ; "filename", Aggregate Expression
      ]
    }
  ; { kind_name   = "InclusionDirective"
    ; type_name   = "inclusion_directive"
    ; func_name   = "inclusion_directive"
    ; description = "inclusion_directive"
    ; prefix      = "inclusion"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "expression", Just "InclusionExpression"
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "CompoundStatement"
    ; type_name   = "compound_statement"
    ; func_name   = "compound_statement"
    ; description = "compound_statement"
    ; prefix      = "compound"
    ; aggregates  = [ TopLevelDeclaration; Statement; LambdaBody ]
    ; fields =
      [ "left_brace", Token
      ; "statements", ZeroOrMore (Aggregate Statement)
      ; "right_brace", Token
      ]
    }
  ; { kind_name   = "ExpressionStatement"
    ; type_name   = "expression_statement"
    ; func_name   = "expression_statement"
    ; description = "expression_statement"
    ; prefix      = "expression_statement"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "expression", ZeroOrOne (Aggregate Expression)
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "MarkupSection"
    ; type_name   = "markup_section"
    ; func_name   = "markup_section"
    ; description = "markup_section"
    ; prefix      = "markup"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "prefix", ZeroOrOne Token
      ; "text", Token
      ; "suffix", ZeroOrOne (Just "MarkupSuffix")
      ; "expression", ZeroOrOne (Aggregate Expression)
      ]
    }
  ; { kind_name   = "MarkupSuffix"
    ; type_name   = "markup_suffix"
    ; func_name   = "markup_suffix"
    ; description = "markup_suffix"
    ; prefix      = "markup_suffix"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "less_than_question", Token
      ; "name", ZeroOrOne Token
      ]
    }
  ; { kind_name   = "UnsetStatement"
    ; type_name   = "unset_statement"
    ; func_name   = "unset_statement"
    ; description = "unset_statement"
    ; prefix      = "unset"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "variables", ZeroOrMore (Aggregate Expression)
      ; "right_paren", Token
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "WhileStatement"
    ; type_name   = "while_statement"
    ; func_name   = "while_statement"
    ; description = "while_statement"
    ; prefix      = "while"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "condition", Aggregate Expression
      ; "right_paren", Token
      ; "body", Aggregate Statement
      ]
    }
  ; { kind_name   = "IfStatement"
    ; type_name   = "if_statement"
    ; func_name   = "if_statement"
    ; description = "if_statement"
    ; prefix      = "if"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "condition", Aggregate Expression
      ; "right_paren", Token
      ; "statement", Aggregate Statement
      ; "elseif_clauses", ZeroOrMore (Just "ElseifClause")
      ; "else_clause", ZeroOrOne (Just "ElseClause")
      ]
    }
  ; { kind_name   = "ElseifClause"
    ; type_name   = "elseif_clause"
    ; func_name   = "elseif_clause"
    ; description = "elseif_clause"
    ; prefix      = "elseif"
    ; aggregates  = []
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "condition", Aggregate Expression
      ; "right_paren", Token
      ; "statement", Aggregate Statement
      ]
    }
  ; { kind_name   = "ElseClause"
    ; type_name   = "else_clause"
    ; func_name   = "else_clause"
    ; description = "else_clause"
    ; prefix      = "else"
    ; aggregates  = []
    ; fields =
      [ "keyword", Token
      ; "statement", Aggregate Statement
      ]
    }
  ; { kind_name   = "TryStatement"
    ; type_name   = "try_statement"
    ; func_name   = "try_statement"
    ; description = "try_statement"
    ; prefix      = "try"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "compound_statement", Just "CompoundStatement"
      ; "catch_clauses", ZeroOrMore (Just "CatchClause")
      ; "finally_clause", ZeroOrOne (Just "FinallyClause")
      ]
    }
  ; { kind_name   = "CatchClause"
    ; type_name   = "catch_clause"
    ; func_name   = "catch_clause"
    ; description = "catch_clause"
    ; prefix      = "catch"
    ; aggregates  = []
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "type", Just "SimpleTypeSpecifier"
      ; "variable", Token
      ; "right_paren", Token
      ; "body", Just "CompoundStatement"
      ]
    }
  ; { kind_name   = "FinallyClause"
    ; type_name   = "finally_clause"
    ; func_name   = "finally_clause"
    ; description = "finally_clause"
    ; prefix      = "finally"
    ; aggregates  = []
    ; fields =
      [ "keyword", Token
      ; "body", Just "CompoundStatement"
      ]
    }
  ; { kind_name   = "DoStatement"
    ; type_name   = "do_statement"
    ; func_name   = "do_statement"
    ; description = "do_statement"
    ; prefix      = "do"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "body", Aggregate Statement
      ; "while_keyword", Token
      ; "left_paren", Token
      ; "condition", Aggregate Expression
      ; "right_paren", Token
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "ForStatement"
    ; type_name   = "for_statement"
    ; func_name   = "for_statement"
    ; description = "for_statement"
    ; prefix      = "for"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "initializer", ZeroOrMore (Aggregate Expression)
      ; "first_semicolon", Token
      ; "control", ZeroOrMore (Aggregate Expression)
      ; "second_semicolon", Token
      ; "end_of_loop", ZeroOrMore (Aggregate Expression)
      ; "right_paren", Token
      ; "body", Aggregate Statement
      ]
    }
  ; { kind_name   = "ForeachStatement"
    ; type_name   = "foreach_statement"
    ; func_name   = "foreach_statement"
    ; description = "foreach_statement"
    ; prefix      = "foreach"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "collection", Aggregate Expression
      ; "await_keyword", ZeroOrOne Token
      ; "as", Token
      ; "key", ZeroOrOne (Aggregate Expression)
      ; "arrow", ZeroOrOne Token
      ; "value", Aggregate Expression
      ; "right_paren", Token
      ; "body", Aggregate Statement
      ]
    }
  ; { kind_name   = "SwitchStatement"
    ; type_name   = "switch_statement"
    ; func_name   = "switch_statement"
    ; description = "switch_statement"
    ; prefix      = "switch"
    ; aggregates  = [ Statement ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "expression", Aggregate Expression
      ; "right_paren", Token
      ; "left_brace", Token
      ; "sections", ZeroOrMore (Just "SwitchSection")
      ; "right_brace", Token
      ]
    }
  ; { kind_name   = "SwitchSection"
    ; type_name   = "switch_section"
    ; func_name   = "switch_section"
    ; description = "switch_section"
    ; prefix      = "switch_section"
    ; aggregates  = []
    ; fields =
      [ "labels", ZeroOrMore (Aggregate SwitchLabel)
      ; "statements", ZeroOrMore (Aggregate TopLevelDeclaration)
      ; "fallthrough", ZeroOrOne (Just "SwitchFallthrough")
      ]
    }
  ; { kind_name   = "SwitchFallthrough"
    ; type_name   = "switch_fallthrough"
    ; func_name   = "switch_fallthrough"
    ; description = "switch_fallthrough"
    ; prefix      = "fallthrough"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "CaseLabel"
    ; type_name   = "case_label"
    ; func_name   = "case_label"
    ; description = "case_label"
    ; prefix      = "case"
    ; aggregates  = [ SwitchLabel ]
    ; fields =
      [ "keyword", Token
      ; "expression", Aggregate Expression
      ; "colon", Token
      ]
    }
  ; { kind_name   = "DefaultLabel"
    ; type_name   = "default_label"
    ; func_name   = "default_label"
    ; description = "default_label"
    ; prefix      = "default"
    ; aggregates  = [ SwitchLabel ]
    ; fields =
      [ "keyword", Token
      ; "colon", Token
      ]
    }
  ; { kind_name   = "ReturnStatement"
    ; type_name   = "return_statement"
    ; func_name   = "return_statement"
    ; description = "return_statement"
    ; prefix      = "return"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "expression", ZeroOrOne (Aggregate Expression)
      ; "semicolon", ZeroOrOne Token
      ]
    }
  ; { kind_name   = "GotoLabel"
    ; type_name   = "goto_label"
    ; func_name   = "goto_label"
    ; description = "goto_label"
    ; prefix      = "goto_label"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "name", Token
      ; "colon", Token
      ]
    }
  ; { kind_name   = "GotoStatement"
    ; type_name   = "goto_statement"
    ; func_name   = "goto_statement"
    ; description = "goto_statement"
    ; prefix      = "goto_statement"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "label_name", Token
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "ThrowStatement"
    ; type_name   = "throw_statement"
    ; func_name   = "throw_statement"
    ; description = "throw_statement"
    ; prefix      = "throw"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "expression", Aggregate Expression
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "BreakStatement"
    ; type_name   = "break_statement"
    ; func_name   = "break_statement"
    ; description = "break_statement"
    ; prefix      = "break"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "level", ZeroOrOne (Just "LiteralExpression")
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "ContinueStatement"
    ; type_name   = "continue_statement"
    ; func_name   = "continue_statement"
    ; description = "continue_statement"
    ; prefix      = "continue"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "level", ZeroOrOne (Just "LiteralExpression")
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "FunctionStaticStatement"
    ; type_name   = "function_static_statement"
    ; func_name   = "function_static_statement"
    ; description = "function_static_statement"
    ; prefix      = "static"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "static_keyword", Token
      ; "declarations", ZeroOrMore (Just "StaticDeclarator")
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "StaticDeclarator"
    ; type_name   = "static_declarator"
    ; func_name   = "static_declarator"
    ; description = "static_declarator"
    ; prefix      = "static"
    ; aggregates  = []
    ; fields =
      [ "name", Token
      ; "initializer", ZeroOrOne (Just "SimpleInitializer")
      ]
    }
  ; { kind_name   = "EchoStatement"
    ; type_name   = "echo_statement"
    ; func_name   = "echo_statement"
    ; description = "echo_statement"
    ; prefix      = "echo"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "expressions", ZeroOrMore (Aggregate Expression)
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "GlobalStatement"
    ; type_name   = "global_statement"
    ; func_name   = "global_statement"
    ; description = "global_statement"
    ; prefix      = "global"
    ; aggregates  = [ TopLevelDeclaration; Statement ]
    ; fields =
      [ "keyword", Token
      ; "variables", ZeroOrMore Token
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "SimpleInitializer"
    ; type_name   = "simple_initializer"
    ; func_name   = "simple_initializer"
    ; description = "simple_initializer"
    ; prefix      = "simple_initializer"
    ; aggregates  = []
    ; fields =
      [ "equal", Token
      ; "value", Aggregate Expression
      ]
    }
  ; { kind_name   = "AnonymousFunction"
    ; type_name   = "anonymous_function"
    ; func_name   = "anonymous_function"
    ; description = "anonymous_function"
    ; prefix      = "anonymous"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "static_keyword", ZeroOrOne Token
      ; "async_keyword", ZeroOrOne Token
      ; "coroutine_keyword", ZeroOrOne Token
      ; "function_keyword", Token
      ; "left_paren", Token
      ; "parameters", ZeroOrMore (Aggregate Parameter)
      ; "right_paren", Token
      ; "colon", ZeroOrOne Token
      ; "type", ZeroOrOne (Aggregate Specifier)
      ; "use", ZeroOrOne (Just "AnonymousFunctionUseClause")
      ; "body", Just "CompoundStatement"
      ]
    }
  ; { kind_name   = "AnonymousFunctionUseClause"
    ; type_name   = "anonymous_function_use_clause"
    ; func_name   = "anonymous_function_use_clause"
    ; description = "anonymous_function_use_clause"
    ; prefix      = "anonymous_use"
    ; aggregates  = []
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "variables", ZeroOrMore (Aggregate Expression)
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "LambdaExpression"
    ; type_name   = "lambda_expression"
    ; func_name   = "lambda_expression"
    ; description = "lambda_expression"
    ; prefix      = "lambda"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "async", ZeroOrOne Token
      ; "coroutine", ZeroOrOne Token
      ; "signature", Aggregate Specifier
      ; "arrow", Token
      ; "body", Aggregate LambdaBody
      ]
    }
  ; { kind_name   = "LambdaSignature"
    ; type_name   = "lambda_signature"
    ; func_name   = "lambda_signature"
    ; description = "lambda_signature"
    ; prefix      = "lambda"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "left_paren", Token
      ; "parameters", ZeroOrMore (Aggregate Parameter)
      ; "right_paren", Token
      ; "colon", ZeroOrOne Token
      ; "type", ZeroOrOne (Aggregate Specifier)
      ]
    }
  ; { kind_name   = "CastExpression"
    ; type_name   = "cast_expression"
    ; func_name   = "cast_expression"
    ; description = "cast_expression"
    ; prefix      = "cast"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "left_paren", Token
      ; "type", Token
      ; "right_paren", Token
      ; "operand", Aggregate Expression
      ]
    }
  ; { kind_name   = "ScopeResolutionExpression"
    ; type_name   = "scope_resolution_expression"
    ; func_name   = "scope_resolution_expression"
    ; description = "scope_resolution_expression"
    ; prefix      = "scope_resolution"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "qualifier", Aggregate Expression
      ; "operator", Token
      ; "name", Aggregate Expression
      ]
    }
  ; { kind_name   = "MemberSelectionExpression"
    ; type_name   = "member_selection_expression"
    ; func_name   = "member_selection_expression"
    ; description = "member_selection_expression"
    ; prefix      = "member"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "object", Aggregate Expression
      ; "operator", Token
      ; "name", Token
      ]
    }
  ; { kind_name   = "SafeMemberSelectionExpression"
    ; type_name   = "safe_member_selection_expression"
    ; func_name   = "safe_member_selection_expression"
    ; description = "safe_member_selection_expression"
    ; prefix      = "safe_member"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "object", Aggregate Expression
      ; "operator", Token
      ; "name", Token
      ]
    }
  ; { kind_name   = "EmbeddedMemberSelectionExpression"
    ; type_name   = "embedded_member_selection_expression"
    ; func_name   = "embedded_member_selection_expression"
    ; description = "embedded_member_selection_expression"
    ; prefix      = "embedded_member"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "object", Just "VariableExpression"
      ; "operator", Token
      ; "name", Token
      ]
    }
  ; { kind_name   = "YieldExpression"
    ; type_name   = "yield_expression"
    ; func_name   = "yield_expression"
    ; description = "yield_expression"
    ; prefix      = "yield"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "operand", Aggregate ConstructorExpression
      ]
    }
  ; { kind_name   = "YieldFromExpression"
    ; type_name   = "yield_from_expression"
    ; func_name   = "yield_from_expression"
    ; description = "yield_from_expression"
    ; prefix      = "yield_from"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "yield_keyword", Token
      ; "from_keyword", Token
      ; "operand", Aggregate Expression
      ]
    }
  ; { kind_name   = "PrefixUnaryExpression"
    ; type_name   = "prefix_unary_expression"
    ; func_name   = "prefix_unary_expression"
    ; description = "prefix_unary_expression"
    ; prefix      = "prefix_unary"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "operator", Token
      ; "operand", Aggregate Expression
      ]
    }
  ; { kind_name   = "PostfixUnaryExpression"
    ; type_name   = "postfix_unary_expression"
    ; func_name   = "postfix_unary_expression"
    ; description = "postfix_unary_expression"
    ; prefix      = "postfix_unary"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "operand", Aggregate Expression
      ; "operator", Token
      ]
    }
  ; { kind_name   = "BinaryExpression"
    ; type_name   = "binary_expression"
    ; func_name   = "binary_expression"
    ; description = "binary_expression"
    ; prefix      = "binary"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "left_operand", Aggregate Expression
      ; "operator", Token
      ; "right_operand", Aggregate Expression
      ]
    }
  ; { kind_name   = "InstanceofExpression"
    ; type_name   = "instanceof_expression"
    ; func_name   = "instanceof_expression"
    ; description = "instanceof_expression"
    ; prefix      = "instanceof"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "left_operand", Aggregate Expression
      ; "operator", Token
      ; "right_operand", Aggregate Expression
      ]
    }
  ; { kind_name   = "ConditionalExpression"
    ; type_name   = "conditional_expression"
    ; func_name   = "conditional_expression"
    ; description = "conditional_expression"
    ; prefix      = "conditional"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "test", Aggregate Expression
      ; "question", Token
      ; "consequence", ZeroOrOne (Aggregate Expression)
      ; "colon", Token
      ; "alternative", Aggregate Expression
      ]
    }
  ; { kind_name   = "EvalExpression"
    ; type_name   = "eval_expression"
    ; func_name   = "eval_expression"
    ; description = "eval_expression"
    ; prefix      = "eval"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "argument", Aggregate Expression
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "EmptyExpression"
    ; type_name   = "empty_expression"
    ; func_name   = "empty_expression"
    ; description = "empty_expression"
    ; prefix      = "empty"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "argument", Aggregate Expression
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "DefineExpression"
    ; type_name   = "define_expression"
    ; func_name   = "define_expression"
    ; description = "define_expression"
    ; prefix      = "define"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "argument_list", ZeroOrMore (Aggregate Expression)
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "IssetExpression"
    ; type_name   = "isset_expression"
    ; func_name   = "isset_expression"
    ; description = "isset_expression"
    ; prefix      = "isset"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "argument_list", ZeroOrMore (Aggregate Expression)
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "FunctionCallExpression"
    ; type_name   = "function_call_expression"
    ; func_name   = "function_call_expression"
    ; description = "function_call_expression"
    ; prefix      = "function_call"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "receiver", Aggregate Expression
      ; "left_paren", Token
      ; "argument_list", ZeroOrMore (Aggregate Expression)
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "FunctionCallWithTypeArgumentsExpression"
    ; type_name   = "function_call_with_type_arguments_expression"
    ; func_name   = "function_call_with_type_arguments_expression"
    ; description = "function_call_with_type_arguments_expression"
    ; prefix      = "function_call_with_type_arguments"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "receiver", Aggregate Expression
      ; "type_args", Just "TypeArguments"
      ; "left_paren", Token
      ; "argument_list", ZeroOrMore (Aggregate Expression)
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "ParenthesizedExpression"
    ; type_name   = "parenthesized_expression"
    ; func_name   = "parenthesized_expression"
    ; description = "parenthesized_expression"
    ; prefix      = "parenthesized_expression"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "left_paren", Token
      ; "expression", Aggregate Expression
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "BracedExpression"
    ; type_name   = "braced_expression"
    ; func_name   = "braced_expression"
    ; description = "braced_expression"
    ; prefix      = "braced_expression"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "left_brace", Token
      ; "expression", Aggregate Expression
      ; "right_brace", Token
      ]
    }
  ; { kind_name   = "EmbeddedBracedExpression"
    ; type_name   = "embedded_braced_expression"
    ; func_name   = "embedded_braced_expression"
    ; description = "embedded_braced_expression"
    ; prefix      = "embedded_braced_expression"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "left_brace", Token
      ; "expression", Aggregate Expression
      ; "right_brace", Token
      ]
    }
  ; { kind_name   = "ListExpression"
    ; type_name   = "list_expression"
    ; func_name   = "list_expression"
    ; description = "list_expression"
    ; prefix      = "list"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "members", ZeroOrMore (ZeroOrOne (Aggregate Expression))
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "CollectionLiteralExpression"
    ; type_name   = "collection_literal_expression"
    ; func_name   = "collection_literal_expression"
    ; description = "collection_literal_expression"
    ; prefix      = "collection_literal"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "name", Token
      ; "left_brace", Token
      ; "initializers", ZeroOrMore (Aggregate ConstructorExpression)
      ; "right_brace", Token
      ]
    }
  ; { kind_name   = "ObjectCreationExpression"
    ; type_name   = "object_creation_expression"
    ; func_name   = "object_creation_expression"
    ; description = "object_creation_expression"
    ; prefix      = "object_creation"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "new_keyword", Token
      ; "type", Aggregate TODO
      ; "left_paren", ZeroOrOne Token
      ; "argument_list", ZeroOrMore (Aggregate Expression)
      ; "right_paren", ZeroOrOne Token
      ]
    }
  ; { kind_name   = "ArrayCreationExpression"
    ; type_name   = "array_creation_expression"
    ; func_name   = "array_creation_expression"
    ; description = "array_creation_expression"
    ; prefix      = "array_creation"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "left_bracket", Token
      ; "members", ZeroOrMore (Aggregate ConstructorExpression)
      ; "right_bracket", Token
      ]
    }
  ; { kind_name   = "ArrayIntrinsicExpression"
    ; type_name   = "array_intrinsic_expression"
    ; func_name   = "array_intrinsic_expression"
    ; description = "array_intrinsic_expression"
    ; prefix      = "array_intrinsic"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "members", ZeroOrMore (Aggregate ConstructorExpression)
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "DarrayIntrinsicExpression"
    ; type_name   = "darray_intrinsic_expression"
    ; func_name   = "darray_intrinsic_expression"
    ; description = "darray_intrinsic_expression"
    ; prefix      = "darray_intrinsic"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_bracket", Token
      ; "members", ZeroOrMore (Just "ElementInitializer")
      ; "right_bracket", Token
      ]
    }
  ; { kind_name   = "DictionaryIntrinsicExpression"
    ; type_name   = "dictionary_intrinsic_expression"
    ; func_name   = "dictionary_intrinsic_expression"
    ; description = "dictionary_intrinsic_expression"
    ; prefix      = "dictionary_intrinsic"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_bracket", Token
      ; "members", ZeroOrMore (Just "ElementInitializer")
      ; "right_bracket", Token
      ]
    }
  ; { kind_name   = "KeysetIntrinsicExpression"
    ; type_name   = "keyset_intrinsic_expression"
    ; func_name   = "keyset_intrinsic_expression"
    ; description = "keyset_intrinsic_expression"
    ; prefix      = "keyset_intrinsic"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_bracket", Token
      ; "members", ZeroOrMore (Aggregate Expression)
      ; "right_bracket", Token
      ]
    }
  ; { kind_name   = "VarrayIntrinsicExpression"
    ; type_name   = "varray_intrinsic_expression"
    ; func_name   = "varray_intrinsic_expression"
    ; description = "varray_intrinsic_expression"
    ; prefix      = "varray_intrinsic"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_bracket", Token
      ; "members", ZeroOrMore (Aggregate Expression)
      ; "right_bracket", Token
      ]
    }
  ; { kind_name   = "VectorIntrinsicExpression"
    ; type_name   = "vector_intrinsic_expression"
    ; func_name   = "vector_intrinsic_expression"
    ; description = "vector_intrinsic_expression"
    ; prefix      = "vector_intrinsic"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_bracket", Token
      ; "members", ZeroOrMore (Aggregate Expression)
      ; "right_bracket", Token
      ]
    }
  ; { kind_name   = "ElementInitializer"
    ; type_name   = "element_initializer"
    ; func_name   = "element_initializer"
    ; description = "element_initializer"
    ; prefix      = "element"
    ; aggregates  = [ ConstructorExpression ]
    ; fields =
      [ "key", Aggregate Expression
      ; "arrow", Token
      ; "value", Aggregate Expression
      ]
    }
  ; { kind_name   = "SubscriptExpression"
    ; type_name   = "subscript_expression"
    ; func_name   = "subscript_expression"
    ; description = "subscript_expression"
    ; prefix      = "subscript"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "receiver", Aggregate Expression
      ; "left_bracket", Token
      ; "index", ZeroOrOne (Aggregate Expression)
      ; "right_bracket", Token
      ]
    }
  ; { kind_name   = "EmbeddedSubscriptExpression"
    ; type_name   = "embedded_subscript_expression"
    ; func_name   = "embedded_subscript_expression"
    ; description = "embedded_subscript_expression"
    ; prefix      = "embedded_subscript"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "receiver", Just "VariableExpression"
      ; "left_bracket", Token
      ; "index", Aggregate Expression
      ; "right_bracket", Token
      ]
    }
  ; { kind_name   = "AwaitableCreationExpression"
    ; type_name   = "awaitable_creation_expression"
    ; func_name   = "awaitable_creation_expression"
    ; description = "awaitable_creation_expression"
    ; prefix      = "awaitable"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "async", Token
      ; "coroutine", ZeroOrOne Token
      ; "compound_statement", Just "CompoundStatement"
      ]
    }
  ; { kind_name   = "XHPChildrenDeclaration"
    ; type_name   = "xhp_children_declaration"
    ; func_name   = "xhp_children_declaration"
    ; description = "xhp_children_declaration"
    ; prefix      = "xhp_children"
    ; aggregates  = [ ClassBodyDeclaration ]
    ; fields =
      [ "keyword", Token
      ; "expression", Aggregate Expression
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "XHPChildrenParenthesizedList"
    ; type_name   = "xhp_children_parenthesized_list"
    ; func_name   = "xhp_children_parenthesized_list"
    ; description = "xhp_children_parenthesized_list"
    ; prefix      = "xhp_children_list"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "left_paren", Token
      ; "xhp_children", ZeroOrMore (Aggregate Expression)
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "XHPCategoryDeclaration"
    ; type_name   = "xhp_category_declaration"
    ; func_name   = "xhp_category_declaration"
    ; description = "xhp_category_declaration"
    ; prefix      = "xhp_category"
    ; aggregates  = [ ClassBodyDeclaration ]
    ; fields =
      [ "keyword", Token
      ; "categories", ZeroOrMore Token
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "XHPEnumType"
    ; type_name   = "xhp_enum_type"
    ; func_name   = "xhp_enum_type"
    ; description = "xhp_enum_type"
    ; prefix      = "xhp_enum"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "keyword", Token
      ; "left_brace", Token
      ; "values", ZeroOrMore (Just "LiteralExpression")
      ; "right_brace", Token
      ]
    }
  ; { kind_name   = "XHPRequired"
    ; type_name   = "xhp_required"
    ; func_name   = "xhp_required"
    ; description = "xhp_required"
    ; prefix      = "xhp_required"
    ; aggregates  = []
    ; fields =
      [ "at", Token
      ; "keyword", Token
      ]
    }
  ; { kind_name   = "XHPClassAttributeDeclaration"
    ; type_name   = "xhp_class_attribute_declaration"
    ; func_name   = "xhp_class_attribute_declaration"
    ; description = "xhp_class_attribute_declaration"
    ; prefix      = "xhp_attribute"
    ; aggregates  = [ ClassBodyDeclaration ]
    ; fields =
      [ "keyword", Token
      ; "attributes", ZeroOrMore (Aggregate TODO)
      ; "semicolon", Token
      ]
    }
  ; { kind_name   = "XHPClassAttribute"
    ; type_name   = "xhp_class_attribute"
    ; func_name   = "xhp_class_attribute"
    ; description = "xhp_class_attribute"
    ; prefix      = "xhp_attribute_decl"
    ; aggregates  = []
    ; fields =
      [ "type", Aggregate Specifier
      ; "name", Token
      ; "initializer", ZeroOrOne (Just "SimpleInitializer")
      ; "required", ZeroOrOne (Just "XHPRequired")
      ]
    }
  ; { kind_name   = "XHPSimpleClassAttribute"
    ; type_name   = "xhp_simple_class_attribute"
    ; func_name   = "xhp_simple_class_attribute"
    ; description = "xhp_simple_class_attribute"
    ; prefix      = "xhp_simple_class_attribute"
    ; aggregates  = []
    ; fields      = [ "type", Just "SimpleTypeSpecifier" ]
    }
  ; { kind_name   = "XHPAttribute"
    ; type_name   = "xhp_attribute"
    ; func_name   = "xhp_attribute"
    ; description = "xhp_attribute"
    ; prefix      = "xhp_attribute"
    ; aggregates  = []
    ; fields =
      [ "name", Token
      ; "equal", Token
      ; "expression", Aggregate Expression
      ]
    }
  ; { kind_name   = "XHPOpen"
    ; type_name   = "xhp_open"
    ; func_name   = "xhp_open"
    ; description = "xhp_open"
    ; prefix      = "xhp_open"
    ; aggregates  = []
    ; fields =
      [ "left_angle", Token
      ; "name", Token
      ; "attributes", ZeroOrMore (Just "XHPAttribute")
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "XHPExpression"
    ; type_name   = "xhp_expression"
    ; func_name   = "xhp_expression"
    ; description = "xhp_expression"
    ; prefix      = "xhp"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "open", Just "XHPOpen"
      ; "body", ZeroOrMore (Aggregate Expression)
      ; "close", ZeroOrOne (Just "XHPClose")
      ]
    }
  ; { kind_name   = "XHPClose"
    ; type_name   = "xhp_close"
    ; func_name   = "xhp_close"
    ; description = "xhp_close"
    ; prefix      = "xhp_close"
    ; aggregates  = []
    ; fields =
      [ "left_angle", Token
      ; "name", Token
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "TypeConstant"
    ; type_name   = "type_constant"
    ; func_name   = "type_constant"
    ; description = "type_constant"
    ; prefix      = "type_constant"
    ; aggregates  = [ Statement ]
    ; fields =
      [ "left_type", Aggregate Specifier
      ; "separator", Token
      ; "right_type", Token
      ]
    }
  ; { kind_name   = "VectorTypeSpecifier"
    ; type_name   = "vector_type_specifier"
    ; func_name   = "vector_type_specifier"
    ; description = "vector_type_specifier"
    ; prefix      = "vector_type"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "keyword", Token
      ; "left_angle", Token
      ; "type", Aggregate Specifier
      ; "trailing_comma", ZeroOrOne Token
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "KeysetTypeSpecifier"
    ; type_name   = "keyset_type_specifier"
    ; func_name   = "keyset_type_specifier"
    ; description = "keyset_type_specifier"
    ; prefix      = "keyset_type"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "keyword", Token
      ; "left_angle", Token
      ; "type", Aggregate Specifier
      ; "trailing_comma", ZeroOrOne Token
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "TupleTypeExplicitSpecifier"
    ; type_name   = "tuple_type_explicit_specifier"
    ; func_name   = "tuple_type_explicit_specifier"
    ; description = "tuple_type_explicit_specifier"
    ; prefix      = "tuple_type"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "keyword", Token
      ; "left_angle", Token
      ; "types", Just "SimpleTypeSpecifier"
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "VarrayTypeSpecifier"
    ; type_name   = "varray_type_specifier"
    ; func_name   = "varray_type_specifier"
    ; description = "varray_type_specifier"
    ; prefix      = "varray"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "keyword", Token
      ; "left_angle", Token
      ; "type", Just "SimpleTypeSpecifier"
      ; "trailing_comma", ZeroOrOne Token
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "VectorArrayTypeSpecifier"
    ; type_name   = "vector_array_type_specifier"
    ; func_name   = "vector_array_type_specifier"
    ; description = "vector_array_type_specifier"
    ; prefix      = "vector_array"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "keyword", Token
      ; "left_angle", Token
      ; "type", Aggregate Specifier
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "TypeParameter"
    ; type_name   = "type_parameter"
    ; func_name   = "type_parameter"
    ; description = "type_parameter"
    ; prefix      = "type"
    ; aggregates  = []
    ; fields =
      [ "variance", ZeroOrOne Token
      ; "name", Token
      ; "constraints", ZeroOrMore (Just "TypeConstraint")
      ]
    }
  ; { kind_name   = "TypeConstraint"
    ; type_name   = "type_constraint"
    ; func_name   = "type_constraint"
    ; description = "type_constraint"
    ; prefix      = "constraint"
    ; aggregates  = []
    ; fields =
      [ "keyword", Token
      ; "type", Aggregate Specifier
      ]
    }
  ; { kind_name   = "DarrayTypeSpecifier"
    ; type_name   = "darray_type_specifier"
    ; func_name   = "darray_type_specifier"
    ; description = "darray_type_specifier"
    ; prefix      = "darray"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "keyword", Token
      ; "left_angle", Token
      ; "key", Just "SimpleTypeSpecifier"
      ; "comma", Token
      ; "value", Just "SimpleTypeSpecifier"
      ; "trailing_comma", ZeroOrOne Token
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "MapArrayTypeSpecifier"
    ; type_name   = "map_array_type_specifier"
    ; func_name   = "map_array_type_specifier"
    ; description = "map_array_type_specifier"
    ; prefix      = "map_array"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "keyword", Token
      ; "left_angle", Token
      ; "key", Aggregate Specifier
      ; "comma", Token
      ; "value", Aggregate Specifier
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "DictionaryTypeSpecifier"
    ; type_name   = "dictionary_type_specifier"
    ; func_name   = "dictionary_type_specifier"
    ; description = "dictionary_type_specifier"
    ; prefix      = "dictionary_type"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "keyword", Token
      ; "left_angle", Token
      ; "members", ZeroOrMore (Aggregate Specifier)
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "ClosureTypeSpecifier"
    ; type_name   = "closure_type_specifier"
    ; func_name   = "closure_type_specifier"
    ; description = "closure_type_specifier"
    ; prefix      = "closure"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "outer_left_paren", Token
      ; "coroutine", ZeroOrOne Token
      ; "function_keyword", Token
      ; "inner_left_paren", Token
      ; "parameter_types", ZeroOrMore (Aggregate Specifier)
      ; "inner_right_paren", Token
      ; "colon", Token
      ; "return_type", Aggregate Specifier
      ; "outer_right_paren", Token
      ]
    }
  ; { kind_name   = "ClassnameTypeSpecifier"
    ; type_name   = "classname_type_specifier"
    ; func_name   = "classname_type_specifier"
    ; description = "classname_type_specifier"
    ; prefix      = "classname"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "keyword", Token
      ; "left_angle", Token
      ; "type", Aggregate Specifier
      ; "trailing_comma", ZeroOrOne Token
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "FieldSpecifier"
    ; type_name   = "field_specifier"
    ; func_name   = "field_specifier"
    ; description = "field_specifier"
    ; prefix      = "field"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "question", ZeroOrOne Token
      ; "name", Aggregate Expression
      ; "arrow", Token
      ; "type", Aggregate Specifier
      ]
    }
  ; { kind_name   = "FieldInitializer"
    ; type_name   = "field_initializer"
    ; func_name   = "field_initializer"
    ; description = "field_initializer"
    ; prefix      = "field_initializer"
    ; aggregates  = []
    ; fields =
      [ "name", Aggregate Expression
      ; "arrow", Token
      ; "value", Aggregate Expression
      ]
    }
  ; { kind_name   = "ShapeTypeSpecifier"
    ; type_name   = "shape_type_specifier"
    ; func_name   = "shape_type_specifier"
    ; description = "shape_type_specifier"
    ; prefix      = "shape_type"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "fields", ZeroOrMore (Just "FieldSpecifier")
      ; "ellipsis", ZeroOrOne Token
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "ShapeExpression"
    ; type_name   = "shape_expression"
    ; func_name   = "shape_expression"
    ; description = "shape_expression"
    ; prefix      = "shape_expression"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "fields", ZeroOrMore (Just "FieldInitializer")
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "TupleExpression"
    ; type_name   = "tuple_expression"
    ; func_name   = "tuple_expression"
    ; description = "tuple_expression"
    ; prefix      = "tuple_expression"
    ; aggregates  = [ Expression; ConstructorExpression; LambdaBody ]
    ; fields =
      [ "keyword", Token
      ; "left_paren", Token
      ; "items", ZeroOrMore (Aggregate Expression)
      ; "right_paren", Token
      ]
    }
  (* TODO: Rename this; generic type specifiers are also used for
   * type-annotated method calls and for object creations with type annotations
   * This naming is now very misleading (e.g. class_type being any name).
   *)
  ; { kind_name   = "GenericTypeSpecifier"
    ; type_name   = "generic_type_specifier"
    ; func_name   = "generic_type_specifier"
    ; description = "generic_type_specifier"
    ; prefix      = "generic"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "class_type", Token
      ; "argument_list", Just "TypeArguments"
      ]
    }
  ; { kind_name   = "NullableTypeSpecifier"
    ; type_name   = "nullable_type_specifier"
    ; func_name   = "nullable_type_specifier"
    ; description = "nullable_type_specifier"
    ; prefix      = "nullable"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "question", Token
      ; "type", Aggregate Specifier
      ]
    }
  ; { kind_name   = "SoftTypeSpecifier"
    ; type_name   = "soft_type_specifier"
    ; func_name   = "soft_type_specifier"
    ; description = "soft_type_specifier"
    ; prefix      = "soft"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "at", Token
      ; "type", Aggregate Specifier
      ]
    }
  ; { kind_name   = "TypeArguments"
    ; type_name   = "type_arguments"
    ; func_name   = "type_arguments"
    ; description = "type_arguments"
    ; prefix      = "type_arguments"
    ; aggregates  = []
    ; fields =
      [ "left_angle", Token
      ; "types", ZeroOrMore (Aggregate Specifier)
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "TypeParameters"
    ; type_name   = "type_parameters"
    ; func_name   = "type_parameters"
    ; description = "type_parameters"
    ; prefix      = "type_parameters"
    ; aggregates  = []
    ; fields =
      [ "left_angle", Token
      ; "parameters", ZeroOrMore (Just "TypeParameter")
      ; "right_angle", Token
      ]
    }
  ; { kind_name   = "TupleTypeSpecifier"
    ; type_name   = "tuple_type_specifier"
    ; func_name   = "tuple_type_specifier"
    ; description = "tuple_type_specifier"
    ; prefix      = "tuple"
    ; aggregates  = [ Specifier ]
    ; fields =
      [ "left_paren", Token
      ; "types", ZeroOrMore (ZeroOrOne (Aggregate Specifier))
      ; "right_paren", Token
      ]
    }
  ; { kind_name   = "ErrorSyntax"
    ; type_name   = "error"
    ; func_name   = "error"
    ; description = "error"
    ; prefix      = "error"
    ; aggregates  = []
    ; fields      = [ "error", Just "error" ]
    }
  ; { kind_name   = "ListItem"
    ; type_name   = "list_item"
    ; func_name   = "list_item"
    ; description = "list_item"
    ; prefix      = "list"
    ; aggregates  = []
    ; fields      =
      [ "item", Just "error"
      ; "separator", Token
      ]
    }
  ]

(******************************************************************************(
 * Utilities for aggregate types
)******************************************************************************)
let generated_aggregate_types =
  [ TopLevelDeclaration
  ; Expression
  ; Specifier
  ; Parameter
  ; ClassBodyDeclaration
  ; Statement
  ; SwitchLabel
  ; LambdaBody
  ; ConstructorExpression
  ; NamespaceInternals
  ; TODO
  ]

let string_of_aggregate_type = function
  | TopLevelDeclaration    -> "TopLevelDeclaration"
  | Expression             -> "Expression"
  | Specifier              -> "Specifier"
  | Parameter              -> "Parameter"
  | ClassBodyDeclaration   -> "ClassBodyDeclaration"
  | Statement              -> "Statement"
  | SwitchLabel            -> "SwitchLabel"
  | LambdaBody             -> "LambdaBody"
  | ConstructorExpression  -> "ConstructorExpression"
  | NamespaceInternals     -> "NamespaceInternals"
  | TODO                   -> "TODO"

module AggregateKey = struct
  type t = aggregate_type
  let compare (x: t) (y: t) = compare x y
  let to_string = string_of_aggregate_type
end
module AggMap = MyMap.Make(AggregateKey)

let aggregation_of_top_level_declaration =
  List.filter (fun x -> List.mem TopLevelDeclaration   x.aggregates) schema
let aggregation_of_expression =
  List.filter (fun x -> List.mem Expression            x.aggregates) schema
let aggregation_of_specifier =
  List.filter (fun x -> List.mem Specifier             x.aggregates) schema
let aggregation_of_parameter =
  List.filter (fun x -> List.mem Parameter             x.aggregates) schema
let aggregation_of_class_body_declaration =
  List.filter (fun x -> List.mem ClassBodyDeclaration  x.aggregates) schema
let aggregation_of_statement =
  List.filter (fun x -> List.mem Statement             x.aggregates) schema
let aggregation_of_switch_label =
  List.filter (fun x -> List.mem SwitchLabel           x.aggregates) schema
let aggregation_of_lambda_body =
  List.filter (fun x -> List.mem LambdaBody            x.aggregates) schema
let aggregation_of_constructor_expression =
  List.filter (fun x -> List.mem ConstructorExpression x.aggregates) schema
let aggregation_of_namespace_internals =
  List.filter (fun x -> List.mem NamespaceInternals    x.aggregates) schema
let aggregation_of_todo_aggregate =
  List.filter (fun x -> List.mem TODO                  x.aggregates) schema

let aggregation_of = function
  | TopLevelDeclaration    -> aggregation_of_top_level_declaration
  | Expression             -> aggregation_of_expression
  | Specifier              -> aggregation_of_specifier
  | Parameter              -> aggregation_of_parameter
  | ClassBodyDeclaration   -> aggregation_of_class_body_declaration
  | Statement              -> aggregation_of_statement
  | SwitchLabel            -> aggregation_of_switch_label
  | LambdaBody             -> aggregation_of_lambda_body
  | ConstructorExpression  -> aggregation_of_constructor_expression
  | NamespaceInternals     -> aggregation_of_namespace_internals
  | TODO                   -> aggregation_of_todo_aggregate

let aggregate_type_name = function
  | TopLevelDeclaration    -> "top_level_declaration"
  | Expression             -> "expression"
  | Specifier              -> "specifier"
  | Parameter              -> "parameter"
  | ClassBodyDeclaration   -> "class_body_declaration"
  | Statement              -> "statement"
  | SwitchLabel            -> "switch_label"
  | LambdaBody             -> "lambda_body"
  | ConstructorExpression  -> "constructor_expression"
  | NamespaceInternals     -> "namespace_internals"
  | TODO                   -> "todo_aggregate"

let aggregate_type_pfx_trim = function
  | TopLevelDeclaration    -> "TLD",    "\\(Declaration\\|Statement\\)$"
  | Expression             -> "Expr",   "Expression$"
  | Specifier              -> "Spec",   "\\(Type\\)?Specifier$"
  | Parameter              -> "Param",  ""
  | ClassBodyDeclaration   -> "Body",   "Declaration"
  | Statement              -> "Stmt",   "Statement$"
  | SwitchLabel            -> "Switch", "Label$"
  | LambdaBody             -> "Lambda", "Expression$"
  | ConstructorExpression  -> "CExpr",  "Expression$"
  | NamespaceInternals     -> "NSI",    ""
  | TODO                   -> "TODO",   ""


(******************************************************************************(
 * Useful for debugging / schema alterations
)******************************************************************************)
let string_of_child_spec =
  let p = Printf.sprintf in
  let rec aux = function
  | Token         -> "Token"
  | Just       x  -> p "Just \"%s\"" x
  | Aggregate  x  -> p "Aggregate %s" (string_of_aggregate_type x)
  | ZeroOrMore c  -> p "ZeroOrMore (%s)" (aux c)
  | ZeroOrOne  c  -> p "ZeroOrOne (%s)" (aux c)
  in
  aux

let string_of_schema s : string =
  let open Printf in
  let print_entry (a,b,c,d,(e : string list),fs) = sprintf
"( \"%s\"
    , \"%s\"
    , \"%s\"
    , \"%s\"
    , [%s%s]
    , [ %s%s]
    )"
    a
    b
    c
    d
    (String.concat ";" @@ List.map (sprintf " \"%s\"") e)
    (if e = [] then "" else " ")
    (String.concat "\n      ; " @@ List.map (fun (f,t) ->
      sprintf "\"%s\", %s" f (string_of_child_spec t)
    ) fs) @@ match fs with
    | [] -> ""
    | [_] -> " "
    | _ -> "\n      "
  in
  sprintf "  [ %s\n  ]" @@ String.concat "\n  ; " @@ List.map print_entry s
