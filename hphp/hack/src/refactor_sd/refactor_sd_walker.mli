(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Refactor_sd_types

val callable :
  string ->
  Tast_env.t ->
  Tast.fun_param list ->
  Tast.func_body ->
  constraint_ list

val program :
  string -> Provider_context.t -> Tast.program -> constraint_ list SMap.t
