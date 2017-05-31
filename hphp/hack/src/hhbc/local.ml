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

let next_local = ref 0

let get_unnamed_local () =
  let current = !next_local in
  next_local := current + 1;
  Unnamed current

let reset_local base =
  next_local := base
