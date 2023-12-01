(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Detail_level = struct
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
  modified_symbols: changed_symbol list;
  added_symbols: changed_symbol list;
}

type result = {
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
      ( "modified_symbols",
        Hh_json.JSON_Array
          (List.map ~f:changed_symbol_to_json explanation.modified_symbols) );
      ( "removed_symbols",
        Hh_json.JSON_Array
          (List.map ~f:changed_symbol_to_json explanation.removed_symbols) );
    ]

let get_symbol_edges_for_file_info (ids : FileInfo.ids) : symbol_edge list =
  let make_edges ~symbol_type ~ids ~f =
    List.map ids ~f:(fun (_, symbol_name, _) ->
        { symbol_type; symbol_name; symbol_dep = f symbol_name })
  in
  let { FileInfo.classes; funs; consts; typedefs; modules } = ids in
  List.concat
    [
      make_edges ~symbol_type:FileInfo.Class ~ids:classes ~f:(fun name ->
          Typing_deps.Dep.Type name);
      make_edges ~symbol_type:FileInfo.Fun ~ids:funs ~f:(fun name ->
          Typing_deps.Dep.Fun name);
      make_edges ~symbol_type:FileInfo.Const ~ids:consts ~f:(fun name ->
          Typing_deps.Dep.GConst name);
      make_edges ~symbol_type:FileInfo.Typedef ~ids:typedefs ~f:(fun name ->
          Typing_deps.Dep.Type name);
      make_edges ~symbol_type:FileInfo.Module ~ids:modules ~f:(fun name ->
          Typing_deps.Dep.Module name);
    ]

let file_info_to_dep_set
    ~(detail_level : Detail_level.t)
    ~(deps_mode : Typing_deps_mode.t)
    (naming_table : Naming_table.t)
    (ids : FileInfo.ids) : Typing_deps.DepSet.t * changed_symbol list =
  List.fold
    (get_symbol_edges_for_file_info ids)
    ~init:(Typing_deps.(DepSet.make ()), [])
    ~f:(fun (dep_set, changed_symbols) symbol_edge ->
      let symbol_dep = Typing_deps.(Dep.make symbol_edge.symbol_dep) in
      let dep_set = Typing_deps.DepSet.add dep_set symbol_dep in
      let changed_symbol =
        match detail_level with
        | Detail_level.Low ->
          { symbol_edge; num_outgoing_edges = None; outgoing_files = None }
        | Detail_level.High ->
          let outgoing_edges =
            symbol_dep
            |> Typing_deps.DepSet.singleton
            |> Typing_deps.add_all_deps deps_mode
          in
          {
            symbol_edge;
            num_outgoing_edges =
              Some (Typing_deps.DepSet.cardinal outgoing_edges);
            outgoing_files =
              Some
                (Naming_table.get_64bit_dep_set_files
                   naming_table
                   outgoing_edges);
          }
      in
      let changed_symbols = changed_symbol :: changed_symbols in
      (dep_set, changed_symbols))

(** Given a file and the symbols that used to be contained in that file, find
the symbols that are currently in the file and construct a set of
dependencies that can be traversed to find the fanout of the changes to those
symbols. *)
let calculate_dep_set_for_path
    ~(detail_level : Detail_level.t)
    ~(deps_mode : Typing_deps_mode.t)
    ~(old_naming_table : Naming_table.t)
    ~(new_naming_table : Naming_table.t)
    ~(path : Relative_path.t)
    ~(delta : 'a Naming_sqlite.forward_naming_table_delta) :
    Typing_deps.DepSet.t * explanation =
  let (old_deps, old_symbols) =
    Naming_table.get_file_info old_naming_table path
    |> Option.map ~f:(fun fi ->
           file_info_to_dep_set
             ~detail_level
             ~deps_mode
             old_naming_table
             fi.FileInfo.ids)
    |> Option.value ~default:(Typing_deps.(DepSet.make ()), [])
  in
  let (new_deps, new_symbols) =
    match delta with
    | Naming_sqlite.Modified new_file_info ->
      file_info_to_dep_set
        ~detail_level
        ~deps_mode
        new_naming_table
        new_file_info.FileInfo.ids
    | Naming_sqlite.Deleted -> (Typing_deps.(DepSet.make ()), [])
  in

  (* NB: could be optimized by constructing sets or by not using polymorphic
     equality. *)
  let (modified_symbols, removed_symbols) =
    List.partition_tf old_symbols ~f:(fun old_symbol ->
        List.exists new_symbols ~f:(fun new_symbol ->
            Poly.(old_symbol.symbol_edge = new_symbol.symbol_edge)))
  in
  let added_symbols =
    List.filter new_symbols ~f:(fun new_symbol ->
        not
          (List.exists old_symbols ~f:(fun old_symbol ->
               Poly.(old_symbol.symbol_edge = new_symbol.symbol_edge))))
  in

  let explanation = { removed_symbols; modified_symbols; added_symbols } in
  (Typing_deps.DepSet.union old_deps new_deps, explanation)

let go
    ~(detail_level : Detail_level.t)
    ~(deps_mode : Typing_deps_mode.t)
    ~(old_naming_table : Naming_table.t)
    ~(new_naming_table : Naming_table.t)
    ~(file_deltas : Naming_sqlite.file_deltas)
    ~(input_files : Relative_path.Set.t) : result =
  let calculate_dep_set_telemetry = Telemetry.create () in
  let start_time = Unix.gettimeofday () in
  let (fanout_dependencies, explanations) =
    Relative_path.Set.fold
      input_files
      ~init:(Typing_deps.(DepSet.make ()), Relative_path.Map.empty)
      ~f:(fun path (fanout_dependencies, explanations) ->
        let delta =
          match Relative_path.Map.find_opt file_deltas path with
          | Some delta -> delta
          | None ->
            failwith
              ("Input path %s was not in the map of `file_deltas`. "
              ^ "This is an internal invariant failure -- please report it. "
              ^ "This means that we can't process it, "
              ^ "as we haven't calculated its `FileInfo.t`. "
              ^ "The caller should have included any elements in `input_files` "
              ^ "when performing the calculation of `file_deltas`.")
        in
        let (file_deps, explanation) =
          calculate_dep_set_for_path
            ~detail_level
            ~deps_mode
            ~old_naming_table
            ~new_naming_table
            ~path
            ~delta
        in
        let fanout_dependencies =
          Typing_deps.DepSet.union fanout_dependencies file_deps
        in
        ( fanout_dependencies,
          Relative_path.Map.add explanations ~key:path ~data:explanation ))
  in

  (* We have the dependencies -- now traverse the dependency graph to get
     their dependents. *)
  let fanout_dependents =
    Typing_deps.add_all_deps deps_mode fanout_dependencies
  in

  let calculate_dep_set_telemetry =
    Telemetry.duration ~start_time calculate_dep_set_telemetry
  in

  let calculate_fanout_telemetry = Telemetry.create () in
  let start_time = Unix.gettimeofday () in
  let fanout_files =
    Naming_table.get_64bit_dep_set_files new_naming_table fanout_dependents
  in

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

  { fanout_dependents; fanout_files; explanations; telemetry }
