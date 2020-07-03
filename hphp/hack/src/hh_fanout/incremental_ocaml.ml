(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hh_bucket = Bucket
open Core_kernel

type typecheck_result = {
  fanout_files_deps: Incremental.dep_graph_delta;
      (** The delta to the dependency graph saved-state. This field is not
      cumulative, so it must be merged with any previous `fanout_files_deps`.
      *)
  errors: Errors.t;
      (** The errors in the codebase at this point in time. This field is
      cumulative, so previous cursors need not be consulted. TODO: is that
      true, or should this be a `Relative_path.Map.t Errors.t`? *)
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
      fanout_result: Calculate_fanout.result;
          (** The result of calcluating
          the fanout for the changed files at the given point in time. *)
    }
  | Typecheck_result of {
      previous: cursor_state;  (** The cursor before this one. *)
      typecheck_result: typecheck_result;
          (** The result of typechecking the fanout. *)
    }

type persistent_state = {
  max_cursor_id: int ref;
  cursors:
    ( Incremental.cursor_id,
      Incremental.client_id * Incremental.cursor )
    Hashtbl.t;
  clients: (Incremental.client_id, Incremental.client_config) Hashtbl.t;
}

let typecheck_and_get_deps_and_errors_job
    (ctx : Provider_context.t) _acc (paths : Relative_path.t list) :
    Errors.t * Incremental.dep_graph_delta =
  List.fold
    paths
    ~init:(Errors.empty, HashSet.create ())
    ~f:(fun acc path ->
      let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
      match Provider_context.read_file_contents entry with
      | Some _ ->
        let deps = HashSet.create () in
        Typing_deps.add_dependency_callback
          "typecheck_and_get_deps_and_errors_job"
          (fun dependent dependency ->
            let dependent = Typing_deps.Dep.make dependent in
            let dependency = Typing_deps.Dep.make dependency in
            HashSet.add deps (dependent, dependency));
        let { Tast_provider.Compute_tast_and_errors.errors; _ } =
          Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry
        in

        let (acc_errors, acc_deps) = acc in
        let acc_errors = Errors.merge errors acc_errors in
        HashSet.union acc_deps ~other:deps;
        (acc_errors, acc_deps)
      | None -> acc)

let get_state_file_path (state_dir : Path.t) : Path.t =
  Path.concat state_dir "ocaml.state"

class cursor ~client_id ~cursor_state : Incremental.cursor =
  object (self)
    val client_id : Incremental.client_id = client_id

    val cursor_state : cursor_state = cursor_state

    method get_file_deltas : Naming_sqlite.file_deltas =
      match cursor_state with
      | Saved_state _
      | Typecheck_result _ ->
        Relative_path.Map.empty
      | Saved_state_delta { changed_files; _ } -> changed_files

    method get_calculate_fanout_result : Calculate_fanout.result option =
      match cursor_state with
      | Saved_state _
      | Typecheck_result _ ->
        None
      | Saved_state_delta { fanout_result; _ } -> Some fanout_result

    method private load_naming_table (ctx : Provider_context.t) : Naming_table.t
        =
      let rec get_naming_table_path (state : cursor_state) :
          Naming_sqlite.db_path =
        match state with
        | Saved_state { naming_table_saved_state_path; _ } ->
          naming_table_saved_state_path
        | Saved_state_delta { previous; _ }
        | Typecheck_result { previous; _ } ->
          get_naming_table_path previous
      in
      let (Naming_sqlite.Db_path naming_table_path) =
        get_naming_table_path cursor_state
      in
      let changed_file_infos =
        self#get_file_deltas
        |> Relative_path.Map.fold ~init:[] ~f:(fun path delta acc ->
               let file_info =
                 match delta with
                 | Naming_sqlite.Modified file_info -> Some file_info
                 | Naming_sqlite.Deleted -> None
               in
               (path, file_info) :: acc)
      in
      Naming_table.load_from_sqlite_with_changed_file_infos
        ctx
        changed_file_infos
        naming_table_path

    method get_dep_graph_delta : Incremental.dep_graph_delta =
      let rec helper cursor_state acc =
        match cursor_state with
        | Saved_state _ -> acc
        | Typecheck_result
            { previous; typecheck_result = { fanout_files_deps; _ } } ->
          HashSet.union acc ~other:fanout_files_deps;
          helper previous acc
        | Saved_state_delta { previous; _ } -> helper previous acc
      in
      helper cursor_state (HashSet.create ())

    method get_client_id : Incremental.client_id = client_id

    method private find_cursors_since_last_typecheck : cursor_state list =
      let rec helper cursor_state =
        match cursor_state with
        | Saved_state _
        | Typecheck_result _ ->
          [cursor_state]
        | Saved_state_delta { previous; _ } -> cursor_state :: helper previous
      in
      helper cursor_state

    method private get_files_to_typecheck : Relative_path.Set.t =
      let cursors = self#find_cursors_since_last_typecheck in
      List.fold cursors ~init:Relative_path.Set.empty ~f:(fun acc cursor ->
          match cursor with
          | Saved_state { dep_table_errors_saved_state_path; _ } ->
            let errors : SaveStateServiceTypes.saved_state_errors =
              if
                Sys.file_exists
                  (Path.to_string dep_table_errors_saved_state_path)
              then
                In_channel.with_file
                  ~binary:true
                  (Path.to_string dep_table_errors_saved_state_path)
                  ~f:(fun ic -> Marshal.from_channel ic)
              else
                []
            in
            errors
            |> List.map ~f:(fun (_phase, path) -> path)
            |> List.fold ~init:acc ~f:Relative_path.Set.union
          | Saved_state_delta { changed_files; _ } ->
            changed_files
            |> Relative_path.Map.keys
            |> List.fold ~init:acc ~f:Relative_path.Set.add
          | Typecheck_result _ ->
            (* Don't need to typecheck any previous cursors. The fanout of
            the files that have changed before this typecheck have already
            been processed. Stop recursion here. *)
            acc)

    method advance
        ~(detail_level : Calculate_fanout.Detail_level.t)
        (ctx : Provider_context.t)
        (_workers : MultiWorker.worker list)
        (changed_paths : Relative_path.Set.t) : cursor =
      let changed_paths =
        Relative_path.Set.union changed_paths self#get_files_to_typecheck
      in
      let changed_files =
        Relative_path.Set.fold
          changed_paths
          ~init:
            (match cursor_state with
            | Saved_state _
            | Typecheck_result _ ->
              Relative_path.Map.empty
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

      let old_naming_table = self#load_naming_table ctx in
      let new_naming_table =
        Naming_table.update_from_deltas old_naming_table changed_files
      in
      let fanout_result =
        Calculate_fanout.go
          ~detail_level
          ~old_naming_table
          ~new_naming_table
          ~file_deltas:changed_files
          ~input_files:changed_paths
      in
      let cursor_state =
        Saved_state_delta
          { previous = cursor_state; changed_files; fanout_result }
      in
      new cursor ~client_id ~cursor_state

    method calculate_errors
        (ctx : Provider_context.t) (workers : MultiWorker.worker list)
        : Errors.t * cursor option =
      match cursor_state with
      | Typecheck_result { typecheck_result = { errors; _ }; _ } ->
        (errors, None)
      | (Saved_state _ | Saved_state_delta _) as current_cursor ->
        (* The global reverse naming table is updated by calling this
        function. We can discard the forward naming table returned to us. *)
        let (_naming_table : Naming_table.t) = self#load_naming_table ctx in

        let files_to_typecheck = self#get_files_to_typecheck in

        let (errors, fanout_files_deps) =
          MultiWorker.call
            (Some workers)
            ~job:(typecheck_and_get_deps_and_errors_job ctx)
            ~neutral:(Errors.empty, HashSet.create ())
            ~merge:(fun (errors, deps) (acc_errors, acc_deps) ->
              let acc_errors = Errors.merge acc_errors errors in
              HashSet.union acc_deps ~other:deps;
              (acc_errors, acc_deps))
            ~next:
              (Hh_bucket.make
                 (Relative_path.Set.elements files_to_typecheck)
                 ~num_workers:(List.length workers))
        in
        let typecheck_result = { fanout_files_deps; errors } in
        let cursor =
          new cursor
            ~client_id
            ~cursor_state:
              (Typecheck_result { previous = current_cursor; typecheck_result })
        in
        (errors, Some cursor)
  end

class state ~state_path ~persistent_state : Incremental.state =
  object
    val state_path : Path.t = state_path

    val persistent_state : persistent_state = persistent_state

    method save : unit =
      Out_channel.with_file
        ~binary:true
        (Path.to_string state_path)
        ~f:(fun oc -> Marshal.to_channel oc persistent_state [Marshal.Closures])

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
    In_channel.with_file ~binary:true (Path.to_string state_path) ~f:(fun ic ->
        Marshal.from_channel ic)
  in
  new state ~state_path ~persistent_state
