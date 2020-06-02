(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module Types = struct
  type job_id = int [@@deriving show, eq]
end

module type S = sig
  include module type of Types

  type command

  type status [@@deriving show]

  val create_command :
    key:string ->
    hash:string ->
    check_id:string ->
    transport_channel:string option ->
    file_system_mode:string ->
    defer_class_declaration_threshold:int ->
    root:string ->
    min_log_level:Hh_logger.Level.t ->
    version_specifier:string option ->
    eden:bool ->
    command

  val is_alive : status -> bool

  val begin_cancel : job_id -> (status, string) result Future.t

  val begin_heartbeat : job_id -> (status, string) result Future.t

  val begin_run :
    command:command -> root:string -> (job_id, string) result Future.t

  val run : command:command -> root:string -> (job_id, string) result
end
