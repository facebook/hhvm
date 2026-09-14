(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** For a file [f], let [defs(f)] be the dep hashes of the symbols it defines.
    [f] is a seed when nothing outside [f] depends on any of them:

      add_typing_deps(defs(f)) \ defs(f)  =  {}

    [add_typing_deps] grows a set by its direct dependents, and comparing hashes
    this way needs no naming-table lookup to resolve a dependent back to a file.

    [defs(f)] holds only top-level hashes, so that difference is every dependent
    hash that is not a top-level definition of [f] — a superset of the dependents
    living outside [f], since one recorded at member granularity inside [f] is
    not in [defs(f)] either. The test is conservative in the safe direction: it
    can withhold seed status from a file that deserves it, never grant it to one
    that does not. *)
let has_external_dependents deps_mode file_info =
  let own =
    Typing_deps.deps_of_file_info file_info |> Typing_deps.DepSet.of_list
  in
  let dependents = Typing_deps.add_typing_deps deps_mode own in
  not (Typing_deps.DepSet.is_empty (Typing_deps.DepSet.diff dependents own))

let go (_genv : Server_env.genv) (env : Server_env.env) : Relative_path.t list =
  let ctx = Provider_utils.ctx_from_server_env env in
  let deps_mode = Provider_context.get_deps_mode ctx in
  let naming_table = env.Server_env.naming_table in
  (* One whole-repo naming table scan, unavoidable for a whole-repo query. The
     [file_info] it yields is used in place; re-deriving it per file would cost
     a SQLite SELECT each. *)
  Naming_table.fold
    ~warn_on_naming_costly_iter:false
    naming_table
    ~init:[]
    ~f:(fun path file_info seeds ->
      if
        Relative_path.is_root (Relative_path.prefix path)
        && not (has_external_dependents deps_mode file_info)
      then
        path :: seeds
      else
        seeds)
  |> List.sort ~compare:Relative_path.compare
