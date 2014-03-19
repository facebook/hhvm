(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Typing_defs

(*****************************************************************************)
(* Section defining the colors we are going to use *)
(*****************************************************************************)

type mycolor =
  | Unchecked_code (* Unchecked code *)
  | Checked_code   (* Checked code *)
  | Keyword        (* Keyword *)
  | Fun            (* Function name *)
  | Default        (* All the rest *)

let make_color (pos, ty) =
  pos, match ty with
  | _, Typing_defs.Tany -> Unchecked_code
  | _ -> Checked_code

(*****************************************************************************)
(* Module comparing positions and types (to sort them later) *)
(*****************************************************************************)

module Compare = struct

  (* If two positions overlap, they are considered equal *)
  let pos pos1 pos2 =
    let char_start1, char_end1 = Pos.info_raw pos1 in
    let char_start2, char_end2 = Pos.info_raw pos2 in
    if char_end1 <= char_start2
    then -1
    else if char_end2 <= char_start1
    then 1
    else 0

  (* All the types that are different from Tany are considered equal *)
  let type_ ty1 ty2 =
    match ty1, ty2 with
    | (_, Tany), (_, Tany) -> 0
    | (_, Tany), _ -> -1
    | _, (_, Tany) -> 1
    | _, _ -> 0

  let pos_type (p1, ty1) (p2, ty2) =
    let c = pos p1 p2 in
    if c = 0 then type_ ty1 ty2 else c
end

(*****************************************************************************)
(* Logic deciding which color we keep when positions overlap *)
(*****************************************************************************)

let rec filter = function
  | [] | [_] as l -> l
  (* One of them is unchecked code, an they overlap. *)
  | (pos1, (_, Tany as ty1) as elt1) :: (pos2, ty2 as elt2) :: rl
  | (pos1, ty1 as elt1) :: (pos2, (_, Tany as ty2) as elt2) :: rl
    when Compare.pos pos1 pos2 = 0 ->
      let cmp_ty = Compare.type_ ty1 ty2 in
      (* Unchecked wins *)
      if cmp_ty < 0
      then filter (elt1 :: rl)
      (* Unchecked wins *)
      else if cmp_ty > 0
      then filter (elt2 :: rl)
      (* The smallest one wins *)
      else if Pos.length pos1 <= Pos.length pos2
      then filter (elt1 :: rl)
      else filter (elt2 :: rl)
  | (pos1, _ as elt1) :: (pos2, _ as elt2) :: rl
    when Compare.pos pos1 pos2 = 0 ->
      (* The largest one wins *)
      if Pos.length pos1 >= Pos.length pos2
      then filter (elt1 :: rl)
      else filter (elt2 :: rl)
  | elt :: (_ :: _ as rl) -> elt :: filter rl

(*****************************************************************************)
(* Walks the content of a string and adds colors at the given positions. *)
(*****************************************************************************)

let walk content pos_color_list =
  let result = ref [] in
  let i = ref 0 in
  let add color j =
    if j <= !i then () else
    let size = (j - !i + 1) in
    result := (color, String.sub content !i size) :: !result;
    i := !i + size
  in
  List.iter begin fun (pos, color) ->
    let char_start, char_end = Pos.info_raw pos in
    add Default (char_start - 1);
    add color (char_end - 1);
  end pos_color_list;
  add Default (String.length content - 1);
  List.rev !result

(*****************************************************************************)
(* The entry point. *)
(*****************************************************************************)

let go str (pos_type_l: (Pos.t * Typing_defs.ty) list) =
  let pos_type_l = List.sort Compare.pos_type pos_type_l in
  let pos_type_l = filter pos_type_l in
  let pos_color_l = List.map make_color pos_type_l in
  walk str pos_color_l
