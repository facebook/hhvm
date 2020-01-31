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

type state = {
  enabled: bool;
  deferments: deferments_t;
  counter: int;
  threshold_opt: int option;
}

let state : state ref =
  ref
    {
      enabled = true;
      deferments = Relative_path.Set.empty;
      counter = 0;
      threshold_opt = None;
    }

let reset ~(enable : bool) ~(threshold_opt : int option) : unit =
  state :=
    {
      enabled = enable;
      counter = 0;
      deferments = Relative_path.Set.empty;
      threshold_opt;
    }

let increment_counter () : unit =
  if !state.enabled then state := { !state with counter = !state.counter + 1 }

let raise_if_should_defer ~(d : Relative_path.t) : unit =
  match (!state.enabled, !state.threshold_opt) with
  | (true, Some threshold) when !state.counter >= threshold -> raise (Defer d)
  | _ -> ()

let add_deferment ~(d : deferment) : unit =
  state :=
    { !state with deferments = Relative_path.Set.add !state.deferments d }

let get_deferments ~(f : deferment -> 'a) : 'a list =
  !state.deferments |> Relative_path.Set.elements |> List.map ~f
