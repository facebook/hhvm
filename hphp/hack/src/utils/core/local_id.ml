(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module S = struct
  type t = int * string [@@deriving ord, hash, eq, show]

  let compare = compare
end

include S

let ctr = ref 1

let next () =
  incr ctr;
  !ctr

let to_string x = snd x

let pp fmt x =
  let (i, name) = x in
  if Int.equal i 0 then
    Format.pp_print_string fmt name
  else
    S.pp fmt x

let to_int x = fst x

let get_name x = to_string x

let is_user_denotable (lid : t) : bool =
  let lid_s = to_string lid in
  (* Allow $foo or $_bar1 but not $#foo or $0bar. *)
  let local_regexp = Str.regexp "^\\$[a-zA-z_][a-zA-Z0-9_]*$" in
  Str.string_match local_regexp lid_s 0 || String.equal lid_s "$$"

let make_scoped x = (next (), x)

let make_unscoped x = (0, x)

let make i x = (i, x)

let tmp () =
  let res = next () in
  (res, "__tmp" ^ string_of_int res)

module Set = Caml.Set.Make (S)

module Map = struct
  include WrappedMap.Make (S)

  let pp pp_data = make_pp (fun fmt id -> Format.fprintf fmt "%a" pp id) pp_data

  let show pp_data x = Format.asprintf "%a" (pp pp_data) x

  let hash_fold_t x = make_hash_fold_t S.hash_fold_t x
end
