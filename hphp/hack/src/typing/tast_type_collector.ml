(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type collected_type = Tast_env.env * Typing_defs.phase_ty [@@deriving show]

let type_collector =
  object
    inherit [_] Tast_visitor.reduce

    method zero = Pos.AbsolutePosMap.empty

    method plus = Pos.AbsolutePosMap.union ~combine:(fun _ a b -> Some (a @ b))

    method! on_'ex env (p, ty) =
      Pos.AbsolutePosMap.singleton
        (Pos.to_absolute p)
        [(env, Typing_defs.LoclTy ty)]

    method! on_class_id env ((_, ty), cid) =
      match cid with
      | Aast.CI (p, _) ->
        Pos.AbsolutePosMap.singleton
          (Pos.to_absolute p)
          [(env, Typing_defs.LoclTy ty)]
      | _ -> Pos.AbsolutePosMap.empty

    method! on_hint (env : Tast_env.t) hint =
      let (pos, _) = hint in
      let ty = Tast_env.hint_to_ty env hint in
      Pos.AbsolutePosMap.singleton
        (Pos.to_absolute pos)
        [(env, Typing_defs.DeclTy ty)]
  end

let collect_types tast = Errors.ignore_ (fun () -> type_collector#go tast)

(*
  Ideally this would be just Pos.AbsolutePosMap.find_opt, however the positions
  in the Tast are off by 1 from positions in the full fidelity parse trees.

  TODO: Fix this when the full fidelity parse tree becomes the parser for type checking.
*)
let get_from_pos_map
    (position : Pos.absolute) (map : collected_type list Pos.AbsolutePosMap.t) =
  let position = Pos.advance_one position in
  Pos.AbsolutePosMap.find_opt position map
