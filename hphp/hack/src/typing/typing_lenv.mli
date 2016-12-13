(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Env = Typing_env

(*****************************************************************************)
(* Functions dealing with old style local environment *)
(*****************************************************************************)

val intersect_fake : Env.fake_members -> Env.fake_members -> Env.fake_members
val intersect :
  Env.env -> Env.local_env -> Env.local_env -> Env.local_env -> Env.env
val integrate : Env.env -> Env.local_env -> Env.local_env -> Env.env
val intersect_list :
  Env.env -> Env.local_env -> (bool * Env.local_env) list -> Env.env
val fully_integrate : Env.env -> Env.local_env -> Env.env
val env_with_empty_fakes : Env.env -> Env.env
