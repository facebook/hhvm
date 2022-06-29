(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type dependent_member =
  | Method of string
  | SMethod of string

type dependency = Typing_deps.Dep.dependency Typing_deps.Dep.variant

type coarse_dependent = Typing_deps.Dep.dependent Typing_deps.Dep.variant

(** [Typing_deps.Dep.dependency Typing_deps.Dep.variant] contains additionally
  * constructors that we don't need here.
  * Also, the fact that we use it for dependents rather than dependencies may
  * be confusing. But this type is not part of the module's interface *)
type fine_dependent = Typing_deps.Dep.dependency Typing_deps.Dep.variant

(** For debugging: When enabled, should record exactly the same dependencies as
  * the original/coarse grained dependency tracking implemented in
  * [Typing_deps] *)
let record_coarse_only = false

type cache = (fine_dependent, dependency Hash_set.t) Hashtbl.t

module SQLitePersistence : sig
  val worker_file_name_glob : string

  val persist_cache : cache -> Typing_deps_mode.t -> unit

  val close_db : unit -> unit
end = struct
  module SU = Sqlite_utils
  module S = Sqlite3

  (** Contains the database connection once opened and a "statement cache",
    * which stores compiled versions of SQL queries for later re-use *)
  let db_stmt_cache : SU.StatementCache.t option ref = ref None

  let worker_file_name_glob = "finedeps-*.sqlite3"

  let target_folder_from_mode = function
    | Typing_deps_mode.SaveToDiskMode { new_edges_dir; _ } -> new_edges_dir
    | _ -> failwith "Can only record fine dependencies in SaveToDiskMode"

  let worker_file_path mode =
    let folder = target_folder_from_mode mode in
    let worker_id = Option.value ~default:0 !Typing_deps.worker_id in
    let file_name =
      String.substr_replace_first
        worker_file_name_glob
        ~pattern:"*"
        ~with_:(Int.to_string worker_id)
    in
    Path.concat (Path.make folder) file_name |> Path.to_string

  let exec_and_check db sql = S.exec db sql |> SU.check_rc db

  let bind_and_exec stmt_cache stmt data =
    let open SU.StatementCache in
    S.bind_values stmt data |> SU.check_rc stmt_cache.db;
    S.step stmt |> SU.check_rc stmt_cache.db

  let open_db path =
    let db = S.db_open path in
    exec_and_check db "PRAGMA synchronous = OFF;";
    exec_and_check db "PRAGMA journal_mode = MEMORY;";
    db

  let close_db () =
    Option.iter !db_stmt_cache ~f:(fun stmt_cache ->
        SU.StatementCache.close stmt_cache;
        S.db_close stmt_cache.SU.StatementCache.db |> ignore;
        db_stmt_cache := None)

  let create_tables db =
    let create_nodes_table =
      "CREATE TABLE IF NOT EXISTS nodes(
           root TEXT NOT NULL,
           kind INTEGER NOT NULL,
           member TEXT,
           hash INTEGER NOT NULL
       );"
    in

    let create_dependencies_table =
      "CREATE TABLE IF NOT EXISTS dependencies(
           dependent_hash INTEGER NOT NULL,
           dependency_hash INTEGER NOT NULL
       );"
    in

    exec_and_check db create_nodes_table;
    exec_and_check db create_dependencies_table

  let get_stmt_cache mode =
    let path = worker_file_path mode in
    match !db_stmt_cache with
    | None ->
      let new_db = open_db path in
      create_tables new_db;

      let new_stmt_cache = SU.StatementCache.make ~db:new_db in

      db_stmt_cache := Some new_stmt_cache;
      new_stmt_cache
    | Some stmt_cache -> stmt_cache

  let write_node stmt_cache data hash =
    let (root, member_opt, kind_id) = data in
    let sql =
      "INSERT INTO nodes(root, kind, member, hash) VALUES (?, ?, ?, ?)"
    in

    let stmt = SU.StatementCache.make_stmt stmt_cache sql in
    let data =
      let open SU.Data_shorthands in
      [text root; int kind_id; opt_text member_opt; int hash]
    in
    bind_and_exec stmt_cache stmt data

  let write_dependency_edge stmt_cache dependent_hash dependency_hash =
    let sql =
      "INSERT INTO dependencies(dependent_hash, dependency_hash) VALUES (?, ?)"
    in

    let stmt = SU.StatementCache.make_stmt stmt_cache sql in
    let data =
      let open SU.Data_shorthands in
      [int dependent_hash; int dependency_hash]
    in
    bind_and_exec stmt_cache stmt data

  let persist_cache (cache : cache) mode =
    let db = get_stmt_cache mode in
    let all_nodes = Hash_set.Poly.create () in
    let data_of_dep dep =
      let open Typing_deps.Dep in
      ( extract_root_name dep,
        extract_member_name dep,
        dep_kind_of_variant dep |> dep_kind_to_enum )
    in

    let hash_of_dep dep = Typing_deps.Dep.make dep |> Typing_deps.Dep.to_int in

    let handle_dependency dependent_hash dependency =
      let dependency_hash = hash_of_dep dependency in
      write_dependency_edge db dependent_hash dependency_hash
    in
    let handle_dependent fine_dependent dependencies =
      let dependent_hash = hash_of_dep fine_dependent in
      Hash_set.add all_nodes fine_dependent;
      Hash_set.iter dependencies ~f:(fun dep ->
          Hash_set.add all_nodes dep;
          handle_dependency dependent_hash dep)
    in
    Hashtbl.iteri cache ~f:(fun ~key ~data -> handle_dependent key data);
    Hash_set.iter all_nodes ~f:(fun dep ->
        write_node db (data_of_dep dep) (hash_of_dep dep))
