(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs

let go_common
    (ctx : Provider_context.t)
    (tast : Tast.program Tast_with_dynamic.t)
    ~(line : int)
    ~(column : int) : ServerCommandTypes.Go_to_type_definition.result =
  let env_and_ty =
    ServerInferType.human_friendly_type_at_pos
      ~under_dynamic:false
      ctx
      tast
      line
      column
  in
  match env_and_ty with
  | None -> []
  | Some (env, ty) ->
    let rec handle_type acc ty =
      match get_node ty with
      | Tclass ((_, str), _, _) -> begin
        match Naming_global.GEnv.type_pos ctx str with
        | None -> acc
        | Some pos -> (pos, str) :: acc
      end
      | Toption ty' -> handle_type acc ty'
      | Tunion ty_lst ->
        List.fold ty_lst ~init:acc ~f:(fun a y -> handle_type a y)
      | Tfun fn_type ->
        let ret_type = fn_type.ft_ret in
        begin
          match get_node ret_type with
          | Tprim _ ->
            (* default to function definition *)
            ( Naming_provider.resolve_position ctx @@ get_pos ty,
              Tast_env.print_ty env ty )
            :: acc
          | _ -> handle_type acc ret_type
        end
      | _ -> acc
    in
    List.map (handle_type [] ty) ~f:(fun (pos, s) -> (Pos.to_absolute pos, s))

(* For serverless ide *)
let go_quarantined
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : ServerCommandTypes.Go_to_type_definition.result =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  go_common ctx tast ~line ~column
