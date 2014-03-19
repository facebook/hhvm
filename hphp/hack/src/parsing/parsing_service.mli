(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Utils

val empty_file_info: FileInfo.t

val go:
  Worker.t list option ->
  FileInfo.t SMap.t ->
  get_next:(unit -> string list) ->
  FileInfo.t SMap.t * Utils.error list * SSet.t

(* used by hack build *)
val legacy_php_file_info: (string -> FileInfo.t) ref