end

module Backend = struct
  let cache = ref None

  (* Let's (conservatively) assume each entry is 100 bytes, and we want to
   * allow 10MB of cache (per worker) *)
  let cache_max_size = 10_000_000 / 100

  let cache_used_size = ref 0

  let make_cache () = Hashtbl.Poly.create ()

  let make_set () = Hash_set.Poly.create ()

  let get_cache () =
    match !cache with
    | None ->
      let new_cache = make_cache () in
      cache := Some new_cache;
      new_cache
    | Some cache -> cache

  let flush_cache mode =
    let cache = get_cache () in
    SQLitePersistence.persist_cache cache mode;
    Hashtbl.clear cache;
    cache_used_size := 0

  let add mode fine_dependent dependency =
    let cache = get_cache () in
    let inc_and_make () =
      cache_used_size := !cache_used_size + 1;
      make_set ()
    in
    let set = Hashtbl.find_or_add cache fine_dependent ~default:inc_and_make in
    if Result.is_ok @@ Hash_set.strict_add set dependency then
      cache_used_size := !cache_used_size + 1;
    if !cache_used_size >= cache_max_size then flush_cache mode;
    ()

  let finalize mode =
    flush_cache mode;
    SQLitePersistence.close_db ()
end

let add_fine_dep mode fine_dependent dependency =
  Backend.add mode fine_dependent dependency

let add_coarse_dep mode coarse_dep =
  add_fine_dep mode (Typing_deps.Dep.dependency_of_variant coarse_dep)

let fine_dependent_of_coarse_and_member :
    coarse_dependent -> dependent_member option -> fine_dependent =
 fun coarse member ->
  let member =
    if record_coarse_only then
      None
    else
      member
  in
  match (coarse, member) with
  | (root, None) -> Typing_deps.Dep.dependency_of_variant root
  | (Typing_deps.Dep.Type t, Some (Method m)) -> Typing_deps.Dep.Method (t, m)
  | (Typing_deps.Dep.Type t, Some (SMethod m)) -> Typing_deps.Dep.SMethod (t, m)
  | (_, Some _) ->
    failwith
      "Only types/classes can have members for the purposes of dependency tracking!"

let try_add_fine_dep mode coarse member dependency =
  let member =
    if record_coarse_only then
      None
    else
      member
  in
  match (coarse, member) with
  | (None, None) -> ()
  | (None, Some _) -> failwith "Cannot have member dependent without root"
  | (Some root, member_opt) ->
    let dependent = fine_dependent_of_coarse_and_member root member_opt in
    add_fine_dep mode dependent dependency

let finalize = Backend.finalize
