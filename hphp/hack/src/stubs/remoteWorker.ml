(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type RemoteServerApi = sig
  type naming_table

  (* Called by the worker to load the naming tabe base when first initializing *)
  val load_naming_table_base :
    naming_table_base:Path.t option -> (naming_table, string) result

  (* Called by the worker to load the naming table before type checking *)
  val load_naming_table_changes_since_baseline :
    Provider_context.t ->
    naming_table:naming_table ->
    naming_table_diff:Naming_table.changes_since_baseline ->
    (naming_table, string) result

  val download_naming_and_dep_table :
    manifold_api_key:string option ->
    use_manifold_cython_client:bool ->
    nonce:Int64.t ->
    string option ->
    naming_table * string option

  (* Updates the naming table with changed files. *)
  val update_naming_table :
    naming_table -> Relative_path.t list option -> (unit, string) result

  val fetch_and_cache_remote_decls :
    ctx:Provider_context.t ->
    naming_table ->
    from_saved_state:bool ->
    string option ->
    string ->
    bool ->
    unit

  (* Called by the worker to type check a list of files.
     The state filename is where the type checker should save its state that
     changed as a result of type checking the files
     (i.e., the dependency graph) *)
  val type_check :
    Provider_context.t ->
    init_id:string ->
    check_id:string ->
    Relative_path.t list ->
    state_filename:string ->
    telemetry:Telemetry.t ->
    Errors.t
end

type 'naming_table work_env = {
  bin_root: Path.t;
  check_id: string;
  ci_info: Ci_util.info option Future.t option;
  ctx: Provider_context.t;
  init_id: string;
  init_start_t: float;
  key: string;
  nonce: Int64.t;
  root: Path.t;
  naming_table_base: 'naming_table;
  timeout: int;
  cache_remote_decls: bool;
  use_shallow_decls_saved_state: bool;
  saved_state_manifold_path: string option;
  shallow_decls_manifold_path: string option;
  server: (module RemoteServerApi with type naming_table = 'naming_table);
}

let make_env
    (ctx : Provider_context.t)
    ~(bin_root : Path.t)
    ~(check_id : string)
    ~(ci_info : Ci_util.info option Future.t option)
    ~(init_id : string)
    ~(init_start_t : float)
    ~(key : string)
    ~(nonce : Int64.t)
    ~(root : Path.t)
    ~(cache_remote_decls : bool)
    ~(use_shallow_decls_saved_state : bool)
    ~(saved_state_manifold_path : string option)
    ~(shallow_decls_manifold_path : string option)
    ?(timeout = (600 : int))
    (server : (module RemoteServerApi with type naming_table = 'naming_table)) :
    'naming_table work_env =
  {
    bin_root;
    check_id;
    ci_info;
    ctx;
    init_id;
    init_start_t;
    key;
    nonce;
    naming_table_base = None;
    root;
    cache_remote_decls;
    use_shallow_decls_saved_state;
    saved_state_manifold_path;
    shallow_decls_manifold_path;
    timeout;
    server;
  }

let go _ = failwith "not implemented"
