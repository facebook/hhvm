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
  optimized_private_member_fanout: bool;
  optimized_parent_fanout: bool;
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
  | Optimized_private_member_fanout
  | Optimized_parent_fanout
[@@deriving show { with_path = false }]

type flag_name = string

(** This module allows to handle the config flag 'ss_force',
  which can have the following values:
  'prod' or 'candidate' or 'prod_with_flag_on:my_flag_name' where 'my_flag_name'
  is a saved state flag from this module.

  This is useful to force using (or creating) the production or candidate saved state,
  or for using/creating an alternative candidate saved state
  (with 'prod_with_flag_on:my_flag_name') *)
module ForcedFlags : sig
  (** Type to represent the possible values of config flag 'ss_force' *)
  type t

  (** Parse value of config flag 'ss_force' *)
  val parse : force_flag_value:string option -> t option

  (** Return the forced value of the current rollout flag, if any.
        Returning None means there is no forcing. *)
  val rollout_flag_value : t option -> bool option

  (** Whether a specific flag is forced to be on. *)
  val is_forced : flag_name -> t option -> bool
end = struct
  type additional_forced_flag = flag_name option

  type t =
    | Prod of additional_forced_flag
    | Candidate

  let force_flag_name : flag_name = "ss_force"

  let parse ~(force_flag_value : string option) : t option =
    Option.bind force_flag_value ~f:(function
        | "production"
        | "prod" ->
          Some (Prod None)
        | "candidate" -> Some Candidate
        | forced ->
          let invalid () =
            Hh_logger.warn
              "Invalid value for flag %s: %s"
              force_flag_name
              forced;
            None
          in
          (match String.split forced ~on:':' with
          | [forced; forced_flag] ->
            (match forced with
            | "prod_with_flag_on"
            | "production_with_flag_on" ->
              Some (Prod (Some forced_flag))
            | _ -> invalid ())
          | _ -> invalid ()))

  let rollout_flag_value (force : t option) : bool option =
    Option.map force ~f:(function
        | Prod _ -> false
        | Candidate -> true)

  let is_forced (flag_name : flag_name) (force : t option) : bool =
    match force with
    | Some (Prod (Some forced)) -> String.equal forced flag_name
    | _ -> false
end

let flag_name flag = String.lowercase (show_flag flag)

(* We need to guarantee that for all flag combinations, there is an available saved
   state corresponding to that combination. There are however an exponential number
   of flag combinations: 2^n where n is the total number of flags.
   What follows allows restricting the number of possible combinations per www revision
   to just two (one for the production saved state, one for the candidate saved state),
   by allowing only one flag to be rolled out at a time (per www revision).

   We do so by specifying a rollout order for the flags, in this function.
   Just like at the butcher, where each customer takes a ticket with a number
   to enter the queue, each new flag gets assigned a number by the developer
   adding the flag below.

   Then for each www revision, .hhconfig specifies which flag can be rolled
   out for that revision, using the following flag:

       current_saved_state_rollout_flag_index = N

   That flag is the equivalent of the counter at the butcher calling customers to be served.
   It means that for all www revisions with this value N in .hhconfig,
   only the flag assigned number N will get its value from JustKnob,
   while the other flags' values are determined by their order:
   * flags whose order is lower than the current flag index are considered to have
     been already rolled out and therefore have their values set to true,
   * flags whose order is greater are yet to be rollout and therefore have their
     values set to false. *)
let rollout_order =
  (* This needs to be specified manually instead of using ppx_enum
     because we want the indices to stay consistent when we remove flags.

     When adding a flag here, do follow these two rules:
     * The chosen number should be different from than any other assigned number.
     * The chosen number should be greater than the current value for
       current_saved_state_rollout_flag_index in .hhconfig.

     It's ok to reassign numbers to flags that haven't yet been rolled out
     if you want your new flag to be rolled out first. *)
  function
  | Dummy_one -> 0
  | Dummy_two -> 1
  | Dummy_three -> 2
  | Optimized_member_fanout -> 4
  | Optimized_private_member_fanout -> 5
  | Optimized_parent_fanout -> 6

let make
    ~current_rolled_out_flag_idx
    ~(deactivate_saved_state_rollout : bool)
    ~(get_default : flag_name -> bool)
    ~(force_flag_value : string option) =
  let force_prod_or_candidate = ForcedFlags.parse ~force_flag_value in
  let get_flag_value flag =
    let i = rollout_order flag in
    let flag_name = flag_name flag in
    if Int.equal current_rolled_out_flag_idx i then
      ForcedFlags.rollout_flag_value force_prod_or_candidate
      |> Option.value
           ~default:
             ((not deactivate_saved_state_rollout) && get_default flag_name)
    else if Int.(current_rolled_out_flag_idx < i) then
      (* This flag will be rolled out next, so return false unless forced. *)
      ForcedFlags.is_forced flag_name force_prod_or_candidate
    else
      (* This flag has already been rolled out *)
      true
  in
  {
    dummy_one = get_flag_value Dummy_one;
    dummy_two = get_flag_value Dummy_two;
    dummy_three = get_flag_value Dummy_three;
    optimized_member_fanout = get_flag_value Optimized_member_fanout;
    optimized_private_member_fanout =
      get_flag_value Optimized_private_member_fanout;
    optimized_parent_fanout = get_flag_value Optimized_parent_fanout;
  }

let default : t =
  make
    ~current_rolled_out_flag_idx:Int.min_value
    ~deactivate_saved_state_rollout:false
    ~get_default:(fun _ -> false)
    ~force_flag_value:None

let output t =
  let print_flag flag value =
    Printf.eprintf "%s = %b\n" (flag_name flag) value
  in
  let {
    dummy_one;
    dummy_two;
    dummy_three;
    optimized_member_fanout;
    optimized_private_member_fanout;
    optimized_parent_fanout;
  } =
    t
  in
  print_flag Dummy_one dummy_one;
  print_flag Dummy_two dummy_two;
  print_flag Dummy_three dummy_three;
  print_flag Optimized_member_fanout optimized_member_fanout;
  print_flag Optimized_private_member_fanout optimized_private_member_fanout;
  print_flag Optimized_parent_fanout optimized_parent_fanout;
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
    optimized_private_member_fanout;
    optimized_parent_fanout;
  } =
    t
  in
  s dummy_one
  ^ s dummy_two
  ^ s dummy_three
  ^ s optimized_member_fanout
  ^ s optimized_private_member_fanout
  ^ s optimized_parent_fanout

let to_hh_json t : Hh_json.json =
  let {
    dummy_one = _;
    dummy_two = _;
    dummy_three = _;
    optimized_member_fanout;
    optimized_private_member_fanout;
    optimized_parent_fanout;
  } =
    t
  in
  Hh_json.JSON_Object
    [
      ("optimized_member_fanout", Hh_json.bool_ optimized_member_fanout);
      ( "optimized_private_member_fanout",
        Hh_json.bool_ optimized_private_member_fanout );
      ("optimized_parent_fanout", Hh_json.bool_ optimized_parent_fanout);
    ]
