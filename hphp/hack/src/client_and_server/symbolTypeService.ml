(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Set = Stdlib.Set
open Aast
open ServerCommandTypes.Symbol_type

module Result_set = Set.Make (struct
  type t = ServerCommandTypes.Symbol_type.t

  let compare a b = Pos.compare_pos String.compare a.pos b.pos
end)

let visitor =
  object (self)
    inherit [_] Tast_visitor.reduce as super

    method zero = Result_set.empty

    method plus = Result_set.union

    method! on_expr env ((ty, pos, expr_) as expr) =
      let acc =
        match expr_ with
        | Lvar (_, id)
        | Dollardollar (_, id) ->
          Result_set.singleton
            {
              pos = Pos.to_relative_string pos;
              type_ = Tast_env.print_ty env ty;
              ident_ = Local_id.to_int id;
            }
        | _ -> self#zero
      in
      self#plus acc @@ super#on_expr env expr

    method! on_fun_param env param =
      let acc =
        let ty = param.param_annotation in
        Result_set.singleton
          {
            pos = Pos.to_relative_string param.param_pos;
            type_ = Tast_env.print_ty env ty;
            ident_ = Local_id.to_int (Local_id.make_unscoped param.param_name);
          }
      in
      self#plus acc @@ super#on_fun_param env param
  end

let generate_types ctx tasts =
  tasts
  |> List.map ~f:(visitor#go ctx)
  |> List.fold ~init:Result_set.empty ~f:Result_set.union
  |> Result_set.elements
