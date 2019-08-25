(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open ServerInitTypes

val full_init : ServerEnv.genv -> ServerEnv.env -> ServerEnv.env * float

val parse_only_init : ServerEnv.genv -> ServerEnv.env -> ServerEnv.env * float

val write_symbol_info_init :
  ServerEnv.genv -> ServerEnv.env -> ServerEnv.env * float

val saved_state_init :
  load_state_approach:load_state_approach ->
  ServerEnv.genv ->
  ServerEnv.env ->
  Path.t ->
  ( (ServerEnv.env * float) * (loaded_info * files_changed_while_parsing),
    load_state_error )
  result
