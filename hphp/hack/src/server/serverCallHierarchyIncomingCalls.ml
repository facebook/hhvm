(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let group_refs_by_file (ref_result : ServerCommandTypes.Find_refs.result) :
    (string * ServerCommandTypes.Find_refs.result) list =
  let table = ref (3 * List.length ref_result / 4 |> Hashtbl.create) in
  let key_list : string list ref = ref [] in
  let add_ref_to_tbl ((name, pos) : string * Pos.absolute) : unit =
    let file_ = Pos.filename pos in
    if not (Hashtbl.mem !table file_) then key_list := file_ :: !key_list;
    Hashtbl.add !table file_ (name, pos)
  in
  List.iter add_ref_to_tbl ref_result;
  List.map (fun key -> (key, Hashtbl.find_all !table key)) !key_list

let occ_defs_of_file (ctx : Provider_context.t) (file : string) :
    (Relative_path.t SymbolOccurrence.t
    * Relative_path.t SymbolDefinition.t option)
    list =
  let path = Relative_path.create_detect_prefix file in
  let (ctx_out, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  let all_symbols = IdentifySymbolService.all_symbols_ctx ~ctx:ctx_out ~entry in
  let all_defs =
    List.filter (fun s -> s.SymbolOccurrence.is_declaration) all_symbols
  in
  let (_, get_def) = ServerDepsUtil.get_ast_getdef ctx_out entry in
  List.map (fun s -> (s, get_def s)) all_defs

let call_sites
    (refs : ServerCommandTypes.Find_refs.result)
    ((_, d) :
      Relative_path.t SymbolOccurrence.t
      * Relative_path.t SymbolDefinition.t option) :
    ServerCommandTypes.Find_refs.result =
  match d with
  | None -> []
  | Some ds ->
    List.filter
      (fun (_, p) -> Pos.contains ds.SymbolDefinition.span (Pos.to_relative p))
      refs

let def_call_sites_to_incoming_call
    ((o, d) :
      Relative_path.t SymbolOccurrence.t
      * Relative_path.t SymbolDefinition.t option)
    (refs : ServerCommandTypes.Find_refs.result) :
    Lsp.CallHierarchyIncomingCalls.callHierarchyIncomingCall =
  let open Lsp.CallHierarchyIncomingCalls in
  let from_ = Lsp_helpers.symbol_to_lsp_call_item o d in
  let fromRanges_ =
    List.map (fun (_, p) -> Lsp_helpers.hack_pos_to_lsp_range_adjusted p) refs
  in
  { from = from_; fromRanges = fromRanges_ }

let file_refs_to_incoming_calls
    (ctx : Provider_context.t)
    ((file, refs) : string * ServerCommandTypes.Find_refs.result) :
    Lsp.CallHierarchyIncomingCalls.callHierarchyIncomingCall list =
  let occ_defs = occ_defs_of_file ctx file in
  let present_occ_defs =
    List.filter (fun (_, d) -> Option.is_some d) occ_defs
  in
  let def_call_sites = List.map (call_sites refs) present_occ_defs in
  let def_call_sites_zipped = List.combine present_occ_defs def_call_sites in
  let def_call_sites_zipped_filtered =
    List.filter (fun (_, r) -> not (Core.List.is_empty r)) def_call_sites_zipped
  in
  let (present_occ_defs_filtered, def_call_sites_filtered) =
    List.split def_call_sites_zipped_filtered
  in
  List.map2
    def_call_sites_to_incoming_call
    present_occ_defs_filtered
    def_call_sites_filtered

let string_pos_to_enclosing_rel_occs
    ~(ctx : Provider_context.t) (pos : string Pos.pos) :
    Relative_path.t SymbolOccurrence.t list =
  let file = Pos.filename pos in
  let path = Relative_path.create_detect_prefix file in
  let (ctx_out, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  let all_symbols = IdentifySymbolService.all_symbols_ctx ~ctx:ctx_out ~entry in
  let all_defs =
    List.filter (fun s -> s.SymbolOccurrence.is_declaration) all_symbols
  in
  let pos_rel = Pos.to_relative pos in
  List.filter (fun s -> Pos.contains s.SymbolOccurrence.pos pos_rel) all_defs

let ref_result_to_incoming_call_result
    (ctx : Provider_context.t)
    (ref_result : ServerCommandTypes.Find_refs.result) :
    Lsp.CallHierarchyIncomingCalls.callHierarchyIncomingCall list =
  let grouped_by_file = group_refs_by_file ref_result in
  let incoming_calls =
    List.concat_map (file_refs_to_incoming_calls ctx) grouped_by_file
  in
  incoming_calls

let go
    (item : Lsp.CallHierarchyItem.t)
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env) :
    Lsp.CallHierarchyIncomingCalls.callHierarchyIncomingCall list
    ServerCommandTypes.Done_or_retry.t
    list =
  let file = Lsp_helpers.lsp_uri_to_path item.Lsp.CallHierarchyItem.uri in
  let (ctx, entry, _, get_def) = ServerDepsUtil.get_def_setup ctx file in
  let declarations =
    IdentifySymbolService.all_symbols_ctx ~ctx ~entry
    |> List.filter (fun s -> s.SymbolOccurrence.is_declaration)
  in
  let target_symbols =
    ServerCallHierarchyUtils.call_item_to_symbol_occ_list ~ctx ~entry ~item
  in
  let get_body_references =
    ServerDepsInBatch.body_references
      ~ctx
      ~entry
      ~genv
      ~env
      ~get_def
      ~declarations
  in
  let ref_result_or_retries =
    List.concat_map get_body_references target_symbols
  in
  List.map
    (fun s ->
      ServerCommandTypes.Done_or_retry.map_env
        ~f:(ref_result_to_incoming_call_result ctx)
        (env, s)
      |> snd)
    ref_result_or_retries
