(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hh_bucket = Bucket
open Hh_prelude

type dependent = Typing_deps.Dep.dependent Typing_deps.Dep.variant

type dependency = Typing_deps.Dep.dependency Typing_deps.Dep.variant

let compare_dependency = Typing_deps.Dep.compare_variant

let compare_dependent = Typing_deps.Dep.compare_variant

module DepEdge = struct
  type t = {
    dependent: dependent;
    dependency: dependency;
  }
  [@@deriving ord]

  let t_of_sexp _ =
    {
      dependency = Typing_deps.Dep.Fun "_";
      dependent = Typing_deps.Dep.Fun "_";
    }

  let sexp_of_t _ = Sexp.Atom ""
end

module DepEdgeSet = Set.Make (DepEdge)

type result = {
  dependencies: dependency list;
  fanout_dependents: Typing_deps.DepSet.t;
  relevant_dep_edges: DepEdgeSet.t;
}

let result_to_json (result : result) : Hh_json.json =
  let dep_to_json dep =
    Hh_json.JSON_Object
      [
        ("variant", Hh_json.JSON_String (Typing_deps.Dep.variant_to_string dep));
        ( "hash",
          Hh_json.JSON_String
            (Typing_deps.(Dep.make dep) |> Typing_deps.Dep.to_debug_string) );
      ]
  in
  let dep_edge_to_json { DepEdge.dependent; dependency } =
    Hh_json.JSON_Object
      [
        ("dependent", dep_to_json dependent);
        ("dependency", dep_to_json dependency);
      ]
  in
  let { dependencies; fanout_dependents; relevant_dep_edges } = result in
  Hh_json.JSON_Object
    [
      ("dependencies", Hh_json.JSON_Array (List.map dependencies ~f:dep_to_json));
      ( "fanout_dependents",
        Hh_json.JSON_Array
          (fanout_dependents
          |> Typing_deps.DepSet.elements
          |> List.map ~f:Typing_deps.Dep.to_debug_string
          |> List.sort ~compare:String.compare
          |> List.map ~f:(fun dep -> Hh_json.JSON_String dep)) );
      ( "relevant_dep_edges",
        Hh_json.JSON_Array
          (Set.elements relevant_dep_edges |> List.map ~f:dep_edge_to_json) );
    ]

let calculate_dep_edges
    ~(ctx : Provider_context.t) _acc (paths : Relative_path.t list) :
    DepEdge.t HashSet.t list =
  List.map paths ~f:(fun path ->
      let dep_edges = HashSet.create () in
      Typing_deps.add_dependency_callback
        ~name:"hh_fanout debug collect deps"
        (fun dependent dependency ->
          HashSet.add dep_edges { DepEdge.dependent; dependency });

      let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
      (match Provider_context.read_file_contents entry with
      | Some _ ->
        let _result : Tast_provider.Compute_tast_and_errors.t =
          Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
        in
        ()
      | None -> ());
      dep_edges)

let go
    ~(ctx : Provider_context.t)
    ~(workers : MultiWorker.worker list)
    ~(old_naming_table : Naming_table.t)
    ~(new_naming_table : Naming_table.t)
    ~(file_deltas : Naming_sqlite.file_deltas)
    ~(path : Relative_path.t) : result =
  let deps_mode = Provider_context.get_deps_mode ctx in
  let { Calculate_fanout.fanout_dependents; fanout_files; explanations; _ } =
    Calculate_fanout.go
      ~deps_mode
      ~detail_level:Calculate_fanout.Detail_level.Low
      ~old_naming_table
      ~new_naming_table
      ~file_deltas
      ~input_files:(Relative_path.Set.singleton path)
  in

  let explanation = Relative_path.Map.find explanations path in
  let { Calculate_fanout.removed_symbols; modified_symbols; added_symbols } =
    explanation
  in
  let dependencies =
    List.map
      removed_symbols
      ~f:(fun Calculate_fanout.{ symbol_edge = { symbol_dep; _ }; _ } ->
        symbol_dep)
    @ List.map
        modified_symbols
        ~f:(fun Calculate_fanout.{ symbol_edge = { symbol_dep; _ }; _ } ->
          symbol_dep)
    @ List.map
        added_symbols
        ~f:(fun Calculate_fanout.{ symbol_edge = { symbol_dep; _ }; _ } ->
          symbol_dep)
  in

  let relevant_dep_edges =
    MultiWorker.call
      (Some workers)
      ~job:(calculate_dep_edges ~ctx)
      ~neutral:(HashSet.create ())
      ~merge:(fun dependencies acc ->
        List.iter dependencies ~f:(fun dependency_set ->
            HashSet.union acc ~other:dependency_set);
        acc)
      ~next:
        (Hh_bucket.make
           (Relative_path.Set.elements fanout_files)
           ~num_workers:(List.length workers))
  in
  HashSet.filter relevant_dep_edges ~f:(fun { DepEdge.dependent; _ } ->
      let open Typing_deps in
      DepSet.mem fanout_dependents (Dep.make dependent));
  let relevant_dep_edges =
    HashSet.to_list relevant_dep_edges |> DepEdgeSet.of_list
  in

  { dependencies; fanout_dependents; relevant_dep_edges }
