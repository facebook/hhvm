(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hh_bucket = Bucket
open Hh_prelude

type client_id = Client_id of string

type cursor_id = Cursor_id of string

type dep_graph_delta = (Typing_deps.Dep.t * Typing_deps.Dep.t) HashSet.t

type client_config = {
  client_id: string;
  ignore_hh_version: bool;
  dep_table_saved_state_path: Path.t;
  dep_table_errors_saved_state_path: Path.t;
  naming_table_saved_state_path: Naming_sqlite.db_path;
  deps_mode: Typing_deps_mode.t;
}

type typecheck_result = {
  errors: Errors.t;
      (** The errors in the codebase at this point in time. This field is
      cumulative, so previous cursors need not be consulted. TODO: is that
      true, or should this be a `Relative_path.Map.t Errors.t`? *)
}

type cursor_state =
  | Saved_state of client_config
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

(** Construct the cursor ID exposed to the user.

For debugging purposes, the `from` and `client_config` fields are also
included in the cursor, even though we could store them in the state and
recover them from the ID.

For convenience during debugging, we try to ensure that cursors are
lexicographically-orderable by the time ordering. For that reason, it's
important that the first field in the cursor ID is the
monotonically-increasing ID.

The choice of `,` as a delimiter is important. Watchman uses `:`, which is
inappropriate for this goal, because the ASCII value of `,` is less than that
of all the numerals, while `:` is greater than that of all the numerals.

Using this delimiter ensures that a string like `cursor,1,foo` is less than a
string like `cursor,10,foo` by the ASCII lexicographical ordering, which is
not true for `cursor:1:foo` vs. `cursor:10:foo`.

Some reasoning about delimiter choice:

  * `-` is likely to appear in `from` strings.
  * `+` would contrast strangely with `-` in `from` strings.
  * `#` is interpreted as a comment in the shell.
  * `$` and `!` may accidentally interpolate values in the shell.
  * `&` launched background processes in Bash.
  * `(`, `)`, `'`, and `"` are usually paired, and have special meaning in
  the shell. Also, in this OCaml comment I have to write this " to close the
  previous double-quote, or this comment is a syntax-error.
  * '/' suggests a hierarchical relationship or an actual file.
  * `%` and `*` look a little strange in my opinion.
  * `.` and  `,` are fine.
*)
let make_cursor_id (id : int) (client_config : client_config) : cursor_id =
  let cursor_id =
    Printf.sprintf
      "cursor,%d,%s,%d"
      id
      client_config.client_id
      (Hashtbl.hash client_config)
  in
  Cursor_id cursor_id

let typecheck_and_get_deps_and_errors_job
    (ctx : Provider_context.t) _acc (paths : Relative_path.t list) :
    Errors.t * dep_graph_delta =
  List.fold
    paths
    ~init:(Errors.empty, HashSet.create ())
    ~f:(fun acc path ->
      let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
      match Provider_context.read_file_contents entry with
      | Some _ ->
        let deps = HashSet.create () in
        Typing_deps.add_dependency_callback
          ~name:"typecheck_and_get_deps_and_errors_job"
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

class cursor ~client_id ~cursor_state =
  object (self)
    val client_id : client_id = client_id

    val cursor_state : cursor_state = cursor_state

    method get_file_deltas : Naming_sqlite.file_deltas =
      let rec helper cursor_state =
        match cursor_state with
        | Saved_state _ -> Relative_path.Map.empty
        | Typecheck_result { previous; _ } -> helper previous
        | Saved_state_delta { changed_files; _ } -> changed_files
      in
      helper cursor_state

    method get_calculate_fanout_result : Calculate_fanout.result option =
      match cursor_state with
      | Saved_state _
      | Typecheck_result _ ->
        None
      | Saved_state_delta { fanout_result; _ } -> Some fanout_result

    method get_calculate_fanout_results_since_last_typecheck
        : Calculate_fanout.result list =
      let rec helper cursor_state =
        match cursor_state with
        | Saved_state _
        | Typecheck_result _ ->
          []
        | Saved_state_delta { fanout_result; previous; _ } ->
          fanout_result :: helper previous
      in
      helper cursor_state

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

    method get_client_id : client_id = client_id

    method get_client_config : client_config =
      let rec helper = function
        | Saved_state client_config -> client_config
        | Saved_state_delta { previous; _ }
        | Typecheck_result { previous; _ } ->
          helper previous
      in
      helper cursor_state

    method get_deps_mode : Typing_deps_mode.t = self#get_client_config.deps_mode

    method private load_dep_table : unit =
      let rec helper cursor_state =
        match cursor_state with
        | Saved_state _ -> ()
        | Saved_state_delta { previous; _ } -> helper previous
        | Typecheck_result _ -> failwith "not implemented"
      in
      helper cursor_state

    method private get_files_to_typecheck : Relative_path.Set.t =
      let rec helper cursor_state acc =
        match cursor_state with
        | Typecheck_result _ ->
          (* Don't need to typecheck any previous cursors. The fanout of
             the files that have changed before this typecheck have already
             been processed. Stop recursion here. *)
          acc
        | Saved_state { dep_table_errors_saved_state_path; _ } ->
          if Sys.file_exists (Path.to_string dep_table_errors_saved_state_path)
          then
            let errors : SaveStateServiceTypes.saved_state_errors =
              In_channel.with_file
                ~binary:true
                (Path.to_string dep_table_errors_saved_state_path)
                ~f:(fun ic -> Marshal.from_channel ic)
            in
            errors
          else
            Relative_path.Set.empty
        | Saved_state_delta { previous; fanout_result; _ } ->
          let acc =
            Relative_path.Set.union
              acc
              fanout_result.Calculate_fanout.fanout_files
          in
          helper previous acc
      in
      helper cursor_state Relative_path.Set.empty

    method advance
        ~(detail_level : Calculate_fanout.Detail_level.t)
        (ctx : Provider_context.t)
        (_workers : MultiWorker.worker list)
        (changed_paths : Relative_path.Set.t) : cursor =
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
              let ids =
                Ast_provider.compute_file_info
                  ~popt:(Provider_context.get_popt ctx)
                  ~entry
              in
              Relative_path.Map.add
                acc
                ~key:path
                ~data:
                  (Naming_sqlite.Modified { FileInfo.empty_t with FileInfo.ids }))
      in

      let old_naming_table = self#load_naming_table ctx in
      let new_naming_table =
        Naming_table.update_from_deltas old_naming_table changed_files
      in
      let () = self#load_dep_table in
      let fanout_result =
        Calculate_fanout.go
          ~detail_level
          ~deps_mode:self#get_deps_mode
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
        Hh_logger.log
          "Got %d new dependency edges as a result of typechecking %d files"
          (HashSet.length fanout_files_deps)
          (Relative_path.Set.cardinal files_to_typecheck);
        let typecheck_result = { errors } in
        let cursor =
          new cursor
            ~client_id
            ~cursor_state:
              (Typecheck_result { previous = current_cursor; typecheck_result })
        in
        (errors, Some cursor)
  end

type persistent_state = {
  max_cursor_id: int ref;
  cursors: (cursor_id, client_id * cursor) Hashtbl.t;
  clients: (client_id, client_config) Hashtbl.t;
}

let save_state ~(state_path : Path.t) ~(persistent_state : persistent_state) :
    unit =
  Out_channel.with_file ~binary:true (Path.to_string state_path) ~f:(fun oc ->
      Marshal.to_channel oc persistent_state [Marshal.Closures])

class state ~state_path ~persistent_state =
  object
    val state_path : Path.t = state_path

    val persistent_state : persistent_state = persistent_state

    method make_client_id (client_config : client_config) : client_id =
      let client_id = Client_id client_config.client_id in
      Hashtbl.set persistent_state.clients ~key:client_id ~data:client_config;
      client_id

    method make_default_cursor (client_id : client_id) : (cursor, string) result
        =
      match Hashtbl.find persistent_state.clients client_id with
      | Some client_config ->
        Ok (new cursor ~client_id ~cursor_state:(Saved_state client_config))
      | None ->
        let (Client_id client_id) = client_id in
        Error (Printf.sprintf "Client ID %s could not be found" client_id)

    method look_up_cursor ~(client_id : client_id option) ~(cursor_id : string)
        : (cursor, string) result =
      let cursor_opt =
        Hashtbl.find persistent_state.cursors (Cursor_id cursor_id)
      in
      match (client_id, cursor_opt) with
      | (None, Some (Client_id _existing_client_id, cursor)) -> Ok cursor
      | (Some (Client_id client_id), Some (Client_id existing_client_id, cursor))
        when String.equal client_id existing_client_id ->
        Ok cursor
      | ( Some (Client_id client_id),
          Some (Client_id existing_client_id, _cursor) ) ->
        Error
          (Printf.sprintf
             "Client ID %s was provided, but cursor %s is associated with client ID %s"
             client_id
             cursor_id
             existing_client_id)
      | (Some (Client_id client_id), None) ->
        Error
          (Printf.sprintf
             "Cursor with ID %s not found (for client ID %s)"
             cursor_id
             client_id)
      | (None, None) ->
        Error (Printf.sprintf "Cursor with ID %s not found)" cursor_id)

    method add_cursor (cursor : cursor) : cursor_id =
      let client_id = cursor#get_client_id in
      let client_config = Hashtbl.find_exn persistent_state.clients client_id in
      let cursor_id =
        make_cursor_id !(persistent_state.max_cursor_id) client_config
      in
      incr persistent_state.max_cursor_id;
      Hashtbl.set
        persistent_state.cursors
        ~key:cursor_id
        ~data:(client_id, cursor);
      save_state ~state_path ~persistent_state;
      cursor_id
  end

let init_state_dir (state_dir : Path.t) ~(populate_dir : Path.t -> unit) : unit
    =
  Disk.mkdir_p (state_dir |> Path.dirname |> Path.to_string);
  if not (Path.file_exists state_dir) then
    Tempfile.with_tempdir (fun temp_dir ->
        populate_dir temp_dir;
        try
          Disk.rename (Path.to_string temp_dir) (Path.to_string state_dir)
        with
        | Disk.Rename_target_already_exists _
        | Disk.Rename_target_dir_not_empty _ ->
          (* Assume that the directory was initialized by another process
             before us, so we don't need to do anything further. *)
          ())

let make_reference_implementation (state_dir : Path.t) : state =
  init_state_dir state_dir ~populate_dir:(fun temp_dir ->
      let temp_state_path = get_state_file_path temp_dir in
      if not (Path.file_exists temp_state_path) then
        save_state
          ~state_path:temp_state_path
          ~persistent_state:
            {
              max_cursor_id = ref 0;
              cursors = Hashtbl.Poly.create ();
              clients = Hashtbl.Poly.create ();
            });
  let state_path = get_state_file_path state_dir in
  let (persistent_state : persistent_state) =
    try
      In_channel.with_file
        ~binary:true
        (Path.to_string state_path)
        ~f:(fun ic -> Marshal.from_channel ic)
    with
    | e ->
      let e = Exception.wrap e in
      Hh_logger.warn
        ("HINT: An error occurred while loading hh_fanout state. "
        ^^ "If it is corrupted, "
        ^^ "try running `hh_fanout clean` to delete the state, "
        ^^ "then try your query again.");
      Exception.reraise e
  in
  let state = new state ~state_path ~persistent_state in
  (state :> state)
