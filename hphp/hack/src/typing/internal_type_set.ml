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
open Hh_prelude
open Typing_defs

module Ty_ = struct
  type t = internal_type

  let compare ty1 ty2 =
    match (ty1, ty2) with
    | (LoclType ty1, LoclType ty2) -> ty_compare ty1 ty2
    | (ConstraintType ty1, ConstraintType ty2) -> constraint_ty_compare ty1 ty2
    | (LoclType _, ConstraintType _) -> 1
    | (ConstraintType _, LoclType _) -> -1
end

include Caml.Set.Make (Ty_)

let fold_map set ~init:acc ~f =
  fold
    (fun x (acc, res) ->
      let (acc, x) = f acc x in
      let res = add x res in
      (acc, res))
    set
    (acc, empty)

let pp fmt t =
  Format.fprintf fmt "@[<hv 2>{";
  ignore
    (List.fold_left
       ~f:(fun sep _ty ->
         if sep then Format.fprintf fmt ";@ ";

         (* TODO
        Pp_type.pp_ty () fmt ty;
        *)
         true)
       ~init:false
       (elements t));
  Format.fprintf fmt "@,}@]"

let show = Format.asprintf "%a" pp
