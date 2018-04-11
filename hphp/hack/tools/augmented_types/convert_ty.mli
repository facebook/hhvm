(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type mode =
  | Strict
  | Loose

type result = (Hack_ty.hack_ty, string) Common.either
val convert : mode -> At_ty.at_ty -> result
