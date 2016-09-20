(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(** Traverses a full fidelity tree and produces the corresponding
 * classic Parser.Ast *)

module SyntaxTree = Full_fidelity_syntax_tree
module Syntax = Full_fidelity_positioned_syntax
module Trivia = Full_fidelity_positioned_trivia
module TokenKind = Full_fidelity_token_kind
module SyntaxKind = Full_fidelity_syntax_kind

open Syntax

open Core

type env = {
  language : string;
  mode : string
}

let missing_text = "MISSING"

(* TODO: Positions are dropped entirely right now. *)
let pos _node = Pos.none

let happly_hint node =
  pos node, Ast.Happly ((pos node, Syntax.text node), [])

let f_mode s = match s with
  | "strict" -> FileInfo.Mstrict
  | "decl" -> FileInfo.Mdecl
  (** TODO: error? *)
  | _ -> FileInfo.Mpartial

let f_name _env node = pos node, Syntax.text node

let f_tparams _env _node =
  (** TODO *)
  []

let f_ret env node = match (Syntax.syntax node) with
  | SimpleTypeSpecifier c -> Some (happly_hint c.simple_type_specifier)
  | _ ->
    (** TODO *)
    None

let f_ret_by_ref _env _node =
  (** TODO *)
  false

let f_body _env _node =
  []

let hint _env node = match Syntax.syntax node with
  | SimpleTypeSpecifier c -> Some (happly_hint c.simple_type_specifier)
  | _ ->
      None

let rec f_param _env acc node =
  let open Ast in
  match Syntax.syntax node with
  | ParameterDeclaration record ->
    let name = match (Syntax.syntax record.param_name) with
      | VariableExpression _ -> Syntax.text record.param_name
      | _ -> ""
    in
    let param_id = (pos record.param_name, name) in
    let param_hint = hint _env record.param_type in
    (** TODOs *)
    let is_reference = false in
    let is_variadic = false in
    let param_expr = None in
    let param_modifier = None in
    let user_attributes = [] in
    let result = {
      param_hint = param_hint;
      param_is_reference = is_reference;
      param_is_variadic = is_variadic;
      param_id = param_id;
      param_expr = param_expr;
      param_modifier = param_modifier;
      param_user_attributes = user_attributes;
    }
    in
    acc @ [result]
  | ListItem item -> f_param _env acc item.list_item
  | _ ->
    (** TODO: Error instead of failing silently. *)
    acc

let f_params env node = match Syntax.syntax node with
  | SyntaxList l ->
      List.fold_left l ~init:[] ~f: begin fun acc c ->
        f_param env acc c
      end
  | ParameterDeclaration _ -> f_param env [] node
  | _ ->
      (** TODO: error *)
      []

let functionDeclaration env node decl =
  let open Ast in
  match Syntax.syntax decl.function_declaration_header with
  | FunctionDeclarationHeader
  { function_async; function_keyword; function_name;
    function_type_parameter_list; function_left_paren; function_parameter_list;
    function_right_paren; function_colon; function_type } ->
    let f_mode = f_mode env.mode in
    let f_tparams = f_tparams env function_type_parameter_list in
    let f_ret = f_ret env function_type in
    let f_ret_by_ref = f_ret_by_ref env function_type in
    let f_name = f_name env function_name in
    let f_params = f_params env function_parameter_list in
    let f_body = f_body env decl.function_body in
    (** TODOs *)
    let f_user_attributes = [] in
    let f_fun_kind = Ast.FSync in
    (* FIXME: Don't use the default popt *)
    let f_namespace = Namespace_env.empty ParserOptions.default in
    let f_span = pos node in
    {
      f_mode = f_mode;
      f_tparams = f_tparams;
      f_ret = f_ret;
      f_ret_by_ref = f_ret_by_ref;
      f_name = f_name;
      f_params = f_params;
      f_body = f_body;
      f_user_attributes = f_user_attributes;
      f_fun_kind = f_fun_kind;
      f_namespace = f_namespace;
      f_span = f_span;
    }
  | _ -> assert false (* TODO what to do when AST from new parser is wrong *)

(** Top-level declarations, excluding the header.
 *
 * e.g. class, interface, function *)
let to_ast_def env node: Ast.def =
  let node = Syntax.syntax node in
  match node with
    | FunctionDeclaration decl -> Ast.Fun (functionDeclaration env node decl)
    | _ -> assert false

let script env tree = match Syntax.syntax tree with
  | Script { script_header; script_declarations } ->
    let _ = script_header in
    let node = Syntax.syntax script_declarations in
    begin match node with
      | SyntaxList l ->
        List.fold_left l ~init:[] ~f: begin fun acc node ->
          acc @ [to_ast_def env node]
        end
      | _ -> [to_ast_def env script_declarations]
    end
  | _ -> (** TODO: errors *) assert false

let from_tree minimal_tree: Ast.program =
  let tree = Syntax.from_tree minimal_tree in
    let env = { language = (SyntaxTree.language minimal_tree);
      mode = (SyntaxTree.mode minimal_tree) } in
  script env tree
