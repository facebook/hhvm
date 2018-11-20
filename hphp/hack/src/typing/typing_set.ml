(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

 (* An implementation of a set of types, using ty_compare for a total order.
  * Typing-rule-equivalent types may get duplicated, as the equality induced
  * by ty_compare does not expand Tvars and type aliases.
 *)
open Core_kernel
open Typing_defs
module Ty_ = struct
  type t = locl ty
  let compare r1 r2 = ty_compare r1 r2
end

include Caml.Set.Make(Ty_)

let pp fmt t =
  Format.fprintf fmt "@[<hv 2>{";
  ignore
    (List.fold_left
      ~f:(fun sep ty ->
        if sep then Format.fprintf fmt ";@ ";
        Pp_type.pp_ty () fmt ty;
        true)
      ~init:false
      (elements t));
  Format.fprintf fmt "@,}@]"

let show = Format.asprintf "%a" pp
