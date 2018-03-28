(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module TySet = Typing_set

type tparam_bounds = TySet.t

type tparam_info = {
  lower_bounds : tparam_bounds;
  upper_bounds : tparam_bounds;
}

type tpenv = tparam_info SMap.t
type t = tpenv

let pp_tparam_info fmt tpi =
  Format.fprintf fmt "@[<hv 2>{ ";

  Format.fprintf fmt "@[%s =@ " "lower_bounds";
  TySet.pp fmt tpi.lower_bounds;
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "upper_bounds";
  TySet.pp fmt tpi.upper_bounds;
  Format.fprintf fmt "@]";

  Format.fprintf fmt " }@]"

let pp_tpenv fmt tpenv =
  SMap.pp pp_tparam_info fmt tpenv

let pp = pp_tpenv
