(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Category = struct
  type t =
    | Decl_accessors
    | Disk_cat
    | Get_ast
    | Typecheck
  [@@deriving ord]

  let all = [Decl_accessors; Disk_cat; Get_ast]

  let to_string (category : t) : string =
    match category with
    | Decl_accessors -> "decl_accessors"
    | Disk_cat -> "disk_cat"
    | Get_ast -> "get_ast"
    | Typecheck -> "typecheck"
end

module CategoryMap = WrappedMap.Make (Category)
module C = Category
module M = CategoryMap

type time_in_sec = float

type counter = {
  count: int;  (** how many times did 'count' get called? *)
  time: time_in_sec;  (** cumulative duration of all calls to 'count' *)
  is_counting: bool;
      (** to avoid double-counting when a method calls 'count' and so does a nested one *)
}

let empty = { is_counting = false; count = 0; time = 0. }

type t = {
  enable: bool;  (** 'enable' controls whether any counting is done at all *)
  counters: counter CategoryMap.t;  (** here we store each individual counter. *)
}

let state : t ref = ref { enable = false; counters = CategoryMap.empty }

let restore_state (new_state : t) : unit = state := new_state

let reset ~(enable : bool) : t =
  let old_state = !state in
  let counters =
    CategoryMap.of_function Category.all (fun _category -> empty)
  in
  state := { enable; counters };
  old_state

let get_counter (category : Category.t) : counter =
  M.find_opt category !state.counters |> Option.value ~default:empty

let set_counter (category : Category.t) (counts : counter) : unit =
  state := { !state with counters = M.add category counts !state.counters }

let count (category : Category.t) (f : unit -> 'a) : 'a =
  let tally = get_counter category in
  if (not !state.enable) || tally.is_counting then
    (* is_counting is to avoid double-counting, in the case that a method calls 'count'
    and then a nested method itself also calls 'count'. *)
    f ()
  else begin
    set_counter category { tally with is_counting = true };
    let start_time = Unix.gettimeofday () in
    Utils.try_finally ~f ~finally:(fun () ->
        set_counter
          category
          {
            is_counting = false;
            count = tally.count + 1;
            time = tally.time +. Unix.gettimeofday () -. start_time;
          })
  end

let count_decl_accessor (f : unit -> 'a) : 'a = count C.Decl_accessors f

let count_disk_cat (f : unit -> 'a) : 'a = count C.Disk_cat f

let count_get_ast (f : unit -> 'a) : 'a = count C.Get_ast f

let count_typecheck (f : unit -> 'a) : 'a = count C.Typecheck f

let read_time (category : Category.t) : time_in_sec =
  (get_counter category).time

let get_counters () : Telemetry.t =
  let telemetry_of_counter counter =
    Telemetry.create ()
    |> Telemetry.int_ ~key:"count" ~value:counter.count
    |> Telemetry.float_ ~key:"time" ~value:counter.time
  in
  let telemetry =
    M.fold
      (fun category counter telemetry ->
        let telemetry =
          Telemetry.object_
            telemetry
            ~key:(Category.to_string category)
            ~value:(telemetry_of_counter counter)
        in
        telemetry)
      !state.counters
      (Telemetry.create ())
  in
  telemetry
