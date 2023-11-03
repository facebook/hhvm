(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type by_names [@@deriving yojson_of]

type t [@@deriving yojson_of]

val is_enabled : TypecheckerOptions.t -> bool

val map :
  Provider_context.t -> Relative_path.t -> Tast.by_names -> Errors.t -> t

val reduce : t -> t -> t

val finalize :
  progress:(string -> unit) ->
  init_id:string ->
  recheck_id:string option ->
  t ->
  unit

val hash_tasts_by_file :
  Tast.by_names Relative_path.Map.t -> by_names Relative_path.Map.t
