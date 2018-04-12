(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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
    match cid with
    | Tast.CI ((p,_),_) ->
      Pos.AbsolutePosMap.singleton (Pos.to_absolute p) [Typing_print.to_json env ty]
    | _ -> Pos.AbsolutePosMap.empty

  method! on_hint (env: Typing_env.env) hint =
    let (pos, _) = hint in
    let ty = Decl_hint.hint env.Typing_env.decl_env hint in
    Pos.AbsolutePosMap.singleton (Pos.to_absolute pos) [Typing_print.to_json env ty]
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

(*
  Ideally this would be just Pos.AbsolutePosMap.get, however the positions
  in the Tast are off by 1 from positions in the full fidelity parse trees.

  TODO: Fix this when the full fidelity parse tree becomes the parser for type checking.
*)
let get_from_pos_map (position: Pos.absolute) (map: Hh_json.json list Pos.AbsolutePosMap.t) =
  let rec aux es =
    match es with
    | [] -> []
    | (pos, tys) :: tl ->
        if ((Pos.start_cnum pos) == (Pos.start_cnum position)
            && (Pos.end_cnum pos) == (1 + (Pos.end_cnum position))) then
          tys
        else
          aux tl
  in
  let elements = Pos.AbsolutePosMap.elements map in
  aux elements
