(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module S = struct
  type t = int * string [@@deriving ord]

  let equal_ref : (t -> t -> bool) ref = ref Caml.( = )

  let equal x y = !equal_ref x y

  let compare = compare
end

include S

let ctr = ref 1

let next () =
  incr ctr;
  !ctr

let to_string x = snd x

let pp_ref : (Format.formatter -> int * string -> unit) ref =
  ref (fun fmt x -> Format.pp_print_string fmt (to_string x))

let pp fmt x = !pp_ref fmt x

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

let immutable_mask = 1 lsl 62

let is_immutable i = i land immutable_mask <> 0

let make_immutable i = i lor immutable_mask

module Set = Set.Make (S)
module Map = WrappedMap.Make (S)
