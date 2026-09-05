(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(* For a file [f], let [defs(f)] be the symbols it defines and [rdeps(s)] the
   files referencing a symbol [s]. [f] is a seed when

     (U_{s in defs(f)} rdeps(s)) \ {f}  =  {}

   that is, when every reference to everything [f] defines comes from [f]
   itself.

   [List.exists] stops at the first [s] whose [rdeps(s)] escapes [f], which for
   most files is the first one, so the usual cost is a single reverse-dep
   lookup rather than one per symbol. *)
let has_external_dependents ctx deps_mode path file_info =
  Typing_deps.deps_of_file_info file_info
  |> List.exists ~f:(fun dep ->
         let dependents =
           Typing_deps.get_ideps_from_hash deps_mode dep
           |> Naming_provider.get_files ctx
         in
         not
           (Relative_path.Set.is_empty
              (Relative_path.Set.remove dependents path)))

let go (_genv : ServerEnv.genv) (env : ServerEnv.env) : Relative_path.t list =
  let ctx = Provider_utils.ctx_from_server_env env in
  let deps_mode = Provider_context.get_deps_mode ctx in
  let naming_table = env.ServerEnv.naming_table in
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
        && not (has_external_dependents ctx deps_mode path file_info)
      then
        path :: seeds
      else
        seeds)
  |> List.sort ~compare:Relative_path.compare
