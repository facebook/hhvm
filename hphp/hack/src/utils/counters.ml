(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Categories (internal): we're using ints rather than enums just for convenience. *)

let category_decl_accessor = 0

let category_disk_cat = 1

let category_get_ast = 2

let category_count = 3

type t = {
  (* how many times did 'count' get called? *)
  count: int;
  (* cumulative duration of all calls to 'count' *)
  time: float;
  (* to avoid double-counting when a method calls 'count' and so does a nested one *)
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

let count_decl_accessor (f : unit -> 'a) : 'a = count category_decl_accessor f

let count_disk_cat (f : unit -> 'a) : 'a = count category_disk_cat f

let count_get_ast (f : unit -> 'a) : 'a = count category_get_ast f

let get_counters () : Telemetry.t =
  let telemetry_of_counter counter =
    Telemetry.create ()
    |> Telemetry.int_ ~key:"count" ~value:counter.count
    |> Telemetry.float_ ~key:"time" ~value:counter.time
  in
  Telemetry.create ()
  |> Telemetry.object_
       ~key:"decl_accessors"
       ~value:(telemetry_of_counter !state.counters.(category_decl_accessor))
  |> Telemetry.object_
       ~key:"disk_cat"
       ~value:(telemetry_of_counter !state.counters.(category_disk_cat))
  |> Telemetry.object_
       ~key:"get_ast"
       ~value:(telemetry_of_counter !state.counters.(category_get_ast))
