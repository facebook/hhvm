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

  (* Called by the worker to type check a list of files.
    The state filename is where the type checker should save its state that
    changed as a result of type checking the files
    (i.e., the dependency graph) *)
  val type_check :
    Provider_context.t ->
    Relative_path.t list ->
    state_filename:string ->
    Errors.t
end

type 'naming_table work_env = {
  bin_root: Path.t;
  check_id: string;
  ctx: Provider_context.t;
  key: string;
  root: Path.t;
  naming_table_base: 'naming_table;
  timeout: int;
  server: (module RemoteServerApi with type naming_table = 'naming_table);
}

let make_env
    (ctx : Provider_context.t)
    ~(bin_root : Path.t)
    ~(check_id : string)
    ~(key : string)
    ~(root : Path.t)
    ?(timeout = (600 : int))
    (server : (module RemoteServerApi with type naming_table = 'naming_table)) :
    'naming_table work_env =
  {
    bin_root;
    key;
    check_id;
    ctx;
    naming_table_base = None;
    root;
    timeout;
    server;
  }

let go _ = failwith "not implemented"
