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
  (* counts decl cache misses, if enabled *)
  decl_cache_misses_counter: int;
  (* counts time spent fetching missing decls, if ennabled *)
  decl_cache_misses_time: float;
  (* threshold is checked against counter by 'should_defer', if enabled *)
  threshold_opt: int option;
}

let state : deferred_decl_state ref =
  ref
    {
      enabled = true;
      deferments = Relative_path.Set.empty;
      decl_cache_misses_counter = 0;
      decl_cache_misses_time = 0.;
      threshold_opt = None;
    }

let count_decl_cache_miss (name : string) ~(start_time : float) : unit =
  if !state.enabled then
    let () = Hh_logger.debug "Decl cache miss: %s" name in
    let t = Unix.gettimeofday () -. start_time in
    state :=
      {
        !state with
        decl_cache_misses_counter = !state.decl_cache_misses_counter + 1;
        decl_cache_misses_time = !state.decl_cache_misses_time +. t;
      }

let raise_if_should_defer ~(d : Relative_path.t) : unit =
  if !state.enabled then
    match !state.threshold_opt with
    | Some threshold when !state.decl_cache_misses_counter >= threshold ->
      raise (Defer d)
    | _ -> ()

let add_deferment ~(d : deferment) : unit =
  state :=
    { !state with deferments = Relative_path.Set.add !state.deferments d }

let restore_state (new_state : deferred_decl_state) : unit = state := new_state

let reset ~(enable : bool) ~(threshold_opt : int option) : deferred_decl_state =
  let old_state = !state in
  state :=
    {
      enabled = enable;
      decl_cache_misses_counter = 0;
      decl_cache_misses_time = 0.;
      deferments = Relative_path.Set.empty;
      threshold_opt;
    };
  old_state

let get_deferments ~(f : deferment -> 'a) : 'a list =
  Relative_path.Set.fold !state.deferments ~init:[] ~f:(fun d l -> f d :: l)

let get_decl_cache_misses_counter () : int = !state.decl_cache_misses_counter

let get_decl_cache_misses_time () : float = !state.decl_cache_misses_time
