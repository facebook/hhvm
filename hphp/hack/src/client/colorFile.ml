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
open Utils

(*****************************************************************************)
(* Section defining the colors we are going to use *)
(*****************************************************************************)

type mycolor =
  | Unchecked_code (* Unchecked code *)
  | Checked_code   (* Checked code *)
  | Partially_checked_code (* Partially checked code *)
  | Keyword        (* Keyword *)
  | Fun            (* Function name *)
  | Default        (* All the rest *)

module HasTany : sig
  val check: ty -> bool
end = struct
  let visitor =
    object(this)
      inherit [bool] TypeVisitor.type_visitor
      method! on_tany _ = true
      method! on_tarray acc _ ty1_opt ty2_opt =
        (* Check for array without its value type parameter specified *)
        (match ty2_opt with
        | None -> true
        | Some ty -> this#on_type acc ty) ||
        (opt_fold_left this#on_type acc ty1_opt)
    end
  let check ty = visitor#on_type false ty
end

let make_color (pos, ty) =
  pos, match ty with
  | _, Typing_defs.Tany -> Unchecked_code
  | _ when HasTany.check ty -> Partially_checked_code
  | _ -> Checked_code

(*****************************************************************************)
(* Module comparing positions (to sort them later)
 * This assumes that all positions are either nested or disjoint,
 * i.e. there are no partial overlaps... which should be the case for all
 * well-formed ASTs *)
(*****************************************************************************)

module Compare = struct

  let pos pos1 pos2 =
    let char_start1, char_end1 = Pos.info_raw pos1 in
    let char_start2, char_end2 = Pos.info_raw pos2 in
    if char_end1 < char_start2
    then -1
    else if char_end2 < char_start1
    then 1
    (* If one position is nested inside another, put the outer position first *)
    else if char_start1 < char_start2
    then -1
    else compare char_end2 char_end1

end

(*****************************************************************************)
(* Flatten nested positions (intervals).
 * E.g. if A, B, C are colors, we convert [AA[B]A[C]A] to [AA][B][A][C][A]. *)
(*****************************************************************************)

let rec flatten_ acc stack = function
  | [] | [_] as l when Stack.is_empty stack -> l @ acc
  | [] | [_] as l ->
      let elem = Stack.pop stack in
      flatten_ acc stack (elem :: l)
  | (pos1, x as elt1) :: ((pos2, _) :: _ as rl) ->
      let _, char_end1 = Pos.info_raw pos1 in
      let char_start2, _ = Pos.info_raw pos2 in
      if char_end1 <= char_start2
      then (* Intervals are disjoint *)
        if Stack.is_empty stack
        then
          flatten_ (elt1 :: acc) stack rl
        else
          let elem = Stack.pop stack in
          flatten_ (elt1 :: acc) stack (elem :: rl)
      else begin (* interval 2 is nested within interval 1 *)
        (* avoid creating zero-length intervals *)
        if pos1.Pos.pos_end <> pos2.Pos.pos_end
        then
          (let pos1_rest = { pos1 with Pos.pos_start = pos2.Pos.pos_end } in
          Stack.push (pos1_rest, x) stack);
        let pos1_head = { pos1 with Pos.pos_end = pos2.Pos.pos_start } in
        flatten_ ((pos1_head, x) :: acc) stack rl
      end

let flatten xs =
  flatten_ [] (Stack.create ()) xs |> List.rev

(*****************************************************************************)
(* Walks the content of a string and adds colors at the given positions. *)
(*****************************************************************************)

let walk content pos_color_list =
  let result = ref [] in
  let i = ref 0 in
  let add color j =
    if j <= !i then () else
    let size = j - !i in
    result := (color, String.sub content !i size) :: !result;
    i := !i + size
  in
  List.iter begin fun (pos, color) ->
    let char_start, char_end = Pos.info_raw pos in
    add Default char_start;
    add color char_end;
  end pos_color_list;
  add Default (String.length content);
  List.rev !result

(*****************************************************************************)
(* The entry point. *)
(*****************************************************************************)

let go str (pos_type_l: (Pos.t * Typing_defs.ty) list) =
  let cmp x y = Compare.pos (fst x) (fst y) in
  let pos_type_l = List.sort cmp pos_type_l in
  let pos_type_l = flatten pos_type_l in
  let pos_color_l = List.map make_color pos_type_l in
  walk str pos_color_l
