(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module AT = At_ty
module H = Hack_ty

module HSet = Set.Make(struct
  type t = H.hack_ty
  let compare = compare
end)

exception Conv_fail of string

type mode =
  | Strict
  | Loose

type result = (H.hack_ty, string) Common.either

let rec convert_impl mode = function
  | AT.ATstring -> H.Hstring
  | AT.ATint -> H.Hint
  | AT.ATfloat -> H.Hfloat
  | AT.ATbool -> H.Hbool
  | AT.ATanyarray -> H.Harray None
  | AT.ATvoid -> H.Hvoid
  | AT.ATresource -> H.Hresource
  | AT.ATclass s -> H.Hclass s
  | AT.ATarray ty -> H.Harray (Some (convert_impl mode ty))
  | AT.ATvariadic ty -> H.Hvariadic (convert_impl mode ty)
  | AT.ATcallable -> raise (Conv_fail "Hack requires specific callable types")
  | AT.ATnull -> raise (Conv_fail "Null only makes sense in a composite!")
  | AT.ATobject -> raise (Conv_fail "Hack doesn't have a generic object type")

  | AT.ATuint when mode = Loose -> H.Hint
  | AT.ATuint -> raise (Conv_fail "Hack doesn't have a uint type")

  | AT.ATnumeric when mode = Loose -> H.Hnum
  | AT.ATnumeric -> raise (Conv_fail "Hack num disallows int-like strings")

  | AT.ATmixed when mode = Loose -> H.Hmixed
  | AT.ATmixed -> raise (Conv_fail "Hack mixed is nullable")

  | AT.ATcomposite tyl -> convert_composite mode tyl

and convert_composite mode tyl =
  let nullable = ref false in
  let tset = List.fold_left begin fun tset ty ->
    if ty = AT.ATnull then (nullable := true; tset)
    (* Manually convert mixed, regardless of mode -- see case in
     * reduce_composite. *)
    else if ty = AT.ATmixed then HSet.add H.Hmixed tset
    else HSet.add (convert_impl mode ty) tset
  end HSet.empty tyl in
  reduce_composite mode !nullable tset

and reduce_composite mode nullable tset =
  let reml = List.fold_left (fun s e -> HSet.remove e s) in
  let meml s = List.fold_left (fun b e -> b && HSet.mem e s) true in
  let num = [H.Hint; H.Hfloat] in
  let c = HSet.cardinal tset in

  if c = 0 then assert false (* Parser shouldn't generate this *)
  else if c = 1 then
    let ty = HSet.choose tset in
    if ty = H.Hmixed then
      if nullable || mode = Loose then H.Hmixed
      else raise (Conv_fail "Hack mixed is nullable")
    else if nullable then H.Hnullable ty else ty
  else if meml tset num then
    let tset = reml tset num in
    let tset = HSet.add H.Hnum tset in
    reduce_composite mode nullable tset
  (* TODO: should we convert int|string to arraykey, in loose mode at least?
   * Unclear how useful of a translation that is. *)
  else raise (Conv_fail "Unknown composite")

let convert mode ty =
  try Common.Left (convert_impl mode ty)
  with Conv_fail s -> Common.Right s
