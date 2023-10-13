(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type flag_name = string

type t = {
  dummy_one: bool;  (** Some documentation for dummy_one *)
  dummy_two: bool;  (** Some documentation for dummy_two *)
  dummy_three: bool;  (** Some documentation for dummy_three *)
  optimized_member_fanout: bool;
  optimized_parent_fanout: bool;
}
[@@deriving eq, show]

val default : t

val make :
  current_rolled_out_flag_idx:int ->
  deactivate_saved_state_rollout:bool ->
  get_default:(flag_name -> bool) ->
  force_flag_value:string option ->
  t

val output : t -> unit

val to_bit_array_string : t -> string

val to_hh_json : t -> Hh_json.json
