(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

type result = {
  pos: string Pos.pos;
  type_: string;
  ident_: int;
}

module Result_set = Set.Make(struct
  type t = result
  let compare a b = Pos.compare a.pos b.pos
end)

class ['self] visitor = object (self : 'self)
  inherit [_] Tast_visitor.reduce as super

  method zero = Result_set.empty
  method plus = Result_set.union

  method! on_expr env (((pos, ty), expr_) as expr) =
    let acc =
      match expr_, ty with
      | Tast.Lvar (_, id), Some ty
      | Tast.Dollardollar (_, id), Some ty ->
        Result_set.singleton {
          pos = Pos.to_relative_string pos;
          type_ = Typing_print.full_strip_ns env ty;
          ident_ = Local_id.to_int id
        }
      | _ -> self#zero
    in
    self#plus acc @@ super#on_expr env expr

  method! on_fun_param env param =
    let acc =
      let (pos, ty) = param.Tast.param_annotation in
      match ty with
      | Some ty ->
        Result_set.singleton {
          pos = Pos.to_relative_string pos;
          type_ = Typing_print.full_strip_ns env ty;
          ident_ = Local_id.to_int (Local_id.get param.Tast.param_name);
        }
      | None -> self#zero
    in
    self#plus acc @@ super#on_fun_param env param
end

let generate_types tasts =
  tasts
  |> List.map ~f:new visitor#go
  |> List.fold ~init:Result_set.empty ~f:Result_set.union
  |> Result_set.elements
