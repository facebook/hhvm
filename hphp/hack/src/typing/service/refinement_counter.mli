(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type summary = {
  count: int;
  positions: SSet.t; [@to_yojson yojson_of_pos_set]
}
[@@deriving yojson_of]

type t = {
  legacy_refinements: summary SMap.t;
  new_refinements: int;
  total_refinements: int;
}
[@@deriving yojson_of]

val is_enabled : TypecheckerOptions.t -> bool

val map :
  Provider_context.t -> Relative_path.t -> Tast.by_names -> Diagnostics.t -> t

val reduce : t -> t -> t

val finalize :
  progress:(string -> unit) ->
  init_id:string ->
  recheck_id:string option ->
  t ->
  unit
