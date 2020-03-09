(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Type of locals as they appear in instructions.
 * Named variables are those appearing in the .declvars declaration. These
 * can also be referenced by number (0 to n-1), but we use Unnamed only for
 * variables n and above not appearing in .declvars
 *)
type t =
  | Unnamed of int
  (* Named local, necessarily starting with `$` *)
  | Named of string

module SN = Naming_special_names

(* use dedicated locals to store label id and return value similar to HHVM *)
let label_id_local = ref None

let retval_local = ref None

let next_local = ref 0

let temp_local_map = ref SMap.empty

let rec get_unnamed_local_id () =
  let current = !next_local in
  next_local := current + 1;

  (* make sure that newly allocated local don't stomp on dedicated locals  *)
  match !label_id_local with
  | Some (Unnamed v) when current = v -> get_unnamed_local_id ()
  | _ ->
    (match !retval_local with
    | Some (Unnamed v) when current = v -> get_unnamed_local_id ()
    | _ -> current)

let get_unnamed_local () = Unnamed (get_unnamed_local_id ())

let get_unnamed_local_for_tempname s =
  SN.SpecialIdents.assert_tmp_var s;
  match SMap.find_opt s !temp_local_map with
  | Some x -> x
  | None -> failwith "Unnamed local never init'ed"

let init_unnamed_local_for_tempname s =
  SN.SpecialIdents.assert_tmp_var s;
  match SMap.find_opt s !temp_local_map with
  | Some _ -> failwith "Attempted to double init"
  | None ->
    let new_local_id = get_unnamed_local_id () in
    let new_local = Unnamed new_local_id in
    temp_local_map := SMap.add s new_local !temp_local_map;
    new_local

let get_or_allocate_unnamed r =
  match !r with
  | None ->
    let l = get_unnamed_local () in
    r := Some l;
    l
  | Some l -> l

let get_label_id_local () = get_or_allocate_unnamed label_id_local

let get_retval_local () = get_or_allocate_unnamed retval_local

let reserve_retval_and_label_id_locals () =
  (* values are ignored because we care only about reserving them
     in the internal table *)
  let _ = get_label_id_local () in
  let _ = get_retval_local () in
  ()

let scope f =
  let current_next_local = !next_local in
  let current_temp_local_map = !temp_local_map in
  let result = f () in
  next_local := current_next_local;
  temp_local_map := current_temp_local_map;
  result

let reset_local base =
  next_local := base;
  label_id_local := None;
  retval_local := None;
  temp_local_map := SMap.empty
