(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*****************************************************************************)
(* Functions dealing with old style local environment *)
(*****************************************************************************)

val intersect_fake :
  Typing_env.fake_members ->
  Typing_env.fake_members -> Typing_env.fake_members
val intersect :
  Typing_env.env ->
  Typing_env.local_env ->
  Typing_env.local_env ->
  Typing_env.local_env -> Typing_env.env
val integrate :
  Typing_env.env ->
  Typing_env.local_env ->
  Typing_env.local_env -> Typing_env.env
val intersect_list :
  Typing_env.env ->
  Typing_env.local_env ->
  (bool * Typing_env.local_env) list -> Typing_env.env
val fully_integrate :
  Typing_env.env ->
  Typing_env.local_env -> Typing_env.env
val env_with_empty_fakes :
  Typing_env.env -> Typing_env.env

(*****************************************************************************)
(* Functions dealing with continuation based flow typing of local variables  *)
(*****************************************************************************)

val union_continuations :
  Typing_env.env ->
  Typing_continuations.t ->
  Typing_env.local Local_id.Map.t -> Typing_env.env
val union_local_types :
  ?intersect_fake_members:bool ->
  Typing_env.env ->
  Typing_env.local_env -> Typing_env.env
val append_cont_to_cont :
  Typing_env.env ->
  Typing_continuations.t ->
  Typing_env.local_env ->
  Typing_continuations.t -> Typing_env.env
val terminate_cont :
  Typing_env.env ->
  Typing_continuations.t -> Typing_env.env
val clear_cont :
  Typing_env.env ->
  Typing_continuations.t -> Typing_env.env
val replace_cont_with :
  Typing_env.env ->
  Typing_continuations.t ->
  Typing_env.local_env ->
  Typing_continuations.t -> Typing_env.env
val replace_cont :
  Typing_env.env ->
  Typing_continuations.t ->
  Typing_env.local_env -> Typing_env.env
