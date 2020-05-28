(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module Verbosity = struct
  type t =
    | Low
    | High
end

type symbol_edge = {
  symbol_type: FileInfo.name_type;
  symbol_name: string;
  symbol_dep: Typing_deps.Dep.dependency Typing_deps.Dep.variant;
}

type changed_symbol = {
  symbol_edge: symbol_edge;
  num_outgoing_edges: int option;
  outgoing_files: Relative_path.Set.t option;
}

type explanation = {
  removed_symbols: changed_symbol list;
  added_symbols: changed_symbol list;
}

type result = {
  naming_table: Naming_table.t;
  fanout_dependents: Typing_deps.DepSet.t;
  fanout_files: Relative_path.Set.t;
  explanations: explanation Relative_path.Map.t;
  telemetry: Telemetry.t;
}

let explanation_to_json (explanation : explanation) : Hh_json.json =
  let changed_symbol_to_json
      {
        symbol_edge = { symbol_type; symbol_name; _ };
        num_outgoing_edges;
        outgoing_files;
      } =
    Hh_json.JSON_Object
      [
        ("type", Hh_json.JSON_String (FileInfo.show_name_type symbol_type));
        ("name", Hh_json.JSON_String symbol_name);
        ( "num_outgoing_edges",
          Option.value_map
            num_outgoing_edges
            ~f:Hh_json.int_
            ~default:Hh_json.JSON_Null );
        ( "outgoing_files",
          Option.value_map
            outgoing_files
            ~default:Hh_json.JSON_Null
            ~f:(fun outgoing_files ->
              Hh_json.JSON_Array
                (Relative_path.Set.fold
                   outgoing_files
                   ~init:[]
                   ~f:(fun path acc ->
                     let path = Relative_path.suffix path in
                     Hh_json.JSON_String path :: acc))) );
      ]
  in
  Hh_json.JSON_Object
    [
      ( "added_symbols",
        Hh_json.JSON_Array
          (List.map ~f:changed_symbol_to_json explanation.added_symbols) );
      ( "removed_symbols",
        Hh_json.JSON_Array
          (List.map ~f:changed_symbol_to_json explanation.removed_symbols) );
    ]

let get_symbol_edges_for_file_info (file_info : FileInfo.t) : symbol_edge list =
  let make_edges ~symbol_type ~ids ~f =
    List.map ids ~f:(fun (_, symbol_name) ->
        { symbol_type; symbol_name; symbol_dep = f symbol_name })
  in
  List.concat
    [
      make_edges
        ~symbol_type:FileInfo.Class
        ~ids:file_info.FileInfo.classes
        ~f:(fun name -> Typing_deps.Dep.Class name);
      make_edges
        ~symbol_type:FileInfo.Fun
        ~ids:file_info.FileInfo.funs
        ~f:(fun name -> Typing_deps.Dep.Fun name);
      make_edges
        ~symbol_type:FileInfo.Const
        ~ids:file_info.FileInfo.consts
        ~f:(fun name -> Typing_deps.Dep.GConst name);
      make_edges
        ~symbol_type:FileInfo.Typedef
        ~ids:file_info.FileInfo.typedefs
        ~f:(fun name -> Typing_deps.Dep.Class name);
      make_edges
        ~symbol_type:FileInfo.RecordDef
        ~ids:file_info.FileInfo.record_defs
        ~f:(fun name -> Typing_deps.Dep.Class name);
    ]

let file_info_to_dep_set ~(verbosity : Verbosity.t) (file_info : FileInfo.t) :
    Typing_deps.DepSet.t * changed_symbol list =
  List.fold
    (get_symbol_edges_for_file_info file_info)
    ~init:(Typing_deps.DepSet.empty, [])
    ~f:(fun (dep_set, changed_symbols) symbol_edge ->
      let symbol_dep = Typing_deps.Dep.make symbol_edge.symbol_dep in
      let dep_set = Typing_deps.DepSet.add dep_set symbol_dep in
      let changed_symbol =
        match verbosity with
        | Verbosity.Low ->
          { symbol_edge; num_outgoing_edges = None; outgoing_files = None }
        | Verbosity.High ->
          let outgoing_edges =
            symbol_dep
            |> Typing_deps.DepSet.singleton
            |> Typing_deps.add_all_deps
          in
          {
            symbol_edge;
            num_outgoing_edges =
              Some (Typing_deps.DepSet.cardinal outgoing_edges);
            outgoing_files = Some (Typing_deps.get_files outgoing_edges);
          }
      in
      let changed_symbols = changed_symbol :: changed_symbols in
      (dep_set, changed_symbols))

(** Given a file and the symbols that used to be contained in that file, find
the symbols that are currently in the file and construct a set of
dependencies that can be traversed to find the fanout of the changes to those
symbols. *)
let calculate_dep_set_for_path
    ~(verbosity : Verbosity.t)
    (ctx : Provider_context.t)
    (naming_table : Naming_table.t)
    (path : Relative_path.t) :
    FileInfo.t option * Typing_deps.DepSet.t * explanation =
  let (old_deps, removed_symbols) =
    Naming_table.get_file_info naming_table path
    |> Option.map ~f:(file_info_to_dep_set ~verbosity)
    |> Option.value ~default:(Typing_deps.DepSet.empty, [])
  in
  let new_file_info =
    match Sys_utils.cat_or_failed (Relative_path.to_absolute path) with
    | None -> None
    | Some contents ->
      let (ctx, entry) =
        Provider_context.add_or_overwrite_entry_contents ~ctx ~path ~contents
      in
      Some
        (Ast_provider.compute_file_info
           ~popt:(Provider_context.get_popt ctx)
           ~entry)
  in
  let (new_deps, added_symbols) =
    match new_file_info with
    | Some new_file_info -> file_info_to_dep_set ~verbosity new_file_info
    | None -> (Typing_deps.DepSet.empty, [])
  in
  let explanation = { removed_symbols; added_symbols } in
  (new_file_info, Typing_deps.DepSet.union old_deps new_deps, explanation)

let go
    ~(verbosity : Verbosity.t)
    (ctx : Provider_context.t)
    (naming_table : Naming_table.t)
    (files_to_process : Path.Set.t) : result =
  let files_to_process =
    Path.Set.fold
      files_to_process
      ~init:Relative_path.Set.empty
      ~f:(fun path acc ->
        path
        |> Path.to_string
        |> Relative_path.create_detect_prefix
        |> Relative_path.Set.add acc)
  in
  let calculate_dep_set_telemetry = Telemetry.create () in
  let start_time = Unix.gettimeofday () in
  let (naming_table, fanout_dependents, explanations) =
    Relative_path.Set.fold
      files_to_process
      ~init:(naming_table, Typing_deps.DepSet.empty, Relative_path.Map.empty)
      ~f:(fun path (naming_table, dep_set, explanations) ->
        let (new_file_info, file_deps, explanation) =
          calculate_dep_set_for_path ~verbosity ctx naming_table path
        in
        let fanout_deps = Typing_deps.add_all_deps file_deps in
        let new_naming_table =
          match new_file_info with
          | Some new_file_info ->
            Naming_table.update naming_table path new_file_info
          | None -> Naming_table.remove naming_table path
        in
        let new_deps = Typing_deps.DepSet.union dep_set fanout_deps in
        ( new_naming_table,
          new_deps,
          Relative_path.Map.add explanations path explanation ))
  in
  let calculate_dep_set_telemetry =
    Telemetry.duration ~start_time calculate_dep_set_telemetry
  in

  let calculate_fanout_telemetry = Telemetry.create () in
  let start_time = Unix.gettimeofday () in
  let fanout_files = Typing_deps.get_files fanout_dependents in
  let calculate_fanout_telemetry =
    Telemetry.duration ~start_time calculate_fanout_telemetry
  in

  let telemetry = Telemetry.create () in
  let telemetry =
    Telemetry.object_
      telemetry
      ~key:"calculate_dep_set"
      ~value:calculate_dep_set_telemetry
  in
  let telemetry =
    Telemetry.object_
      telemetry
      ~key:"calculate_fanout"
      ~value:calculate_fanout_telemetry
  in

  { naming_table; fanout_dependents; fanout_files; explanations; telemetry }
