(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* An implementation of a set of types, using compare_locl_ty for a total order.
 * Typing-rule-equivalent types may get duplicated, as the equality induced
 * by compare_locl_ty does not expand Tvars and type aliases.
 *)
open Hh_prelude
open Typing_defs

module Ty_ = struct
  type t = locl_ty [@@deriving hash, show]

  let compare r1 r2 = Typing_defs.compare_locl_ty r1 r2
end

include Caml.Set.Make (Ty_)

let pp fmt t =
  Format.fprintf fmt "@[<hv 2>{";
  ignore
    (List.fold_left
       ~f:(fun sep ty ->
         if sep then Format.fprintf fmt ";@ ";
         Ty_.pp fmt ty;
         true)
       ~init:false
       (elements t));
  Format.fprintf fmt "@,}@]"

let show = Format.asprintf "%a" pp

let hash_fold_t state x =
  hash_fold_list
    Ty_.hash_fold_t
    state
    (elements x |> List.sort ~compare:Ty_.compare)

let hash = Hash.of_fold hash_fold_t
