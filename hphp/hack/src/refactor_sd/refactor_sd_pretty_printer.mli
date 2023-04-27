(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Refactor_sd_types

val show_constraint_ : Typing_env_types.env -> constraint_ -> string

val show_refactor_sd_result :
  Typing_env_types.env -> refactor_sd_result -> string
