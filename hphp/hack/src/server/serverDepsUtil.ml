(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let is_target target_line target_char (occ : Relative_path.t SymbolOccurrence.t)
    =
  let open SymbolOccurrence in
  let pos = occ.pos in
  let (l, start, end_) = Pos.info_pos pos in
  l = target_line && start <= target_char && target_char - 1 <= end_

let body_symbols
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    (declarations : Relative_path.t SymbolOccurrence.t list)
    (occ : Relative_path.t SymbolOccurrence.t)
    (def : Relative_path.t SymbolDefinition.t) :
    Relative_path.t SymbolOccurrence.t list =
  let open SymbolOccurrence in
  let open SymbolDefinition in
  let node_opt =
    ServerSymbolDefinition.get_definition_cst_node_ctx
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
      let pos_filter (o : Relative_path.t SymbolOccurrence.t) =
        (not (phys_equal o occ)) && Pos.contains span_pos o.pos
      in
      List.filter declarations ~f:pos_filter)
