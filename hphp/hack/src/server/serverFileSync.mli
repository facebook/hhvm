(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

val open_file : ServerEnv.env -> string -> string -> ServerEnv.env

val edit_file :
  ServerEnv.env ->
  string -> File_content.code_edit list -> ServerEnv.env

val close_file : ServerEnv.env -> string -> ServerEnv.env

val clear_sync_data : ServerEnv.env -> ServerEnv.env

val try_relativize_path : string -> Relative_path.t option
