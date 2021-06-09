(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type size = int

module type Entry = sig
  type _ t

  type 'a key = 'a t

  type 'a value = 'a

  val get_size : key:'a key -> value:'a value -> size

  val key_to_log_string : 'a key -> string
end

module Cache (Entry : Entry) = struct
  type telemetry = {
    time_spent: float;
    num_evictions: int;
  }

  let empty_telemetry = { time_spent = 0.; num_evictions = 0 }

  type key_wrapper = Key : 'a Entry.key -> key_wrapper

  type value_wrapper = Value_wrapper : 'a Entry.value -> value_wrapper

  type entry = {
    size: size;
    last_used_timestamp: int;
    value: value_wrapper;
  }

  type t = {
    mutable timestamp: int;
    mutable total_size: size;
    mutable telemetry: telemetry;
    max_size: size;
    entries: (key_wrapper, entry) Hashtbl.t;
  }

  let time_internal (t : t) (start_time : float) : unit =
    t.telemetry <-
      {
        t.telemetry with
        time_spent =
          t.telemetry.time_spent +. (Unix.gettimeofday () -. start_time);
      }

  let make ~(max_size : size) : t =
    {
      timestamp = 0;
      total_size = 0;
      telemetry = empty_telemetry;
      max_size;
      entries = Hashtbl.Poly.create ();
    }

  let clear (t : t) : unit =
    t.timestamp <- 0;
    t.total_size <- 0;
    Hashtbl.clear t.entries

  let length (t : t) : int = Hashtbl.length t.entries

  let incr_timestamp (t : t) : unit =
    let new_timestamp = t.timestamp + 1 in
    t.timestamp <- new_timestamp

  let trim_to_memory_limit (t : t) : unit =
    (* Do a linear search and evict the least-recent entry, and repeat until
    we're below the size threshold. NOTE: We could make this more efficient by
    using a linked hash map instead, which would let you access the next entry
    to be evicted in O(1) time. *)
    while t.total_size > t.max_size do
      let oldest_entry =
        Hashtbl.fold t.entries ~init:None ~f:(fun ~key ~data acc ->
            match acc with
            | None -> Some (key, data)
            | Some (old_key, old_data) ->
              if data.last_used_timestamp < old_data.last_used_timestamp then
                Some (key, data)
              else
                Some (old_key, old_data))
      in
      match oldest_entry with
      | Some (key, value) ->
        Hashtbl.remove t.entries key;
        t.total_size <- t.total_size - value.size;
        t.telemetry <-
          { t.telemetry with num_evictions = t.telemetry.num_evictions + 1 }
      | None ->
        (* Probably shouldn't get here. *)
        assert (t.total_size = 0)
    done

  let add_internal (t : t) (key : 'a Entry.key) (value : 'a Entry.value) : entry
      =
    begin
      match Hashtbl.find_and_remove t.entries (Key key) with
      | None -> ()
      | Some { size; _ } ->
        t.total_size <- t.total_size - size;
        assert (t.total_size >= 0)
    end;

    incr_timestamp t;
    let entry =
      {
        size = Entry.get_size ~key ~value;
        last_used_timestamp = t.timestamp;
        value = Value_wrapper value;
      }
    in
    Hashtbl.add_exn t.entries ~key:(Key key) ~data:entry;
    t.total_size <- t.total_size + entry.size;
    trim_to_memory_limit t;
    entry

  let add (t : t) ~(key : 'a Entry.key) ~(value : 'a Entry.value) : unit =
    let start_time = Unix.gettimeofday () in
    let (_ : entry) = add_internal t key value in
    time_internal t start_time;
    ()

  let find_or_add
      (type a)
      (t : t)
      ~(key : a Entry.key)
      ~(default : unit -> a Entry.value option) : a Entry.value option =
    let start_time = ref (Unix.gettimeofday ()) in
    let entry =
      Hashtbl.find_and_call
        t.entries
        (Key key)
        ~if_found:(fun entry -> Some entry)
        ~if_not_found:(fun _key ->
          time_internal t !start_time;
          let value_opt = default () in
          start_time := Unix.gettimeofday ();
          Option.map value_opt ~f:(add_internal t key))
    in
    trim_to_memory_limit t;
    time_internal t !start_time;

    match entry with
    | None -> None
    | Some { value = Value_wrapper value; _ } ->
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
    let start_time = Unix.gettimeofday () in
    begin
      match Hashtbl.find_and_remove t.entries (Key key) with
      | None -> ()
      | Some value -> t.total_size <- t.total_size - value.size
    end;
    time_internal t start_time;
    ()

  let reset_telemetry (t : t) : unit = t.telemetry <- empty_telemetry

  let get_telemetry ~(key : string) (t : t) (telemetry : Telemetry.t) :
      Telemetry.t =
    if length t = 0 then
      telemetry
    else
      let sub_telemetry =
        Telemetry.create ()
        |> Telemetry.float_ ~key:"time_spent" ~value:t.telemetry.time_spent
        |> Telemetry.int_ ~key:"num_evictions" ~value:t.telemetry.num_evictions
        |> Telemetry.int_ ~key:"length" ~value:(length t)
        |> Telemetry.int_ ~key:"total_size" ~value:t.total_size
      in
      Telemetry.object_ telemetry ~key ~value:sub_telemetry
end
