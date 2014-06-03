(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* Contains all the inotify context *)
type env 

(* A subscription to events for a directory *)
type watch

type event = {
  watch : watch;
  filename : string;
}

val init : unit -> env
val add_watch : env -> string -> watch
val rm_watch : env -> watch -> unit
val read : env -> event list
