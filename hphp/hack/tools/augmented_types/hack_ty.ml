(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* This is an EXTREMELY simplified version of the Hack type system basically
 * only useful for conversion from AT. *)

type hack_ty =
  | Hstring
  | Hint
  | Hfloat
  | Hbool
  | Hnum
  | Hmixed
  | Hvoid
  | Hresource
  | Hclass of string
  | Hnullable of hack_ty
  | Harray of hack_ty option
  | Hvariadic of hack_ty
