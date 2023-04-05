(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*Assusmes no occ_list is empty*)
let def_with_calling_occs_to_outgoing_call_option
    ((def, occ_list) :
      Relative_path.t SymbolDefinition.t
      * Relative_path.t SymbolOccurrence.t list) :
    Lsp.CallHierarchyOutgoingCalls.callHierarchyOutgoingCall =
  let open Lsp.CallHierarchyOutgoingCalls in
  match occ_list with
  | [] -> raise (Invalid_argument "Empty occ list")
  | h :: _ ->
    let call_to_ = Lsp_helpers.symbol_to_lsp_call_item h (Some def) in
    let fromRanges_ =
      List.map
        (fun occ ->
          Lsp_helpers.hack_pos_to_lsp_range_adjusted occ.SymbolOccurrence.pos)
        occ_list
    in
    { call_to = call_to_; fromRanges = fromRanges_ }

let group_occs_by_def
    (get_def :
      Relative_path.t SymbolOccurrence.t ->
      Relative_path.t SymbolDefinition.t option)
    (sym_occs : Relative_path.t SymbolOccurrence.t list) :
    (Relative_path.t SymbolDefinition.t
    * Relative_path.t SymbolOccurrence.t list)
    list =
  let table = ref (3 * List.length sym_occs / 4 |> Hashtbl.create) in
  let (key_list : Relative_path.t SymbolDefinition.t list ref) = ref [] in
  let add_sym_occ_to_table (sym_occ : Relative_path.t SymbolOccurrence.t) : unit
      =
    match get_def sym_occ with
    | None -> ()
    | Some def ->
      if not (Hashtbl.mem !table def) then key_list := def :: !key_list;
      Hashtbl.add !table def sym_occ
  in
  List.iter add_sym_occ_to_table sym_occs;
  List.map (fun key -> (key, Hashtbl.find_all !table key)) !key_list

let go (item : Lsp.CallHierarchyItem.t) ~(ctx : Provider_context.t) =
  let file = Lsp_helpers.lsp_uri_to_path item.Lsp.CallHierarchyItem.uri in
  let (ctx, entry, _, get_def) = ServerDepsUtil.get_def_setup ctx file in
  let dependable_symbols =
    IdentifySymbolService.all_symbols_ctx ~ctx ~entry
    |> List.filter ServerDepsUtil.symbol_in_call_hierarchy
  in
  let target_symbols =
    ServerCallHierarchyUtils.call_item_to_symbol_occ_list ~ctx ~entry ~item
  in
  let target_defs = List.map get_def target_symbols in
  let targets_zipped = List.combine target_symbols target_defs in
  let targets_zipped_filtered =
    List.filter (fun (_, d) -> Option.is_some d) targets_zipped
  in
  let (target_symbols_filtered, target_defs_filtered) =
    List.split targets_zipped_filtered
  in
  let target_defs_filtered_nopt = List.map Option.get target_defs_filtered in
  let body_symbols_lists =
    List.map2
      (ServerDepsUtil.body_symbols ~ctx ~entry dependable_symbols)
      target_symbols_filtered
      target_defs_filtered_nopt
  in
  let grouped_body_symbols_list =
    List.concat_map (group_occs_by_def get_def) body_symbols_lists
  in
  try
    let call_out_list =
      List.map
        def_with_calling_occs_to_outgoing_call_option
        grouped_body_symbols_list
    in
    Some call_out_list
  with
  | Invalid_argument _ -> None
