(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* TODO(T170647909): In preparation to upgrading to ppx_yojson_conv.v0.16.X.
         Remove the suppress warning when the upgrade is done. *)
[@@@warning "-66"]

open Core
open Option.Monad_infix

(** This is made of sequence [seq] and a hash table [tbl].
    Initially, the hash table is empty while the sequence is not.
    As we traverse the sequence to search for keys, we add them in the
    hash table for future lookups. *)
type 'a t = {
  tbl: ('a * bool) String.Table.t;
  mutable get_single_seq: (string -> 'a Sequence.t) option;
  mutable seq: (string * 'a) Sequence.t;
  is_canonical: 'a -> bool;
  merge: earlier:'a -> later:'a -> 'a;
}

let make ~is_canonical ~merge ?get_single_seq seq =
  let tbl = String.Table.create () in
  { tbl; get_single_seq; seq; is_canonical; merge }

type 'a advance_result =
  | Complete
      (** The cache's [seq] has been exhausted, and its [tbl] now contains the
      complete mapping of all elements. *)
  | Skipped
      (** The cache's [seq] emitted some non-canonical element, which may or may not
      have replaced a previously emitted element stored in the [tbl]. *)
  | Yield of string * 'a
      (** The cache's [seq] emitted this canonical element (along with its ID). This
      element may be immediately used without traversing the rest of the
      sequence (since canonical elements cannot be replaced or updated as we
      traverse the rest of the sequence). *)

(** Fetch the next value from the cache's [seq]. Update its [tbl] by storing the
    new value, ignoring the new value, or merging the new value with an existing
    value as necessary. *)
let advance t =
  match Sequence.next t.seq with
  | None ->
    t.get_single_seq <- None;
    Complete
  | Some ((id, v), rest) ->
    t.seq <- rest;
    let (extant_value, extant_value_is_canonical) =
      match Hashtbl.find t.tbl id with
      | None -> (None, false)
      | Some (v, canonical) -> (Some v, canonical)
    in
    if extant_value_is_canonical then
      Skipped
    else
      let replace_with v =
        let canonical = t.is_canonical v in
        Hashtbl.set t.tbl ~key:id ~data:(v, canonical);
        if canonical then
          Yield (id, v)
        else
          Skipped
      in
      (match extant_value with
      | None -> replace_with v
      | Some extant_value ->
        let v = t.merge ~earlier:extant_value ~later:v in
        if phys_equal v extant_value then
          Skipped
        else
          replace_with v)

(** Advance the sequence until we have the final version of that element*)
let rec get_from_single_seq t seq result =
  match result with
  | Some (v, true) -> Some v
  | _ ->
    (match Sequence.next seq with
    | None -> Option.map result ~f:fst
    | Some (v, rest) ->
      let result =
        match result with
        | None -> (v, t.is_canonical v)
        | Some ((extant_value, _) as extant_result) ->
          let v = t.merge ~earlier:extant_value ~later:v in
          if phys_equal v extant_value then
            extant_result
          else
            (v, t.is_canonical v)
      in
      get_from_single_seq t rest (Some result))

let get t id =
  let rec search_in_seq () =
    match advance t with
    | Complete ->
      (* We've traversed the entire seq, so whatever element we have cached
         (canonical or not) is the correct value. *)
      Hashtbl.find t.tbl id >>| fst
    | Yield (id', v) when String.equal id' id -> Some v
    | Skipped
    | Yield _ ->
      search_in_seq ()
  in
  match Hashtbl.find t.tbl id with
  | Some (v, true) -> Some v
  | None
  | Some (_, false) ->
    (match t.get_single_seq with
    | None -> search_in_seq ()
    | Some get_sub_seq ->
      let result = get_from_single_seq t (get_sub_seq id) None in
      Option.iter result ~f:(fun v -> Hashtbl.set t.tbl ~key:id ~data:(v, true));
      result)

let mem t id =
  let rec go () =
    if Hashtbl.mem t.tbl id then
      true
    else
      match advance t with
      | Complete -> false
      | Yield (id', _) when String.equal id' id -> true
      | Skipped
      | Yield _ ->
        go ()
  in
  if Hashtbl.mem t.tbl id then
    true
  else
    match t.get_single_seq with
    | None -> go ()
    | Some f ->
      let result = get_from_single_seq t (f id) None in
      Option.iter result ~f:(fun v -> Hashtbl.set t.tbl ~key:id ~data:(v, true));
      Option.is_some result

let rec to_list t =
  match advance t with
  | Skipped
  | Yield _ ->
    to_list t
  | Complete ->
    Hashtbl.fold t.tbl ~init:[] ~f:(fun ~key ~data acc ->
        (key, fst data) :: acc)
