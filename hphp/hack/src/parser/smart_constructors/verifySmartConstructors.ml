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
 * This module contains smart constructors implementation that can be used to
 * build AST.
 
 *)

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
  module Token = Syntax.Token
  type t = Syntax.t list [@@deriving show]
  type r = Syntax.t [@@deriving show]

  exception NotEquals of
    string * Syntax.t list * Syntax.t list * Syntax.t list
  exception NotPhysicallyEquals of
    string * Syntax.t list * Syntax.t list * Syntax.t list

  let verify ~stack params args cons_name =
    let equals e1 e2 =
      if e1 != e2 then
        if e1 = e2
        then
          raise @@ NotPhysicallyEquals
            (cons_name
            , List.rev stack
            , params
            , args
            )
        else
          raise @@ NotEquals
            (cons_name
            , List.rev stack
            , params
            , args
            )
    in
    Core_list.iter2_exn ~f:equals params args

  let initial_state _ = []

  let make_token token stack =
    let token = Syntax.make_token token in
    token :: stack, token

  let make_missing (s, o) stack =
    let missing = Syntax.make_missing s o in
    missing :: stack, missing

  let make_list (s, o) items stack =
    if items <> [] then
      let (h, t) = Core_list.split_n stack (List.length items) in
      let () = verify ~stack items (List.rev h) "list" in
      let lst = Syntax.make_list s o items in
      lst :: t, lst
    else make_missing (s, o) stack

  let make_end_of_file p0 stack =
    match stack with
    | a0 :: rem ->
      let () = verify ~stack [p0] [a0] "end_of_file" in
      let node = Syntax.make_end_of_file p0 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_script p0 stack =
    match stack with
    | a0 :: rem ->
      let () = verify ~stack [p0] [a0] "script" in
      let node = Syntax.make_script p0 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_qualified_name p0 stack =
    match stack with
    | a0 :: rem ->
      let () = verify ~stack [p0] [a0] "qualified_name" in
      let node = Syntax.make_qualified_name p0 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_simple_type_specifier p0 stack =
    match stack with
    | a0 :: rem ->
      let () = verify ~stack [p0] [a0] "simple_type_specifier" in
      let node = Syntax.make_simple_type_specifier p0 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_literal_expression p0 stack =
    match stack with
    | a0 :: rem ->
      let () = verify ~stack [p0] [a0] "literal_expression" in
      let node = Syntax.make_literal_expression p0 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_prefixed_string_expression p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "prefixed_string_expression" in
      let node = Syntax.make_prefixed_string_expression p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_variable_expression p0 stack =
    match stack with
    | a0 :: rem ->
      let () = verify ~stack [p0] [a0] "variable_expression" in
      let node = Syntax.make_variable_expression p0 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_pipe_variable_expression p0 stack =
    match stack with
    | a0 :: rem ->
      let () = verify ~stack [p0] [a0] "pipe_variable_expression" in
      let node = Syntax.make_pipe_variable_expression p0 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_enum_declaration p0 p1 p2 p3 p4 p5 p6 p7 p8 stack =
    match stack with
    | a8 :: a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7; p8] [a0; a1; a2; a3; a4; a5; a6; a7; a8] "enum_declaration" in
      let node = Syntax.make_enum_declaration p0 p1 p2 p3 p4 p5 p6 p7 p8 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_enumerator p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "enumerator" in
      let node = Syntax.make_enumerator p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_alias_declaration p0 p1 p2 p3 p4 p5 p6 p7 stack =
    match stack with
    | a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7] [a0; a1; a2; a3; a4; a5; a6; a7] "alias_declaration" in
      let node = Syntax.make_alias_declaration p0 p1 p2 p3 p4 p5 p6 p7 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_property_declaration p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "property_declaration" in
      let node = Syntax.make_property_declaration p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_property_declarator p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "property_declarator" in
      let node = Syntax.make_property_declarator p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_namespace_declaration p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "namespace_declaration" in
      let node = Syntax.make_namespace_declaration p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_namespace_body p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "namespace_body" in
      let node = Syntax.make_namespace_body p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_namespace_empty_body p0 stack =
    match stack with
    | a0 :: rem ->
      let () = verify ~stack [p0] [a0] "namespace_empty_body" in
      let node = Syntax.make_namespace_empty_body p0 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_namespace_use_declaration p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "namespace_use_declaration" in
      let node = Syntax.make_namespace_use_declaration p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_namespace_group_use_declaration p0 p1 p2 p3 p4 p5 p6 stack =
    match stack with
    | a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6] [a0; a1; a2; a3; a4; a5; a6] "namespace_group_use_declaration" in
      let node = Syntax.make_namespace_group_use_declaration p0 p1 p2 p3 p4 p5 p6 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_namespace_use_clause p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "namespace_use_clause" in
      let node = Syntax.make_namespace_use_clause p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_function_declaration p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "function_declaration" in
      let node = Syntax.make_function_declaration p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_function_declaration_header p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 stack =
    match stack with
    | a10 :: a9 :: a8 :: a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7; p8; p9; p10] [a0; a1; a2; a3; a4; a5; a6; a7; a8; a9; a10] "function_declaration_header" in
      let node = Syntax.make_function_declaration_header p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_where_clause p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "where_clause" in
      let node = Syntax.make_where_clause p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_where_constraint p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "where_constraint" in
      let node = Syntax.make_where_constraint p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_methodish_declaration p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "methodish_declaration" in
      let node = Syntax.make_methodish_declaration p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_classish_declaration p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 stack =
    match stack with
    | a9 :: a8 :: a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7; p8; p9] [a0; a1; a2; a3; a4; a5; a6; a7; a8; a9] "classish_declaration" in
      let node = Syntax.make_classish_declaration p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_classish_body p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "classish_body" in
      let node = Syntax.make_classish_body p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_trait_use_precedence_item p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "trait_use_precedence_item" in
      let node = Syntax.make_trait_use_precedence_item p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_trait_use_alias_item p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "trait_use_alias_item" in
      let node = Syntax.make_trait_use_alias_item p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_trait_use_conflict_resolution p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "trait_use_conflict_resolution" in
      let node = Syntax.make_trait_use_conflict_resolution p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_trait_use p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "trait_use" in
      let node = Syntax.make_trait_use p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_require_clause p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "require_clause" in
      let node = Syntax.make_require_clause p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_const_declaration p0 p1 p2 p3 p4 p5 stack =
    match stack with
    | a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5] [a0; a1; a2; a3; a4; a5] "const_declaration" in
      let node = Syntax.make_const_declaration p0 p1 p2 p3 p4 p5 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_constant_declarator p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "constant_declarator" in
      let node = Syntax.make_constant_declarator p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_type_const_declaration p0 p1 p2 p3 p4 p5 p6 p7 p8 stack =
    match stack with
    | a8 :: a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7; p8] [a0; a1; a2; a3; a4; a5; a6; a7; a8] "type_const_declaration" in
      let node = Syntax.make_type_const_declaration p0 p1 p2 p3 p4 p5 p6 p7 p8 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_decorated_expression p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "decorated_expression" in
      let node = Syntax.make_decorated_expression p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_parameter_declaration p0 p1 p2 p3 p4 p5 stack =
    match stack with
    | a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5] [a0; a1; a2; a3; a4; a5] "parameter_declaration" in
      let node = Syntax.make_parameter_declaration p0 p1 p2 p3 p4 p5 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_variadic_parameter p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "variadic_parameter" in
      let node = Syntax.make_variadic_parameter p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_attribute_specification p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "attribute_specification" in
      let node = Syntax.make_attribute_specification p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_attribute p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "attribute" in
      let node = Syntax.make_attribute p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_inclusion_expression p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "inclusion_expression" in
      let node = Syntax.make_inclusion_expression p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_inclusion_directive p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "inclusion_directive" in
      let node = Syntax.make_inclusion_directive p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_compound_statement p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "compound_statement" in
      let node = Syntax.make_compound_statement p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_alternate_loop_statement p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "alternate_loop_statement" in
      let node = Syntax.make_alternate_loop_statement p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_expression_statement p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "expression_statement" in
      let node = Syntax.make_expression_statement p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_markup_section p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "markup_section" in
      let node = Syntax.make_markup_section p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_markup_suffix p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "markup_suffix" in
      let node = Syntax.make_markup_suffix p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_unset_statement p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "unset_statement" in
      let node = Syntax.make_unset_statement p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_let_statement p0 p1 p2 p3 p4 p5 stack =
    match stack with
    | a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5] [a0; a1; a2; a3; a4; a5] "let_statement" in
      let node = Syntax.make_let_statement p0 p1 p2 p3 p4 p5 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_using_statement_block_scoped p0 p1 p2 p3 p4 p5 stack =
    match stack with
    | a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5] [a0; a1; a2; a3; a4; a5] "using_statement_block_scoped" in
      let node = Syntax.make_using_statement_block_scoped p0 p1 p2 p3 p4 p5 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_using_statement_function_scoped p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "using_statement_function_scoped" in
      let node = Syntax.make_using_statement_function_scoped p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_declare_directive_statement p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "declare_directive_statement" in
      let node = Syntax.make_declare_directive_statement p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_declare_block_statement p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "declare_block_statement" in
      let node = Syntax.make_declare_block_statement p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_while_statement p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "while_statement" in
      let node = Syntax.make_while_statement p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_if_statement p0 p1 p2 p3 p4 p5 p6 stack =
    match stack with
    | a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6] [a0; a1; a2; a3; a4; a5; a6] "if_statement" in
      let node = Syntax.make_if_statement p0 p1 p2 p3 p4 p5 p6 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_elseif_clause p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "elseif_clause" in
      let node = Syntax.make_elseif_clause p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_else_clause p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "else_clause" in
      let node = Syntax.make_else_clause p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_alternate_if_statement p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 stack =
    match stack with
    | a9 :: a8 :: a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7; p8; p9] [a0; a1; a2; a3; a4; a5; a6; a7; a8; a9] "alternate_if_statement" in
      let node = Syntax.make_alternate_if_statement p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_alternate_elseif_clause p0 p1 p2 p3 p4 p5 stack =
    match stack with
    | a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5] [a0; a1; a2; a3; a4; a5] "alternate_elseif_clause" in
      let node = Syntax.make_alternate_elseif_clause p0 p1 p2 p3 p4 p5 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_alternate_else_clause p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "alternate_else_clause" in
      let node = Syntax.make_alternate_else_clause p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_try_statement p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "try_statement" in
      let node = Syntax.make_try_statement p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_catch_clause p0 p1 p2 p3 p4 p5 stack =
    match stack with
    | a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5] [a0; a1; a2; a3; a4; a5] "catch_clause" in
      let node = Syntax.make_catch_clause p0 p1 p2 p3 p4 p5 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_finally_clause p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "finally_clause" in
      let node = Syntax.make_finally_clause p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_do_statement p0 p1 p2 p3 p4 p5 p6 stack =
    match stack with
    | a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6] [a0; a1; a2; a3; a4; a5; a6] "do_statement" in
      let node = Syntax.make_do_statement p0 p1 p2 p3 p4 p5 p6 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_for_statement p0 p1 p2 p3 p4 p5 p6 p7 p8 stack =
    match stack with
    | a8 :: a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7; p8] [a0; a1; a2; a3; a4; a5; a6; a7; a8] "for_statement" in
      let node = Syntax.make_for_statement p0 p1 p2 p3 p4 p5 p6 p7 p8 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_foreach_statement p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 stack =
    match stack with
    | a9 :: a8 :: a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7; p8; p9] [a0; a1; a2; a3; a4; a5; a6; a7; a8; a9] "foreach_statement" in
      let node = Syntax.make_foreach_statement p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_switch_statement p0 p1 p2 p3 p4 p5 p6 stack =
    match stack with
    | a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6] [a0; a1; a2; a3; a4; a5; a6] "switch_statement" in
      let node = Syntax.make_switch_statement p0 p1 p2 p3 p4 p5 p6 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_alternate_switch_statement p0 p1 p2 p3 p4 p5 p6 p7 stack =
    match stack with
    | a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7] [a0; a1; a2; a3; a4; a5; a6; a7] "alternate_switch_statement" in
      let node = Syntax.make_alternate_switch_statement p0 p1 p2 p3 p4 p5 p6 p7 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_switch_section p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "switch_section" in
      let node = Syntax.make_switch_section p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_switch_fallthrough p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "switch_fallthrough" in
      let node = Syntax.make_switch_fallthrough p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_case_label p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "case_label" in
      let node = Syntax.make_case_label p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_default_label p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "default_label" in
      let node = Syntax.make_default_label p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_return_statement p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "return_statement" in
      let node = Syntax.make_return_statement p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_goto_label p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "goto_label" in
      let node = Syntax.make_goto_label p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_goto_statement p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "goto_statement" in
      let node = Syntax.make_goto_statement p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_throw_statement p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "throw_statement" in
      let node = Syntax.make_throw_statement p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_break_statement p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "break_statement" in
      let node = Syntax.make_break_statement p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_continue_statement p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "continue_statement" in
      let node = Syntax.make_continue_statement p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_function_static_statement p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "function_static_statement" in
      let node = Syntax.make_function_static_statement p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_static_declarator p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "static_declarator" in
      let node = Syntax.make_static_declarator p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_echo_statement p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "echo_statement" in
      let node = Syntax.make_echo_statement p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_global_statement p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "global_statement" in
      let node = Syntax.make_global_statement p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_simple_initializer p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "simple_initializer" in
      let node = Syntax.make_simple_initializer p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_anonymous_class p0 p1 p2 p3 p4 p5 p6 p7 p8 stack =
    match stack with
    | a8 :: a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7; p8] [a0; a1; a2; a3; a4; a5; a6; a7; a8] "anonymous_class" in
      let node = Syntax.make_anonymous_class p0 p1 p2 p3 p4 p5 p6 p7 p8 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_anonymous_function p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 stack =
    match stack with
    | a12 :: a11 :: a10 :: a9 :: a8 :: a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7; p8; p9; p10; p11; p12] [a0; a1; a2; a3; a4; a5; a6; a7; a8; a9; a10; a11; a12] "anonymous_function" in
      let node = Syntax.make_anonymous_function p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_php7_anonymous_function p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 stack =
    match stack with
    | a12 :: a11 :: a10 :: a9 :: a8 :: a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7; p8; p9; p10; p11; p12] [a0; a1; a2; a3; a4; a5; a6; a7; a8; a9; a10; a11; a12] "php7_anonymous_function" in
      let node = Syntax.make_php7_anonymous_function p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_anonymous_function_use_clause p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "anonymous_function_use_clause" in
      let node = Syntax.make_anonymous_function_use_clause p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_lambda_expression p0 p1 p2 p3 p4 p5 stack =
    match stack with
    | a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5] [a0; a1; a2; a3; a4; a5] "lambda_expression" in
      let node = Syntax.make_lambda_expression p0 p1 p2 p3 p4 p5 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_lambda_signature p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "lambda_signature" in
      let node = Syntax.make_lambda_signature p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_cast_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "cast_expression" in
      let node = Syntax.make_cast_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_scope_resolution_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "scope_resolution_expression" in
      let node = Syntax.make_scope_resolution_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_member_selection_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "member_selection_expression" in
      let node = Syntax.make_member_selection_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_safe_member_selection_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "safe_member_selection_expression" in
      let node = Syntax.make_safe_member_selection_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_embedded_member_selection_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "embedded_member_selection_expression" in
      let node = Syntax.make_embedded_member_selection_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_yield_expression p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "yield_expression" in
      let node = Syntax.make_yield_expression p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_yield_from_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "yield_from_expression" in
      let node = Syntax.make_yield_from_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_prefix_unary_expression p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "prefix_unary_expression" in
      let node = Syntax.make_prefix_unary_expression p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_postfix_unary_expression p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "postfix_unary_expression" in
      let node = Syntax.make_postfix_unary_expression p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_binary_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "binary_expression" in
      let node = Syntax.make_binary_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_instanceof_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "instanceof_expression" in
      let node = Syntax.make_instanceof_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_is_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "is_expression" in
      let node = Syntax.make_is_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_as_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "as_expression" in
      let node = Syntax.make_as_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_nullable_as_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "nullable_as_expression" in
      let node = Syntax.make_nullable_as_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_conditional_expression p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "conditional_expression" in
      let node = Syntax.make_conditional_expression p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_eval_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "eval_expression" in
      let node = Syntax.make_eval_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_empty_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "empty_expression" in
      let node = Syntax.make_empty_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_define_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "define_expression" in
      let node = Syntax.make_define_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_halt_compiler_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "halt_compiler_expression" in
      let node = Syntax.make_halt_compiler_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_isset_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "isset_expression" in
      let node = Syntax.make_isset_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_function_call_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "function_call_expression" in
      let node = Syntax.make_function_call_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_function_call_with_type_arguments_expression p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "function_call_with_type_arguments_expression" in
      let node = Syntax.make_function_call_with_type_arguments_expression p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_parenthesized_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "parenthesized_expression" in
      let node = Syntax.make_parenthesized_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_braced_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "braced_expression" in
      let node = Syntax.make_braced_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_embedded_braced_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "embedded_braced_expression" in
      let node = Syntax.make_embedded_braced_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_list_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "list_expression" in
      let node = Syntax.make_list_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_collection_literal_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "collection_literal_expression" in
      let node = Syntax.make_collection_literal_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_object_creation_expression p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "object_creation_expression" in
      let node = Syntax.make_object_creation_expression p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_constructor_call p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "constructor_call" in
      let node = Syntax.make_constructor_call p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_array_creation_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "array_creation_expression" in
      let node = Syntax.make_array_creation_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_array_intrinsic_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "array_intrinsic_expression" in
      let node = Syntax.make_array_intrinsic_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_darray_intrinsic_expression p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "darray_intrinsic_expression" in
      let node = Syntax.make_darray_intrinsic_expression p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_dictionary_intrinsic_expression p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "dictionary_intrinsic_expression" in
      let node = Syntax.make_dictionary_intrinsic_expression p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_keyset_intrinsic_expression p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "keyset_intrinsic_expression" in
      let node = Syntax.make_keyset_intrinsic_expression p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_varray_intrinsic_expression p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "varray_intrinsic_expression" in
      let node = Syntax.make_varray_intrinsic_expression p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_vector_intrinsic_expression p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "vector_intrinsic_expression" in
      let node = Syntax.make_vector_intrinsic_expression p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_element_initializer p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "element_initializer" in
      let node = Syntax.make_element_initializer p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_subscript_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "subscript_expression" in
      let node = Syntax.make_subscript_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_embedded_subscript_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "embedded_subscript_expression" in
      let node = Syntax.make_embedded_subscript_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_awaitable_creation_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "awaitable_creation_expression" in
      let node = Syntax.make_awaitable_creation_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_children_declaration p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "xhp_children_declaration" in
      let node = Syntax.make_xhp_children_declaration p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_children_parenthesized_list p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "xhp_children_parenthesized_list" in
      let node = Syntax.make_xhp_children_parenthesized_list p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_category_declaration p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "xhp_category_declaration" in
      let node = Syntax.make_xhp_category_declaration p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_enum_type p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "xhp_enum_type" in
      let node = Syntax.make_xhp_enum_type p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_required p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "xhp_required" in
      let node = Syntax.make_xhp_required p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_class_attribute_declaration p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "xhp_class_attribute_declaration" in
      let node = Syntax.make_xhp_class_attribute_declaration p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_class_attribute p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "xhp_class_attribute" in
      let node = Syntax.make_xhp_class_attribute p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_simple_class_attribute p0 stack =
    match stack with
    | a0 :: rem ->
      let () = verify ~stack [p0] [a0] "xhp_simple_class_attribute" in
      let node = Syntax.make_xhp_simple_class_attribute p0 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_simple_attribute p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "xhp_simple_attribute" in
      let node = Syntax.make_xhp_simple_attribute p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_spread_attribute p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "xhp_spread_attribute" in
      let node = Syntax.make_xhp_spread_attribute p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_open p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "xhp_open" in
      let node = Syntax.make_xhp_open p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_expression p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "xhp_expression" in
      let node = Syntax.make_xhp_expression p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_xhp_close p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "xhp_close" in
      let node = Syntax.make_xhp_close p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_type_constant p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "type_constant" in
      let node = Syntax.make_type_constant p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_vector_type_specifier p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "vector_type_specifier" in
      let node = Syntax.make_vector_type_specifier p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_keyset_type_specifier p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "keyset_type_specifier" in
      let node = Syntax.make_keyset_type_specifier p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_tuple_type_explicit_specifier p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "tuple_type_explicit_specifier" in
      let node = Syntax.make_tuple_type_explicit_specifier p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_varray_type_specifier p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "varray_type_specifier" in
      let node = Syntax.make_varray_type_specifier p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_vector_array_type_specifier p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "vector_array_type_specifier" in
      let node = Syntax.make_vector_array_type_specifier p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_type_parameter p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "type_parameter" in
      let node = Syntax.make_type_parameter p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_type_constraint p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "type_constraint" in
      let node = Syntax.make_type_constraint p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_darray_type_specifier p0 p1 p2 p3 p4 p5 p6 stack =
    match stack with
    | a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6] [a0; a1; a2; a3; a4; a5; a6] "darray_type_specifier" in
      let node = Syntax.make_darray_type_specifier p0 p1 p2 p3 p4 p5 p6 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_map_array_type_specifier p0 p1 p2 p3 p4 p5 stack =
    match stack with
    | a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5] [a0; a1; a2; a3; a4; a5] "map_array_type_specifier" in
      let node = Syntax.make_map_array_type_specifier p0 p1 p2 p3 p4 p5 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_dictionary_type_specifier p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "dictionary_type_specifier" in
      let node = Syntax.make_dictionary_type_specifier p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_closure_type_specifier p0 p1 p2 p3 p4 p5 p6 p7 p8 stack =
    match stack with
    | a8 :: a7 :: a6 :: a5 :: a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4; p5; p6; p7; p8] [a0; a1; a2; a3; a4; a5; a6; a7; a8] "closure_type_specifier" in
      let node = Syntax.make_closure_type_specifier p0 p1 p2 p3 p4 p5 p6 p7 p8 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_closure_parameter_type_specifier p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "closure_parameter_type_specifier" in
      let node = Syntax.make_closure_parameter_type_specifier p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_classname_type_specifier p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "classname_type_specifier" in
      let node = Syntax.make_classname_type_specifier p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_field_specifier p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "field_specifier" in
      let node = Syntax.make_field_specifier p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_field_initializer p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "field_initializer" in
      let node = Syntax.make_field_initializer p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_shape_type_specifier p0 p1 p2 p3 p4 stack =
    match stack with
    | a4 :: a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3; p4] [a0; a1; a2; a3; a4] "shape_type_specifier" in
      let node = Syntax.make_shape_type_specifier p0 p1 p2 p3 p4 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_shape_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "shape_expression" in
      let node = Syntax.make_shape_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_tuple_expression p0 p1 p2 p3 stack =
    match stack with
    | a3 :: a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2; p3] [a0; a1; a2; a3] "tuple_expression" in
      let node = Syntax.make_tuple_expression p0 p1 p2 p3 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_generic_type_specifier p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "generic_type_specifier" in
      let node = Syntax.make_generic_type_specifier p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_nullable_type_specifier p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "nullable_type_specifier" in
      let node = Syntax.make_nullable_type_specifier p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_soft_type_specifier p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "soft_type_specifier" in
      let node = Syntax.make_soft_type_specifier p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_reified_type_argument p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "reified_type_argument" in
      let node = Syntax.make_reified_type_argument p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_type_arguments p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "type_arguments" in
      let node = Syntax.make_type_arguments p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_type_parameters p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "type_parameters" in
      let node = Syntax.make_type_parameters p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_tuple_type_specifier p0 p1 p2 stack =
    match stack with
    | a2 :: a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1; p2] [a0; a1; a2] "tuple_type_specifier" in
      let node = Syntax.make_tuple_type_specifier p0 p1 p2 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_error p0 stack =
    match stack with
    | a0 :: rem ->
      let () = verify ~stack [p0] [a0] "error" in
      let node = Syntax.make_error p0 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
  let make_list_item p0 p1 stack =
    match stack with
    | a1 :: a0 :: rem ->
      let () = verify ~stack [p0; p1] [a0; a1] "list_item" in
      let node = Syntax.make_list_item p0 p1 in
      node :: rem, node
    | _ -> failwith "Unexpected stack state"
    
end (* WithSyntax *)
