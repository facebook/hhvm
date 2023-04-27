(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val find_route :
  Typing_env_types.env -> classish:string -> ancestor:string -> string list

val describe_route :
  Typing_env_types.env ->
  classish:string ->
  ancestor:string ->
  (Pos_or_decl.t * string) list

val describe_route_via :
  Typing_env_types.env ->
  classish:string ->
  ancestor:string ->
  via:string ->
  (Pos_or_decl.t * string) list
