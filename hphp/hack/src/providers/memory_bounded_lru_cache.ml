(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel

type 'v entry = {
  size_in_words: int;
  last_used_timestamp: int;
  value: 'v;
}

type ('k, 'v) t = {
  mutable timestamp: int;
  mutable total_size_in_words: int;
  max_size_in_words: int;
  entries: ('k, 'v entry) Hashtbl.t;
}

let make ~(max_size_in_words : int) : ('k, 'v) t =
  {
    timestamp = 0;
    total_size_in_words = 0;
    max_size_in_words;
    entries = Hashtbl.Poly.create ();
  }

let clear (t : ('k, 'v) t) : unit =
  t.timestamp <- 0;
  t.total_size_in_words <- 0;
  Hashtbl.clear t.entries

let incr_timestamp (t : ('k, 'v) t) : unit =
  let new_timestamp = t.timestamp + 1 in
  t.timestamp <- new_timestamp

let trim_to_memory_limit (t : ('k, 'v) t) : unit =
  (* Do a linear search and evict the least-recent entry, and repeat until we're
  below the memory threshold. NOTE: We could make this more efficient by using a
  linked hash map instead, which would let you access the next entry to be
  evicted in O(1) time. *)
  while t.total_size_in_words > t.max_size_in_words do
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
      t.total_size_in_words <- t.total_size_in_words - value.size_in_words
    | None ->
      (* Probably shouldn't get here. *)
      assert (t.total_size_in_words = 0)
  done

let add_internal (t : ('k, 'v) t) (key : 'k) (value : 'v) : 'v entry =
  begin
    match Hashtbl.find_and_remove t.entries key with
    | None -> ()
    | Some { size_in_words; _ } ->
      t.total_size_in_words <- t.total_size_in_words - size_in_words;
      assert (t.total_size_in_words >= 0)
  end;

  incr_timestamp t;
  let entry =
    {
      size_in_words = Obj.reachable_words (Obj.repr value);
      last_used_timestamp = t.timestamp;
      value;
    }
  in
  Hashtbl.add_exn t.entries key entry;
  t.total_size_in_words <- t.total_size_in_words + entry.size_in_words;
  trim_to_memory_limit t;
  entry

let add (t : ('k, 'v) t) ~(key : 'k) ~(value : 'v) : unit =
  let (_ : 'a entry) = add_internal t key value in
  ()

let find_or_add (t : ('k, 'v) t) ~(key : 'k) ~(default : unit -> 'v) : 'v =
  let entry =
    Hashtbl.find_and_call
      t.entries
      key
      ~if_found:(fun entry -> entry)
      ~if_not_found:(fun key -> add_internal t key (default ()))
  in
  trim_to_memory_limit t;
  entry.value

let remove (t : ('k, 'v) t) ~(key : 'k) : unit = Hashtbl.remove t.entries key
