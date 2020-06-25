(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hh_bucket = Bucket
open Core_kernel

type fanout_result = {
  fanout_files_deps: Typing_deps.DepSet.t;
  errors: Errors.t;
}

type cursor_state =
  | Saved_state of {
      dep_table_saved_state_path: Path.t;
      dep_table_errors_saved_state_path: Path.t;
      naming_table_saved_state_path: Naming_sqlite.db_path;
    }
  | Saved_state_delta of {
      previous: cursor_state;  (** The cursor before this one. *)
      changed_files: Naming_sqlite.file_deltas;
          (** The files that have changed since the saved-state. This field
          is cumulative, so previous cursors need not be consulted. *)
      changed_files_deps: Incremental.dep_graph_delta;
          (** The dependency edges that were produces from typechecking
          `changed_files`. This field is not cumulative, so it must be
          merged with the results of previous cursors. *)
      fanout_result: fanout_result option;
          (** The result of typechecking the fanout. It's not meaningful to
          merge this field with the results of previous cursors. *)
    }

type persistent_state = {
  max_cursor_id: int ref;
  cursors:
    ( Incremental.cursor_id,
      Incremental.client_id * Incremental.cursor )
    Hashtbl.t;
  clients: (Incremental.client_id, Incremental.client_config) Hashtbl.t;
}

let typecheck_and_get_deps_job
    (ctx : Provider_context.t) _acc (paths : Relative_path.t list) :
    Incremental.dep_graph_delta =
  let deps = HashSet.create () in
  Typing_deps.add_dependency_callback
    "typecheck_and_get_deps_job"
    (fun dependent dependency ->
      let dependent = Typing_deps.Dep.make dependent in
      let dependency = Typing_deps.Dep.make dependency in
      HashSet.add deps (dependent, dependency));
  List.iter paths ~f:(fun path ->
      let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
      match Provider_context.read_file_contents entry with
      | Some _ ->
        let (_ : Tast_provider.Compute_tast_and_errors.t) =
          Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
        in
        ()
      | None -> ());
  deps

let get_state_file_path (state_dir : Path.t) : Path.t =
  Path.concat state_dir "ocaml.state"

class cursor ~client_id ~cursor_state : Incremental.cursor =
  object (self)
    val client_id : Incremental.client_id = client_id

    val cursor_state : cursor_state = cursor_state

    method get_file_deltas : Naming_sqlite.file_deltas =
      match cursor_state with
      | Saved_state _ -> Relative_path.Map.empty
      | Saved_state_delta { changed_files; _ } -> changed_files

    method get_dep_graph_delta : Incremental.dep_graph_delta =
      let rec helper cursor_state acc =
        match cursor_state with
        | Saved_state _ -> acc
        | Saved_state_delta { previous; changed_files_deps; _ } ->
          HashSet.union acc ~other:changed_files_deps;
          helper previous acc
      in
      helper cursor_state (HashSet.create ())

    method get_client_id : Incremental.client_id = client_id

    method private find_cursors_up_to_and_including_last_complete
        : cursor_state list =
      let rec helper cursor_state =
        match cursor_state with
        | Saved_state _ -> [cursor_state]
        | Saved_state_delta { fanout_result = Some _; _ } -> [cursor_state]
        | Saved_state_delta { fanout_result = None; previous; _ } ->
          cursor_state :: helper previous
      in
      helper cursor_state

    method private get_files_to_typecheck : Relative_path.Set.t =
      let cursors = self#find_cursors_up_to_and_including_last_complete in
      List.fold cursors ~init:Relative_path.Set.empty ~f:(fun acc cursor ->
          match cursor with
          | Saved_state _ -> acc
          | Saved_state_delta { changed_files; _ } ->
            changed_files
            |> Relative_path.Map.keys
            |> List.fold ~init:acc ~f:Relative_path.Set.add)

    method advance
        (ctx : Provider_context.t)
        (workers : MultiWorker.worker list)
        (changed_paths : Relative_path.Set.t) : cursor =
      let changed_paths =
        Relative_path.Set.union changed_paths self#get_files_to_typecheck
      in
      let changed_files =
        Relative_path.Set.fold
          changed_paths
          ~init:
            (match cursor_state with
            | Saved_state _ -> Relative_path.Map.empty
            | Saved_state_delta { changed_files; _ } -> changed_files)
          ~f:(fun path acc ->
            let (ctx, entry) =
              Provider_context.add_entry_if_missing ~ctx ~path
            in
            match Provider_context.read_file_contents entry with
            | None ->
              Relative_path.Map.add acc ~key:path ~data:Naming_sqlite.Deleted
            | Some _ ->
              let file_info =
                Ast_provider.compute_file_info
                  ~popt:(Provider_context.get_popt ctx)
                  ~entry
              in
              Relative_path.Map.add
                acc
                ~key:path
                ~data:(Naming_sqlite.Modified file_info))
      in

      let changed_files_deps =
        MultiWorker.call
          (Some workers)
          ~job:(typecheck_and_get_deps_job ctx)
          ~neutral:(HashSet.create ())
          ~merge:(fun lhs rhs ->
            HashSet.union lhs ~other:rhs;
            lhs)
          ~next:
            (Hh_bucket.make
               (Relative_path.Set.elements changed_paths)
               ~num_workers:(List.length workers))
      in

      let cursor_state =
        Saved_state_delta
          {
            previous = cursor_state;
            changed_files;
            changed_files_deps;
            fanout_result = None;
          }
      in
      new cursor ~client_id ~cursor_state
  end

class state ~state_path ~persistent_state : Incremental.state =
  object
    val state_path : Path.t = state_path

    val persistent_state : persistent_state = persistent_state

    method save : unit =
      Out_channel.with_file (Path.to_string state_path) ~f:(fun oc ->
          Marshal.to_channel oc persistent_state [Marshal.Closures])

    method look_up_client_id (client_config : Incremental.client_config)
        : Incremental.client_id =
      let client_id = Incremental_utils.make_client_id client_config in
      Hashtbl.set persistent_state.clients client_id client_config;
      client_id

    method make_default_cursor (client_id : Incremental.client_id)
        : (cursor, string) result =
      match Hashtbl.find persistent_state.clients client_id with
      | Some client_config ->
        Ok
          (new cursor
             ~client_id
             ~cursor_state:
               (Saved_state
                  {
                    dep_table_saved_state_path =
                      client_config.Incremental.dep_table_saved_state_path;
                    dep_table_errors_saved_state_path =
                      client_config.Incremental.errors_saved_state_path;
                    naming_table_saved_state_path =
                      client_config.Incremental.naming_table_saved_state_path;
                  }))
      | None ->
        let (Incremental.Client_id client_id) = client_id in
        Error (Printf.sprintf "Client ID %s could not be found" client_id)

    method look_up_cursor
        ~(client_id : Incremental.client_id option) ~(cursor_id : string)
        : (cursor, string) result =
      let cursor_opt =
        Hashtbl.find persistent_state.cursors (Incremental.Cursor_id cursor_id)
      in
      match (client_id, cursor_opt) with
      | (None, Some (Incremental.Client_id _existing_client_id, cursor)) ->
        Ok cursor
      | ( Some (Incremental.Client_id client_id),
          Some (Incremental.Client_id existing_client_id, cursor) )
        when String.equal client_id existing_client_id ->
        Ok cursor
      | ( Some (Incremental.Client_id client_id),
          Some (Incremental.Client_id existing_client_id, _cursor) ) ->
        Error
          (Printf.sprintf
             "Client ID %s was provided, but cursor %s is associated with client ID %s"
             client_id
             cursor_id
             existing_client_id)
      | (Some (Incremental.Client_id client_id), None) ->
        Error
          (Printf.sprintf
             "Cursor with ID %s not found (for client ID %s)"
             cursor_id
             client_id)
      | (None, None) ->
        Error (Printf.sprintf "Cursor with ID %s not found)" cursor_id)

    method add_cursor (cursor : cursor) : Incremental.cursor_id =
      let client_id = cursor#get_client_id in
      let client_config = Hashtbl.find_exn persistent_state.clients client_id in
      let cursor_id =
        Incremental_utils.make_cursor_id
          !(persistent_state.max_cursor_id)
          client_config
      in
      incr persistent_state.max_cursor_id;
      Hashtbl.set persistent_state.cursors cursor_id (client_id, cursor);
      cursor_id
  end

let make (state_dir : Path.t) : Incremental.state =
  Incremental_utils.init_state_dir state_dir ~populate_dir:(fun temp_dir ->
      let temp_state_path = get_state_file_path temp_dir in
      if not (Path.file_exists temp_state_path) then
        let state =
          new state
            ~state_path:temp_state_path
            ~persistent_state:
              {
                max_cursor_id = ref 0;
                cursors = Hashtbl.Poly.create ();
                clients = Hashtbl.Poly.create ();
              }
        in
        state#save);
  let state_path = get_state_file_path state_dir in
  let (persistent_state : persistent_state) =
    In_channel.with_file (Path.to_string state_path) ~f:(fun ic ->
        Marshal.from_channel ic)
  in
  new state ~state_path ~persistent_state
