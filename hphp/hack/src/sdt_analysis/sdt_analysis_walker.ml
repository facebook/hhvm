(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Sdt_analysis_types
module A = Aast
module T = Typing_defs
module SN = Naming_special_names
module MakeType = Typing_make_type
module Reason = Typing_reason
module TPEnv = Type_parameter_env

let mixed = MakeType.mixed Reason.Rnone

let supportdyn_of_mixed = MakeType.supportdyn Reason.Rnone mixed

let remove_supportdyn_of_mixed_upper_bound_from_tparams env =
  let typing_env = Tast_env.tast_env_as_typing_env env in
  let tpenv = Typing_env.get_tpenv typing_env in
  let remove_upper_bound tpname tpinfo tpenv =
    let open Typing_kinding_defs in
    let upper_bounds = TySet.remove supportdyn_of_mixed tpinfo.upper_bounds in
    let tpinfo = { tpinfo with upper_bounds } in
    TPEnv.add ~def_pos:Pos_or_decl.none tpname tpinfo tpenv
  in
  let tpenv = TPEnv.fold remove_upper_bound tpenv TPEnv.empty in
  let typing_env = Typing_env_types.{ typing_env with tpenv } in
  Tast_env.typing_env_as_tast_env typing_env

let remove_supportdyn_from_ty ty =
  match T.get_node ty with
  | T.Tnewtype (c, [ty], _) when String.equal c SN.Classes.cSupportDyn -> ty
  | _ -> ty

let tast_handler writer =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env (_, e_pos, e_) =
      let decorated_constraint ~origin =
        DecoratedConstraint.
          { origin; hack_pos = e_pos; constraint_ = Constraint.NeedsSDT }
      in
      let id_constraint_pairs =
        match e_ with
        | A.Call ((base_ty, _, base_exp), _tel, el, _unpacked_el) ->
          let doesnt_subtype (fp, (_, (arg_ty, _, _))) =
            not @@ Tast_env.is_sub_type env arg_ty fp.T.fp_type.T.et_type
          in
          let constraints_of_class_sid sid =
            let id = (H.Class, sid) in
            let ty = remove_supportdyn_from_ty base_ty in
            match T.get_node ty with
            | T.Tfun ft ->
              let param_arg_pairs =
                let open List.Or_unequal_lengths in
                match List.zip ft.T.ft_params el with
                | Ok pairs -> pairs
                | Unequal_lengths -> []
              in
              if List.exists ~f:doesnt_subtype param_arg_pairs then
                let constraint_ = decorated_constraint ~origin:__LINE__ in
                [(id, constraint_)]
              else
                []
            | _ -> []
          in
          begin
            match base_exp with
            | A.Id (_, sid) -> constraints_of_class_sid sid
            | A.Obj_get ((receiver_ty, _, _), _, _, _) -> begin
              match T.get_node receiver_ty with
              | T.Tclass ((_, sid), _, _) -> constraints_of_class_sid sid
              | _ -> []
            end
            | A.Class_const ((_, _, A.CI (_, sid)), _) ->
              constraints_of_class_sid sid
            | _ -> []
          end
        | _ -> []
      in
      id_constraint_pairs
      |> List.iter ~f:(fun (id, constraint_) ->
             H.Write.add_intra writer id constraint_)

    method! at_fun_def env A.{ fd_name = (_, sid); fd_fun; _ } : unit =
      let id = (H.Function, sid) in
      H.Write.add_id writer id;
      let env = remove_supportdyn_of_mixed_upper_bound_from_tparams env in
      let decorated_constraint ~origin =
        let hack_pos = fd_fun.A.f_span in
        DecoratedConstraint.
          { origin; hack_pos; constraint_ = Constraint.NeedsSDT }
      in
      let from_dynamically_callable =
        let is_dynamically_callable =
          List.exists ~f:(fun attr ->
              String.equal
                (snd attr.A.ua_name)
                SN.UserAttributes.uaDynamicallyCallable)
        in
        if is_dynamically_callable fd_fun.A.f_user_attributes then
          let constraint_ = decorated_constraint ~origin:__LINE__ in
          [(id, constraint_)]
        else
          []
      in
      let from_signature_check =
        let is_supportdyn_of_mixed ty =
          Tast_env.is_sub_type env ty supportdyn_of_mixed
        in
        let fails_formation =
          List.exists fd_fun.A.f_params ~f:(fun param ->
              not @@ is_supportdyn_of_mixed @@ fst param.A.param_type_hint)
          || (not @@ is_supportdyn_of_mixed @@ fst fd_fun.A.f_ret)
        in
        if fails_formation then
          let constraint_ = decorated_constraint ~origin:__LINE__ in
          [(id, constraint_)]
        else
          []
      in

      from_dynamically_callable @ from_signature_check
      |> List.iter ~f:(fun (id, constraint_) ->
             H.Write.add_intra writer id constraint_)

    method! at_class_ _ A.{ c_name = (_, sid); c_extends; _ } =
      let id = (H.Class, sid) in
      H.Write.add_id writer id;

      let at_extends (_, hint_) =
        match hint_ with
        | A.Happly ((_, parent_sid), _hints) ->
          let parent_id = (H.Class, parent_sid) in
          H.Write.add_inter writer id (H.ClassExtends parent_id)
        | _ -> ()
      in
      c_extends |> List.iter ~f:at_extends
  end
