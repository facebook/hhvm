(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open Hh_json

class ['self] type_collector = object (_ : 'self)
  inherit [_] Tast_visitor.reduce
  method zero = Pos.AbsolutePosMap.empty
  method plus = Pos.AbsolutePosMap.union ~combine:(fun _ a b -> Some (a @ b))
  method! on_expr_annotation env (p,ty) =
    match ty with
    | Some ty -> Pos.AbsolutePosMap.singleton (Pos.to_absolute p) [Typing_print.to_json env ty]
    | None -> Pos.AbsolutePosMap.empty
  method! on_class_id env (ty,cid) =
    match ty, cid with
    | Some ty, Tast.CI ((p,_),_) ->
      Pos.AbsolutePosMap.singleton (Pos.to_absolute p) [Typing_print.to_json env ty]
    | _ -> Pos.AbsolutePosMap.empty
end

let collect_types = new type_collector#go

let types_to_json tast =
  let types_list =
    tast
    |> collect_types
    |> Pos.AbsolutePosMap.elements
    |> List.map ~f:(fun (pos, tys) ->
      JSON_Array [Pos.json pos; JSON_Array tys])
  in
  JSON_Array types_list
