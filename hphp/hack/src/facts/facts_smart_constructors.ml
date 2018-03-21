(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Syntax = Full_fidelity_positioned_syntax
module Token = Syntax.Token
module TK = Full_fidelity_token_kind

(* result of parsing: simplified AST *)

type get_name = unit -> string

type node =
  | Ignored
  | List of node list
  (* tokens *)
  | Name of get_name
  | String of get_name
  | XhpName of get_name
  | Backslash
  | ListItem of node * node
  | Class
  | Interface
  | Trait
  | Extends
  | Implements
  | Abstract
  | Final
  | Static
  | QualifiedName of node list
  (* declarations *)
  | ClassDecl of {
      modifiers: node;
      kind: node;
      name: node;
      extends: node;
      implements: node;
      body: node
    }
  | FunctionDecl of node
  | MethodDecl of node
  | EnumDecl of node
  | TraitUseClause of node
  | RequireExtendsClause of node
  | RequireImplementsClause of node
  | ConstDecl of node
  | Define of node
  | TypeAliasDecl of node
  | NamespaceDecl of node * node
  | EmptyBody

module SC = struct

  module Token = Syntax.Token
  type t = unit

  let is_zero = function
    | Ignored
    (* tokens *)
    | Name _
    | String _
    | XhpName _
    | Backslash
    | ListItem _
    | Class
    | Interface
    | Trait
    | Extends
    | Implements
    | Abstract
    | Final
    | Static
    | QualifiedName _ -> true
    | _ -> false

  let flatten l =
    let r = Core_list.concat_map l ~f:(
      function
      | List l -> l
      | x -> if is_zero x then [] else [x]
    ) in
    match r with
    | [] -> Ignored
    | [r] -> r
    | x -> List x

  include Flatten_smart_constructors.WithOp(struct
    type r = node
    let is_zero v = is_zero v
    let flatten l = flatten l
    let zero = Ignored
  end)
  let initial_state _ = ()

  let make_token token () =
    let result =
      match Token.kind token with
      | TK.Name -> Name (fun () -> Token.text token)
      | TK.SingleQuotedStringLiteral
      | TK.DoubleQuotedStringLiteral -> String (fun () -> Token.text token)
      | TK.XHPClassName -> XhpName (fun () -> Token.text token)
      | TK.Backslash -> Backslash
      | TK.Class -> Class
      | TK.Trait -> Trait
      | TK.Interface -> Interface
      | TK.Extends -> Extends
      | TK.Implements -> Implements
      | TK.Abstract -> Abstract
      | TK.Final -> Final
      | TK.Static -> Static
      | _ -> Ignored in
    (), result

  let make_missing _ () =
    (), Ignored

  let make_list _ items () =
    if items <> []
    then (), (if Core_list.for_all ~f:((=) Ignored) items then Ignored else List items)
    else (), Ignored

  let make_qualified_name arg0 () =
    match arg0 with
    | Ignored -> (), Ignored
    | List nodes -> (), QualifiedName nodes
    | node -> (), QualifiedName [node]
  let make_simple_type_specifier arg0 () =
    (), arg0
  let make_literal_expression arg0 () =
    (), arg0
  let make_list_item item separator () =
    match item, separator with
    | Ignored, Ignored -> (), Ignored
    | x, Ignored | Ignored, x -> (), x
    | x, y -> (), ListItem (x, y)

  let make_generic_type_specifier class_type _argument_list () =
    (), class_type
  let make_enum_declaration _attributes _keyword name _colon _base _type
    _left_brace _enumerators _right_brace () =
    (), if name = Ignored then Ignored else EnumDecl name

  let make_alias_declaration _attributes _keyword name _generic_params _constraint
    _equal _type _semicolon () =
    (), if name = Ignored then Ignored else TypeAliasDecl name

  let make_define_expression _keyword _left_paren args _right_paren () =
    match args with
    | List [String _ as name; _] -> (), Define name
    | _ -> (), Ignored

  let make_function_declaration _attributes header body () =
    match header, body with
    | Ignored, Ignored -> (), Ignored
    | v, Ignored | Ignored, v -> (), v
    | v1, v2 -> (), List [v1; v2]

  let make_function_declaration_header _modifiers _keyword _ampersand name
    _type_parameters _left_paren _param_list _right_paren
    _colon _type _where () =
    (), if name = Ignored then Ignored else FunctionDecl name

  let make_trait_use _keyword names _semicolon () =
    (), if names = Ignored then Ignored else TraitUseClause names

  let make_require_clause _keyword kind name _semicolon () =
    if name = Ignored then (), Ignored
    else
    match kind with
    | Extends -> (), RequireExtendsClause name
    | Implements -> (), RequireImplementsClause name
    | _ -> (), Ignored

  let make_constant_declarator name _initializer () =
    (), if name = Ignored then Ignored else ConstDecl name

  let make_namespace_declaration _keyword name body () =
    if body = Ignored then (), Ignored
    else (), NamespaceDecl (name, body)

  let make_namespace_body _left_brace decls _right_brace () =
    (), decls

  let make_namespace_empty_body _semicolon () =
    (), EmptyBody

  let make_methodish_declaration _attributes _function_decl_header body
    _semicolon () =
    if body = Ignored then (), Ignored
    else (), MethodDecl body

  let make_classish_declaration _attributes modifiers keyword name _type_parameters
    _extends_keyword extends _implements_keyword implements body () =
    if name = Ignored then (), Ignored
    else (), ClassDecl { modifiers; kind = keyword; name; extends; implements; body }

  let make_classish_body _left_brace elements _right_brace () =
    (), elements

end
