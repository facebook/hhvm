(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Types = struct
  type job_id = Job_id of Int64.t [@@deriving show, eq]

  type nonce = Nonce of Int64.t [@@deriving show, eq]
end

module type S = sig
  (* include module type of Types *)

  type command

  type status [@@deriving show]

  (* Note on the nonce parameter: it is up to the caller to specify a nonce that
     results in the desired behavior, such as grouping multiple commands together.
     It can be used later in `begin_cancel` to cancel a group of commands, for example. *)
  val create_command :
    nonce:string ->
    tenant:string ->
    keys:string list ->
    hash:string ->
    check_id:string ->
    root:string ->
    min_log_level:Hh_logger.Level.t ->
    version_specifier:string option ->
    eden:bool ->
    cache_remote_decls:bool ->
    use_shallow_decls_saved_state:bool ->
    saved_state_manifold_path:string option ->
    shallow_decls_manifold_path:string option ->
    command Future.t

  val is_alive : status -> bool

  (* Cancels a group of running commands using the nonce they were created with. *)
  val begin_cancel_batch : Types.nonce -> (status list, string) result Future.t

  val begin_cancel : Types.job_id -> (status, string) result Future.t

  val begin_run : command:command -> (Types.job_id list, string) result Future.t

  val run : command:command -> (Types.job_id list, string) result
end
