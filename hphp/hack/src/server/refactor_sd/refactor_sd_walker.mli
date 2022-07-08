(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Refactor_sd_types

val callable :
  Tast_env.t ->
  Tast.fun_param list ->
  Tast.func_body ->
  string ->
  constraint_ list

val program :
  Provider_context.t -> Tast.program -> string -> constraint_ list SMap.t
