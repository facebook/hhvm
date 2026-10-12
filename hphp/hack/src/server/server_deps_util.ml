(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let symbol_in_call_hierarchy (sym_occ : Relative_path.t Symbol_occurrence.t) :
    bool =
  let open Symbol_occurrence in
  match sym_occ.type_ with
  | Class _ -> true
  | BuiltInType _ -> false
  | Function -> true
  | Method _ -> true
  | LocalVar -> false
  | TypeVar -> false
  | Property _ -> true
  | XhpLiteralAttr _ -> false
  | ClassConst _ -> false
  | Typeconst _ -> false
  | GConst -> false
  | Attribute _ -> true
  | EnumClassLabel _ -> true
  | Keyword _ -> false
  | PureFunctionContext -> false
  | BestEffortArgument _ -> false
  | HhFixme -> false
  | HhIgnore -> false
  | Module -> true

let is_target
    target_line target_char (occ : Relative_path.t Symbol_occurrence.t) =
  let open Symbol_occurrence in
  let pos = occ.pos in
  let (l, start, end_) = Pos.info_pos pos in
  l = target_line && start <= target_char && target_char - 1 <= end_

let body_symbols
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    (filter_against : Relative_path.t Symbol_occurrence.t list)
    (occ : Relative_path.t Symbol_occurrence.t)
    (def : Relative_path.t Symbol_definition.t) :
    Relative_path.t Symbol_occurrence.t list =
  let open Symbol_occurrence in
  let open Symbol_definition in
  let node_opt =
    Server_symbol_definition.get_definition_cst_node_ctx
      ~ctx
      ~entry
      ~kind:def.kind
      ~pos:def.pos
  in
  match node_opt with
  | None -> []
  | Some node ->
    let span_pos_opt =
      Full_fidelity_positioned_syntax.position (Pos.filename def.pos) node
    in
    (match span_pos_opt with
    | None -> []
    | Some span_pos ->
      let pos_filter (o : Relative_path.t Symbol_occurrence.t) =
        (not (phys_equal o occ)) && Pos.contains span_pos o.pos
      in
      List.filter filter_against ~f:pos_filter)

let get_ast_getdef (ctx : Provider_context.t) (entry : Provider_context.entry) =
  let ast =
    Ast_provider.compute_ast ~popt:(Provider_context.get_popt ctx) ~entry
  in
  let get_def = Server_symbol_definition.go ctx (Some ast) in
  (ast, get_def)

let get_def_setup (acc_ctx_in : Provider_context.t) (file : string) =
  let path = Relative_path.create_detect_prefix file in
  let (acc_ctx_out, entry) =
    Provider_context.add_entry_if_missing ~ctx:acc_ctx_in ~path
  in
  let (ast, get_def) = get_ast_getdef acc_ctx_out entry in
  (acc_ctx_out, entry, ast, get_def)
