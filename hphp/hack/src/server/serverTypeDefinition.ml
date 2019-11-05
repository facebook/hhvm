(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs

let go_common (tast : Tast.program) ~(line : int) ~(column : int) :
    ServerCommandTypes.Go_to_type_definition.result =
  let env_and_ty = ServerInferType.type_at_pos tast line column in
  match env_and_ty with
  | None -> []
  | Some (env, (reason, ty)) ->
    let rec handle_type acc ty =
      match ty with
      | Tclass ((_, str), _, _) ->
        begin
          match NamingGlobal.GEnv.type_pos str with
          | None -> acc
          | Some pos -> (pos, str) :: acc
        end
      | Toption (_, ty') -> handle_type acc ty'
      | Tunion ty_lst ->
        List.fold ty_lst ~init:acc ~f:(fun a (_, y) -> handle_type a y)
      | Tfun fn_type ->
        let (_, ret_type) = fn_type.ft_ret.et_type in
        begin
          match ret_type with
          | Tprim _ ->
            (* default to function definition *)
            (Reason.to_pos reason, Tast_env.print_ty env (reason, ty)) :: acc
          | _ -> handle_type acc ret_type
        end
      | _ -> acc
    in
    List.map (handle_type [] ty) ~f:(fun (pos, s) -> (Pos.to_absolute pos, s))

(* For serverless ide *)
let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : ServerCommandTypes.Go_to_type_definition.result =
  let (tast, _errors) = Provider_utils.compute_tast_and_errors ~ctx ~entry in
  go_common tast ~line ~column

let go
    (env : ServerEnv.env)
    (position : ServerCommandTypes.file_input * int * int) :
    ServerCommandTypes.Go_to_type_definition.result =
  let (file, line, column) = position in
  let ServerEnv.{ tcopt; naming_table; _ } = env in
  let (_, tast) = ServerIdeUtils.check_file_input tcopt naming_table file in
  go_common tast ~line ~column
