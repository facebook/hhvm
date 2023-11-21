(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* TODO(T170647909): In preparation to upgrading to ppx_yojson_conv.v0.16.X.
         Remove the suppress warning when the upgrade is done. *)
[@@@warning "-66"]

include Set.Make (IntKey)
open Ppx_yojson_conv_lib.Yojson_conv.Primitives

let pp fmt iset =
  Format.fprintf fmt "@[<2>{";
  let elements = elements iset in
  (match elements with
  | [] -> ()
  | _ -> Format.fprintf fmt " ");
  ignore
    (List.fold_left
       (fun sep s ->
         if sep then Format.fprintf fmt ";@ ";
         Format.pp_print_int fmt s;
         true)
       false
       elements);
  (match elements with
  | [] -> ()
  | _ -> Format.fprintf fmt " ");
  Format.fprintf fmt "@,}@]"

let show iset = Format.asprintf "%a" pp iset

let to_string = show

let yojson_of_t t =
  elements t |> List.sort IntKey.compare |> yojson_of_list IntKey.yojson_of_t
