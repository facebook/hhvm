(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* In a more full-featured AT parser and AST, this would be more carefully
 * structured -- in particular, ATvariadic can only appear at the toplevel.
 * However, since we're just munging existing AT structures, we can generally
 * assume them to already be well-formed, and so can afford to play a little
 * more fast and loose here. *)

type at_ty =
  | ATstring
  | ATint
  | ATuint
  | ATfloat
  | ATbool
  | ATnumeric
  | ATcallable
  | ATanyarray
  | ATnull
  | ATvoid
  | ATobject
  | ATresource
  | ATmixed
  | ATclass of string
  | ATarray of at_ty
  | ATcomposite of at_ty list
  | ATvariadic of at_ty
