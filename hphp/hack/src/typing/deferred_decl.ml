(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

exception Defer of Relative_path.t

type deferment = Relative_path.t

type deferments_t = Relative_path.Set.t

type deferred_decl_state = {
  (* if enabled is false, then 'should_defer' is a no-op *)
  enabled: bool;
  (* deferments is grown by 'add' *)
  deferments: deferments_t;
  (* counter is incremented by 'should_defer', if enabled *)
  counter: int;
  (* threshold is checked against counter by 'should_defer', if enabled *)
  threshold_opt: int option;
}

let state : deferred_decl_state ref =
  ref
    {
      enabled = true;
      deferments = Relative_path.Set.empty;
      counter = 0;
      threshold_opt = None;
    }

let should_defer ~(d : Relative_path.t) : unit =
  if !state.enabled then (
    state := { !state with counter = !state.counter + 1 };
    match !state.threshold_opt with
    | Some threshold when threshold < !state.counter -> raise (Defer d)
    | _ -> ()
  )

let add ~(d : deferment) : unit =
  state :=
    { !state with deferments = Relative_path.Set.add !state.deferments d }

let restore_state (new_state : deferred_decl_state) : unit = state := new_state

let reset ~(enable : bool) ~(threshold_opt : int option) : deferred_decl_state =
  let old_state = !state in
  state :=
    {
      enabled = enable;
      counter = 0;
      deferments = Relative_path.Set.empty;
      threshold_opt;
    };
  old_state

let get_deferments ~(f : deferment -> 'a) : 'a list =
  Relative_path.Set.fold !state.deferments ~init:[] ~f:(fun d l -> f d :: l)

let get_counter () : int = !state.counter
