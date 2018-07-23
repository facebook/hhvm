(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

type collected_type = Tast_env.env * Typing_defs.phase_ty [@@deriving show]

let type_collector = object
  inherit [_] Tast_visitor.reduce
  method zero = Pos.AbsolutePosMap.empty
  method plus = Pos.AbsolutePosMap.union ~combine:(fun _ a b -> Some (a @ b))
  method! on_expr_annotation env (p, ty) =
    Pos.AbsolutePosMap.singleton
      (Pos.to_absolute p)
      [(env, Typing_defs.LoclTy ty)]

  method! on_class_id env ((_, ty), cid) =
    match cid with
    | Tast.CI ((p,_),_) ->
      Pos.AbsolutePosMap.singleton
        (Pos.to_absolute p)
        [(env, Typing_defs.LoclTy ty)]
    | _ -> Pos.AbsolutePosMap.empty

  method! on_hint (env: Tast_env.t) hint =
    let (pos, _) = hint in
    let ty = Tast_env.hint_to_ty env hint in
    Pos.AbsolutePosMap.singleton
      (Pos.to_absolute pos)
      [(env, Typing_defs.DeclTy ty)]
end

let collect_types tast =
  Errors.ignore_ (fun () -> type_collector#go tast)

let collected_types_to_json
  (collected_types: collected_type list)
  : Hh_json.json list =
  List.map collected_types ~f:(fun (env, ty) ->
    match ty with
    | Typing_defs.DeclTy ty -> Tast_env.ty_to_json env ty
    | Typing_defs.LoclTy ty -> Tast_env.ty_to_json env ty
  )

(*
  Ideally this would be just Pos.AbsolutePosMap.get, however the positions
  in the Tast are off by 1 from positions in the full fidelity parse trees.

  TODO: Fix this when the full fidelity parse tree becomes the parser for type checking.
*)
let get_from_pos_map
    (position: Pos.absolute)
    (map: collected_type list Pos.AbsolutePosMap.t) =
  let rec aux es =
    match es with
    | [] -> []
    | (pos, tys) :: tl ->
        if ((Pos.start_cnum pos) = (Pos.start_cnum position)
            && (Pos.end_cnum pos) = (1 + (Pos.end_cnum position))) then
          tys
        else
          aux tl
  in
  let elements = Pos.AbsolutePosMap.elements map in
  aux elements
