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

let (deferments : deferments_t ref) = ref Relative_path.Set.empty

let (counter : int ref) = ref 0

let (enabled : bool ref) = ref true

let should_defer ~(d : Relative_path.t) ~(threshold_opt : int option) : unit =
  if !enabled then (
    counter := !counter + 1;
    match threshold_opt with
    | Some threshold when threshold < !counter -> raise (Defer d)
    | _ -> ()
  )

let add ~(d : deferment) : unit =
  deferments := Relative_path.Set.add !deferments d

let reset ~enable =
  deferments := Relative_path.Set.empty;
  counter := 0;
  enabled := enable

let get_deferments ~(f : deferment -> 'a) : 'a list =
  Relative_path.Set.fold !deferments ~init:[] ~f:(fun d l -> f d :: l)

let get_counter () : int = !counter
