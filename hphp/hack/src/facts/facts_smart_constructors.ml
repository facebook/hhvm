(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module Syntax = Full_fidelity_positioned_syntax
module Token = Syntax.Token
module TK = Full_fidelity_token_kind

(* result of parsing: simplified AST *)

type get_name = unit -> string [@@deriving show]

type has_script_content = bool [@@deriving show]

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
  [@@deriving show]

module SC = struct

  module Token = Syntax.Token
  type t = has_script_content [@@deriving show]

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
    let r = List.concat_map l ~f:(
      function
      | List l -> l
      | x -> if is_zero x then [] else [x]
    ) in
    match r with
    | [] -> Ignored
    | [r] -> r
    | x -> List x

  include Flatten_smart_constructors.WithOp(struct
    type r = node [@@deriving show]
    let is_zero v = is_zero v
    let flatten l = flatten l
    let zero = Ignored
  end)
  let initial_state _ = false

  let make_token token st =
    let kind = Token.kind token in
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
    (* assume file has script content if it has any tokens
       besides markup or EOF *)
    let st = st ||
      match kind with
      | TK.EndOfFile
      | TK.Markup -> false
      | _ -> true
    in
    st, result

  let make_missing _ st =
    st, Ignored

  let make_list _ items st =
    if items <> []
    then st, (if List.for_all ~f:((=) Ignored) items then Ignored else List items)
    else st, Ignored

  let make_qualified_name arg0 st =
    match arg0 with
    | Ignored -> st, Ignored
    | List nodes -> st, QualifiedName nodes
    | node -> st, QualifiedName [node]
  let make_simple_type_specifier arg0 st =
    st, arg0
  let make_literal_expression arg0 st =
    st, arg0
  let make_list_item item separator st =
    match item, separator with
    | Ignored, Ignored -> st, Ignored
    | x, Ignored | Ignored, x -> st, x
    | x, y -> st, ListItem (x, y)

  let make_generic_type_specifier class_type _argument_list st =
    st, class_type
  let make_enum_declaration _attributes _keyword name _colon _base _type
    _left_brace _enumerators _right_brace st =
    st, if name = Ignored then Ignored else EnumDecl name

  let make_alias_declaration _attributes _keyword name _generic_params _constraint
    _equal _type _semicolon st =
    st, if name = Ignored then Ignored else TypeAliasDecl name

  let make_define_expression _keyword _left_paren args _right_paren st =
    match args with
    | List [String _ as name; _] -> st, Define name
    | _ -> st, Ignored

  let make_function_declaration _attributes header body st =
    match header, body with
    | Ignored, Ignored -> st, Ignored
    | v, Ignored | Ignored, v -> st, v
    | v1, v2 -> st, List [v1; v2]

  let make_function_declaration_header _modifiers _keyword name
    _type_parameters _left_paren _param_list _right_paren
    _colon _type _where st =
    st, if name = Ignored then Ignored else FunctionDecl name

  let make_trait_use _keyword names _semicolon st =
    st, if names = Ignored then Ignored else TraitUseClause names

  let make_require_clause _keyword kind name _semicolon st =
    if name = Ignored then st, Ignored
    else
    match kind with
    | Extends -> st, RequireExtendsClause name
    | Implements -> st, RequireImplementsClause name
    | _ -> st, Ignored

  let make_constant_declarator name _initializer st =
    st, if name = Ignored then Ignored else ConstDecl name

  let make_namespace_declaration _keyword name body st =
    if body = Ignored then st, Ignored
    else st, NamespaceDecl (name, body)

  let make_namespace_body _left_brace decls _right_brace st =
    st, decls

  let make_namespace_empty_body _semicolon st =
    st, EmptyBody

  let make_methodish_declaration _attributes _function_decl_header body
    _semicolon st =
    if body = Ignored then st, Ignored
    else st, MethodDecl body

  let make_classish_declaration _attributes modifiers keyword name _type_parameters
    _extends_keyword extends _implements_keyword implements body st =
    if name = Ignored then st, Ignored
    else st, ClassDecl { modifiers; kind = keyword; name; extends; implements; body }

  let make_classish_body _left_brace elements _right_brace st =
    st, elements

end
