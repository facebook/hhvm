(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type load_mini_approach =
  | Load_mini_script of Path.t
  | Precomputed of ServerArgs.mini_state_target_info
  (** Load a saved state using Ocaml implementation of saved state loader. *)
  | Load_state_natively
  (* Use the supplied saved state target to skip lookup in XDB. *)
  | Load_state_natively_with_target of ServerMonitorUtils.target_mini_state

(* Saves the state that load_mini_script below reads in *)
val save_state: ServerEnv.env -> string -> unit

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

val init_to_save_state : ServerEnv.genv -> ServerEnv.env 
