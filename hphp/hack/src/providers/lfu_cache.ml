(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Option.Monad_infix

type size = int

module type Entry = sig
  type _ t

  type 'a key = 'a t

  type 'a value = 'a

  val get_size : key:'a key -> value:'a value -> size

  val key_to_log_string : 'a key -> string
end

module RevIMap = Stdlib.Map.Make (struct
  type t = int

  let compare a b = Int.compare b a
end)

exception Done

module Cache (Entry : Entry) = struct
  type key_wrapper = Key : 'a Entry.key -> key_wrapper

  type value_wrapper = Value_wrapper : 'a Entry.value -> value_wrapper

  type entry = {
    frequency: int ref;
    value: value_wrapper;
  }

  type t = {
    capacity: size;
    entries: (key_wrapper, entry) Hashtbl.t;
  }

  let make_entry value = { frequency = ref 0; value }

  let make ~(max_size : size) : t =
    { capacity = max_size; entries = Hashtbl.Poly.create () }

  let clear (t : t) : unit = Hashtbl.clear t.entries

  let length (t : t) : int = Hashtbl.length t.entries

  (** The collection function is called when we reach twice original
      capacity in size. When the collection is triggered, we only keep
      the most frequently used objects.
      So before collection: size = 2 * capacity
      After collection: size = capacity (with the most frequently
      used objects) *)
  let collect { entries; capacity } =
    if Hashtbl.length entries < 2 * capacity then
      ()
    else
      let sorted_by_freq =
        (* bucket sort *)
        Hashtbl.fold
          ~f:(fun ~key ~data:{ frequency; value } m ->
            RevIMap.add
              !frequency
              ((key, value)
              :: (RevIMap.find_opt !frequency m |> Option.value ~default:[]))
              m)
          entries
          ~init:RevIMap.empty
      in
      Hashtbl.clear entries;
      try
        ignore
        @@ RevIMap.fold
             (fun _freq values count ->
               List.fold values ~init:count ~f:(fun count (key, value) ->
                   Hashtbl.set entries ~key ~data:(make_entry value);
                   let count = count + 1 in
                   if count >= capacity then raise Done;
                   count))
             sorted_by_freq
             0
      with
      | Done -> ()

  let add (type a) (t : t) ~(key : a Entry.key) ~(value : a Entry.value) : unit
      =
    collect t;
    let key = Key key in
    match Hashtbl.find t.entries key with
    | Some { frequency; value = Value_wrapper value' } ->
      incr frequency;
      if phys_equal (Obj.magic value' : a Entry.value) value then
        ()
      else
        Hashtbl.set
          t.entries
          ~key
          ~data:{ frequency; value = Value_wrapper value }
    | None ->
      Hashtbl.set t.entries ~key ~data:(make_entry (Value_wrapper value))

  let find_or_add
      (type a)
      (t : t)
      ~(key : a Entry.key)
      ~(default : unit -> a Entry.value option) : a Entry.value option =
    let entry =
      Hashtbl.find_and_call
        t.entries
        (Key key)
        ~if_found:(fun { value; frequency } ->
          incr frequency;
          Some value)
        ~if_not_found:(fun _key ->
          let value_opt = default () in
          Option.iter value_opt ~f:(fun value -> add t ~key ~value);
          value_opt >>| fun value -> Value_wrapper value)
    in

    match entry with
    | None -> None
    | Some (Value_wrapper value) ->
      (* OCaml [Hashtbl.t] isn't a heterogeneous map. There's no way to indicate
         that the key and the value type have some relation. Consequently, the
         [value] we've just retrieved from the hash table has type
         [$Value_wrapper_'a] but we need one of type ['a], and there's no good way
         to convince the OCaml compiler that these two types are equivalent.

         We hope to reduce the danger of this using this cache as a heterogeneous
         map by having this be the only call to unsafe [Obj] functions, as opposed
         to having every caller call into [Obj]. (The alternative is to implement a
         heterogeneous map from scratch, or import a library for one.) *)
      let value = (Obj.magic value : a Entry.value) in
      Some value

  let remove (t : t) ~(key : 'a Entry.key) : unit =
    Hashtbl.remove t.entries (Key key)

  let get_telemetry (_ : t) ~(key : string) (telemetry : Telemetry.t) :
      Telemetry.t =
    telemetry |> Telemetry.string_ ~key ~value:"LFU telemetry not implemented"

  let reset_telemetry (_ : t) : unit = ()
end
