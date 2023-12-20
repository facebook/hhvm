(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type t = {
  dummy_one: bool;
  dummy_two: bool;
  dummy_three: bool;
  optimized_member_fanout: bool;
  optimized_parent_fanout: bool;
  optimized_attribute_fanout: bool;
  new_naming_table: bool;
}
[@@deriving eq, show]

(* The names of these should be exactly the capitalized flag names.
   For example for a flag named

      my_flag

   use the enum value

      My_flag
*)
type flag =
  | Dummy_one
  | Dummy_two
  | Dummy_three
  | Optimized_member_fanout
  | Optimized_parent_fanout
  | Optimized_attribute_fanout
  | New_naming_table
[@@deriving show { with_path = false }]

type flag_name = string

let flag_name flag = String.lowercase (show_flag flag)

external make_rust : int -> bool -> string option -> bool -> t
  = "make_saved_state_rollouts"

external get_current_rollout_flag : int -> string option
  = "get_current_rollout_flag"

external default_saved_state_rollouts : unit -> t
  = "default_saved_state_rollouts"

(** @param  get_default   typically an external config reading, e.g. from JK *)
let make
    ~current_rolled_out_flag_idx
    ~(deactivate_saved_state_rollout : bool)
    ~(get_default : flag_name -> bool)
    ~(force_flag_value : string option) =
  let default =
    match get_current_rollout_flag current_rolled_out_flag_idx with
    | Some flag_name -> get_default flag_name
    | None -> false
  in
  make_rust
    current_rolled_out_flag_idx
    deactivate_saved_state_rollout
    force_flag_value
    default

let default : t = default_saved_state_rollouts ()

let output t =
  let print_flag flag value =
    Printf.eprintf "%s = %b\n" (flag_name flag) value
  in
  let {
    dummy_one;
    dummy_two;
    dummy_three;
    optimized_member_fanout;
    optimized_parent_fanout;
    optimized_attribute_fanout;
    new_naming_table;
  } =
    t
  in
  print_flag Dummy_one dummy_one;
  print_flag Dummy_two dummy_two;
  print_flag Dummy_three dummy_three;
  print_flag Optimized_member_fanout optimized_member_fanout;
  print_flag Optimized_parent_fanout optimized_parent_fanout;
  print_flag Optimized_attribute_fanout optimized_attribute_fanout;
  print_flag New_naming_table new_naming_table;
  ()

let to_bit_array_string t : string =
  let s : bool -> string = function
    | true -> "1"
    | false -> "0"
  in
  let {
    dummy_one;
    dummy_two;
    dummy_three;
    optimized_member_fanout;
    optimized_parent_fanout;
    optimized_attribute_fanout;
    new_naming_table;
  } =
    t
  in
  s dummy_one
  ^ s dummy_two
  ^ s dummy_three
  ^ s optimized_member_fanout
  ^ s optimized_parent_fanout
  ^ s optimized_attribute_fanout
  ^ s new_naming_table

let to_hh_json t : Hh_json.json =
  let {
    dummy_one = _;
    dummy_two = _;
    dummy_three = _;
    optimized_member_fanout;
    optimized_parent_fanout;
    optimized_attribute_fanout;
    new_naming_table;
  } =
    t
  in
  Hh_json.JSON_Object
    [
      ("optimized_member_fanout", Hh_json.bool_ optimized_member_fanout);
      ("optimized_parent_fanout", Hh_json.bool_ optimized_parent_fanout);
      ("optimized_attribute_fanout", Hh_json.bool_ optimized_attribute_fanout);
      ("new_naming_table", Hh_json.bool_ new_naming_table);
    ]
