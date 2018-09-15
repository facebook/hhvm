(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type load_mini_approach =
  | Precomputed of ServerArgs.mini_state_target_info
  (** Load a saved state using Ocaml implementation of saved state loader.
   * Bool is true when using canary, i.e. look up saved state by hg commit
   * hash instead of nearest SVN rev. *)
  | Load_state_natively of bool
  (* Use the supplied saved state target to skip lookup in XDB. *)
  | Load_state_natively_with_target of ServerMonitorUtils.target_mini_state

(* Saves the state that is used by init below. *)
val save_state: ServerEnv.genv -> ServerEnv.env -> string -> unit

type init_result =
  (** Loaded a mini saved state of this distance. Note: for older load scripts
   * distance is unknown, thus None. *)
  | Mini_load of int option
  (** Loading a  *)
  | Mini_load_failed of string

(* will parse, name, typecheck, the next set of files
 * and refresh the environment and update the many shared heaps
 *)
val init: ?load_mini_approach:load_mini_approach -> ServerEnv.genv
  -> ServerEnv.env * init_result (* If the script failed, the error message *)
