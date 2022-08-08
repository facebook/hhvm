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

  val build_naming_table : unit -> unit

  (* A combination of `download_naming_and_dep_table` and `update_naming_table` below.
     Downloads the naming table via saved state and adds updates from changed_files.
     Returns the path to the dep table downloaded.
  *)
  val download_and_update_naming_table :
    manifold_api_key:string option ->
    use_manifold_cython_client:bool ->
    string option ->
    Relative_path.t list option ->
    string option

  (* Downloads the naming table via saved state.
     Returns the naming table along with the path to the dep table. *)
  val download_naming_and_dep_table :
    string option ->
    string ->
    use_manifold_cython_client:bool ->
    (Naming_table.t * Path.t, string) result

  (* Updates the naming table with changed files.
     Returns the path to the dep table downloaded. *)
  val update_naming_table :
    Naming_table.t ->
    Path.t ->
    Relative_path.t list option ->
    (string, string) result

  val fetch_and_cache_remote_decls :
    ctx:Provider_context.t ->
    Naming_table.t ->
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
  artifact_store_config: ArtifactStore.config;
  bin_root: Path.t;
  check_id: string;
  ci_info: Ci_util.info option Future.t option;
  ctx: Provider_context.t;
  init_id: string;
  init_start_t: float;
  key: string;
  nonce: Int64.t;
  transport_channel: string option;
  root: Path.t;
  naming_table_base: 'naming_table;
  timeout: int;
  mode: HulkStrategy.hulk_mode;
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
    ~(transport_channel : string option)
    ~(root : Path.t)
    ~(mode : HulkStrategy.hulk_mode)
    ~(cache_remote_decls : bool)
    ~(use_shallow_decls_saved_state : bool)
    ~(saved_state_manifold_path : string option)
    ~(shallow_decls_manifold_path : string option)
    ?(timeout = (600 : int))
    (artifact_store_config : ArtifactStore.config)
    (server : (module RemoteServerApi with type naming_table = 'naming_table)) :
    'naming_table work_env =
  {
    artifact_store_config;
    bin_root;
    check_id;
    ci_info;
    ctx;
    init_id;
    init_start_t;
    key;
    nonce;
    transport_channel;
    naming_table_base = None;
    root;
    mode;
    cache_remote_decls;
    use_shallow_decls_saved_state;
    saved_state_manifold_path;
    shallow_decls_manifold_path;
    timeout;
    server;
  }

let go _ = failwith "not implemented"
