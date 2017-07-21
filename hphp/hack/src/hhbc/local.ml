(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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

(* use dedicated locals to store label id and return value similar to HHVM *)
let label_id_local = ref None
let retval_local = ref None

let next_local = ref 0

let rec get_unnamed_local () =
  let current = !next_local in
  next_local := current + 1;
  (* make sure that newly allocated local don't stomp on dedicated locals  *)
  match !label_id_local with
  | Some (Unnamed v) when current = v -> get_unnamed_local ()
  | _ ->
  match !retval_local with
  | Some (Unnamed v) when current = v -> get_unnamed_local ()
  | _ -> Unnamed current

let get_or_allocate_unnamed r =
  match !r with
  | None ->
    let l = get_unnamed_local () in
    r := Some l;
    l
  | Some l -> l

let get_label_id_local () = get_or_allocate_unnamed label_id_local
let has_retval_local () = Option.is_some !retval_local
let get_retval_local () = get_or_allocate_unnamed retval_local

let scope f =
  let current = !next_local in
  let result = f () in
  next_local := current;
  result

let reset_local base =
  next_local := base;
  label_id_local := None;
  retval_local := None;
