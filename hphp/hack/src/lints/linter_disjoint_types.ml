(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Common
open Aast
open Typing_defs
module Cls = Decl_provider.Class
module Env = Tast_env
module SN = Naming_special_names

let check_non_disjoint env p name ty1 ty2 =
  let tenv = Tast_env.tast_env_as_typing_env env in
  if
    Typing_utils.(
      (not (is_nothing tenv ty1))
      && (not (is_nothing tenv ty2))
      && is_type_disjoint tenv ty1 ty2)
  then
    Lints_errors.invalid_disjointness_check
      p
      (Utils.strip_ns name)
      (Env.print_ty env ty1)
      (Env.print_ty env ty2)

let rec check_non_disjoint_tyl env p name tyl =
  match tyl with
  | [] -> ()
  | ty :: tyl ->
    List.iter tyl ~f:(check_non_disjoint env p name ty);
    check_non_disjoint_tyl env p name tyl

let has_non_disjoint_attr tp =
  Attributes.mem SN.UserAttributes.uaNonDisjoint tp.tp_user_attributes

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, p, Call ((_, _, Id (_, name)), (_ :: _ as tal), _, _)) -> begin
        match Decl_provider.get_fun (Tast_env.get_ctx env) name with
        | Some { fe_type; _ } -> begin
          match get_node fe_type with
          | Tfun { ft_tparams = tpl; _ } ->
            if List.exists tpl ~f:has_non_disjoint_attr then
              let (pairs, _) = List.zip_with_remainder tpl tal in
              let tyl =
                List.filter_map pairs ~f:(fun (tp, (ty, _)) ->
                    if has_non_disjoint_attr tp then
                      Some ty
                    else
                      None)
              in
              check_non_disjoint_tyl env p name tyl
          | _ -> ()
        end
        | _ -> ()
      end
      | _ -> ()
  end
