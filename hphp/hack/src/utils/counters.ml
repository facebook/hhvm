(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Categories (internal): we're using ints rather than enums just for convenience. *)

let category_decl_accessor = 0

let category_count = 1

type t = {
  count: int;
  time: float;
  is_counting: bool;
}

let empty = { is_counting = false; count = 0; time = 0. }

type state = {
  (* 'enable' controls whether any counting is done at all *)
  enable: bool;
  (* here we store each individual counter. *)
  counters: t array;
}

let state : state ref =
  ref { enable = false; counters = Array.make category_count empty }

let restore_state (new_state : state) : unit = state := new_state

let reset ~(enable : bool) : state =
  let old_state = !state in
  state := { enable; counters = Array.make category_count empty };
  old_state

let count (category : int) (f : unit -> 'a) : 'a =
  let tally = !state.counters.(category) in
  if (not !state.enable) || tally.is_counting then
    (* is_counting is to avoid double-counting, in the case that a method calls 'count'
    and then a nested method itself also calls 'count'. *)
    f ()
  else begin
    !state.counters.(category) <- { tally with is_counting = true };
    let start_time = Unix.gettimeofday () in
    Utils.try_finally ~f ~finally:(fun () ->
        !state.counters.(category) <-
          {
            is_counting = false;
            count = tally.count + 1;
            time = tally.time +. Unix.gettimeofday () -. start_time;
          })
  end

let get_counter (category : int) : t = !state.counters.(category)

let count_decl_accessor (f : unit -> 'a) : 'a = count category_decl_accessor f

let get_decl_accessor_counter () : t = get_counter category_decl_accessor
