(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]
open Core_kernel
[@@@warning "+33"]
module TySet = Typing_set

type tparam_bounds = TySet.t

type tparam_info = {
  lower_bounds : tparam_bounds;
  upper_bounds : tparam_bounds;
  reified      : Nast.reify_kind;
  enforceable  : bool;
  newable      : bool;
}

type tpenv = tparam_info SMap.t
type t = tpenv

let empty: t = SMap.empty

let pp_tparam_info fmt tpi =
  Format.fprintf fmt "@[<hv 2>{ ";

  Format.fprintf fmt "@[%s =@ " "lower_bounds";
  TySet.pp fmt tpi.lower_bounds;
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "upper_bounds";
  TySet.pp fmt tpi.upper_bounds;
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "reified";
  Nast.pp_reify_kind fmt tpi.reified;
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "enforceable";
  Format.pp_print_bool fmt tpi.enforceable;
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "newable";
  Format.pp_print_bool fmt tpi.newable;
  Format.fprintf fmt "@]";

  Format.fprintf fmt " }@]"

let pp_tpenv fmt tpenv =
  SMap.pp pp_tparam_info fmt tpenv

let pp = pp_tpenv
