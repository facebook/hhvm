(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type size = int

module type Entry = sig
  type key

  type value

  val get_size : value -> size
end

module Cache (Entry : Entry) = struct
  type telemetry = {
    time_spent: float;
    peak_size: size;
    num_evictions: int;
  }

  let empty_telemetry = { peak_size = 0; time_spent = 0.; num_evictions = 0 }

  type entry = {
    size: size;
    last_used_timestamp: int;
    value: Entry.value;
  }

  type t = {
    mutable timestamp: int;
    mutable total_size: size;
    mutable telemetry: telemetry;
    max_size: size;
    entries: (Entry.key, entry) Hashtbl.t;
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

  let add_internal (t : t) (key : Entry.key) (value : Entry.value) : entry =
    begin
      match Hashtbl.find_and_remove t.entries key with
      | None -> ()
      | Some { size; _ } ->
        t.total_size <- t.total_size - size;
        assert (t.total_size >= 0)
    end;

    incr_timestamp t;
    let entry =
      { size = Entry.get_size value; last_used_timestamp = t.timestamp; value }
    in
    Hashtbl.add_exn t.entries key entry;
    t.total_size <- t.total_size + entry.size;
    t.telemetry <-
      { t.telemetry with peak_size = max t.telemetry.peak_size t.total_size };
    trim_to_memory_limit t;
    entry

  let add (t : t) ~(key : Entry.key) ~(value : Entry.value) : unit =
    let start_time = Unix.gettimeofday () in
    let (_ : entry) = add_internal t key value in
    time_internal t start_time;
    ()

  let find_or_add (t : t) ~(key : Entry.key) ~(default : unit -> Entry.value) :
      Entry.value =
    let start_time = ref (Unix.gettimeofday ()) in
    let entry =
      Hashtbl.find_and_call
        t.entries
        key
        ~if_found:(fun entry -> entry)
        ~if_not_found:(fun key ->
          time_internal t !start_time;
          let value = default () in
          start_time := Unix.gettimeofday ();
          add_internal t key value)
    in
    trim_to_memory_limit t;
    time_internal t !start_time;
    entry.value

  let remove (t : t) ~(key : Entry.key) : unit =
    let start_time = Unix.gettimeofday () in
    Hashtbl.remove t.entries key;
    time_internal t start_time;
    ()

  let reset_telemetry (t : t) : unit = t.telemetry <- empty_telemetry

  let get_telemetry (t : t) : telemetry = t.telemetry
end
