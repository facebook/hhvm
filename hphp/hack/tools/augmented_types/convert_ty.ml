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

exception Conv_fail of string

type mode =
  | Strict
  | Loose

type result = (Hack_ty.hack_ty, string) Common.either

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

  | AT.ATnum when mode = Loose -> H.Hnum
  | AT.ATnum -> raise (Conv_fail "Hack num disallows int-like strings")

  | AT.ATmixed when mode = Loose -> H.Hmixed
  | AT.ATmixed -> raise (Conv_fail "Hack mixed is nullable")

  | AT.ATcomposite _ -> raise (Conv_fail "UNIMPLEMENTED")

let convert mode ty =
  try Common.Left (convert_impl mode ty)
  with Conv_fail s -> Common.Right s
