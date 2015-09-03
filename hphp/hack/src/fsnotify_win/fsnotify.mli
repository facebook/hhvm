(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* This is a replicate of `fsnotify_linux/fsnotify.mli`.` *)

exception Error of string * int
type env
type watch
type event = {
  path : string;
  wpath : string;
}
val init : string list -> env
val add_watch : env -> string -> watch option
type fd_select = Unix.file_descr * (unit -> unit)
val select :
  env ->
  ?read_fdl:(fd_select list) ->
  ?write_fdl:(fd_select list) ->
  timeout:float ->
  (event list -> unit) ->
  unit
