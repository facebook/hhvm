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

  val compare : 'a t -> 'b t -> int

  val hash : 'a t -> int

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

  module KeyWrapper = struct
    type t = key_wrapper

    let compare (Key key1) (Key key2) = Entry.compare key1 key2

    let sexp_of_t (Key key) = Sexp.Atom (Entry.key_to_log_string key)

    let hash (Key key) = Entry.hash key
  end

  type entry = {
    frequency: int ref;
    value: value_wrapper;
  }

  type t = {
    capacity: size;
    entries: (key_wrapper, entry) Hashtbl.t;
    can_collect: bool ref;
    num_added: int ref;
    num_collected: int ref;
    num_collections: int ref;
  }

  let make_entry value = { frequency = ref 0; value }

  let make ~(max_size : size) : t =
    {
      capacity = max_size;
      entries = Hashtbl.create (module KeyWrapper);
      can_collect = ref true;
      num_added = ref 0;
      num_collected = ref 0;
      num_collections = ref 0;
    }

  let clear (t : t) : unit = Hashtbl.clear t.entries

  let length (t : t) : int = Hashtbl.length t.entries

  (** The collection function is called when we reach twice original
      capacity in size. When the collection is triggered, we only keep
      the most frequently used objects.
      So before collection: size = 2 * capacity
      After collection: size = capacity (with the most frequently
      used objects) *)
  let collect
      {
        can_collect;
        entries;
        capacity;
        num_collected;
        num_collections;
        num_added = _;
      } =
    if (not !can_collect) || Hashtbl.length entries < 2 * capacity then
      ()
    else begin
      incr num_collections;
      let prev_length = Hashtbl.length entries in
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
      begin
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
      end;
      num_collected := !num_collected + prev_length - Hashtbl.length entries
    end

  let without_collections (t : t) ~(f : unit -> 'a) : 'a =
    let prev = !(t.can_collect) in
    t.can_collect := false;
    Utils.try_finally ~f ~finally:(fun () -> t.can_collect := prev)

  let add (type a) (t : t) ~(key : a Entry.key) ~(value : a Entry.value) : unit
      =
    collect t;
    let key = Key key in
    match Hashtbl.find t.entries key with
    | Some { frequency; value = Value_wrapper value' } ->
      incr frequency;
      if phys_equal (Obj.magic value' : a Entry.value) value then
        ()
      else begin
        Hashtbl.set
          t.entries
          ~key
          ~data:{ frequency; value = Value_wrapper value }
      end
    | None ->
      incr t.num_added;
      Hashtbl.set t.entries ~key ~data:(make_entry (Value_wrapper value))

  let find (type a) (t : t) ~(key : a Entry.key) : a Entry.value option =
    match Hashtbl.find t.entries (Key key) with
    | None -> None
    | Some { value = Value_wrapper value; frequency = _ } ->
      let value = (Obj.magic value : a Entry.value) in
      Some value

  let keys_as_log_strings (t : t) : string list =
    Hashtbl.keys t.entries
    |> List.map ~f:(fun (Key key) -> Entry.key_to_log_string key)

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

  let get_telemetry (t : t) ~(key : string) (telemetry : Telemetry.t) :
      Telemetry.t =
    Telemetry.object_
      telemetry
      ~key
      ~value:
        (Telemetry.create ()
        |> Telemetry.int_ ~key:"num_added" ~value:!(t.num_added)
        |> Telemetry.int_ ~key:"num_collected" ~value:!(t.num_collected)
        |> Telemetry.int_ ~key:"num_collections" ~value:!(t.num_collections)
        |> Telemetry.int_ ~key:"capacity" ~value:t.capacity
        |> Telemetry.int_ ~key:"length" ~value:(length t))

  let reset_telemetry (t : t) : unit =
    t.num_added := 0;
    t.num_collected := 0;
    t.num_collections := 0;
    ()
end
