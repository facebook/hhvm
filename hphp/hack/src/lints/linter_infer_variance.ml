(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module Cls = Decl_provider.Class

let infer_variance_class env class_name class_type =
  let tparams = Cls.tparams class_type in
  let tracked =
    List.fold_left
      tparams
      ~f:(fun tracked t -> SSet.add (snd t.tp_name) tracked)
      ~init:SSet.empty
  in
  if
    List.exists tparams ~f:(fun t ->
        match t.tp_variance with
        | Ast_defs.Invariant -> true
        | _ -> false)
  then
    (* All non-private methods defined in this class except for constructors *)
    let methods =
      Cls.methods class_type @ Cls.smethods class_type
      |> ListLabels.filter ~f:(fun (name, member) ->
             String.equal member.ce_origin class_name
             && (not
                   (String.equal name Naming_special_names.Members.__construct))
             &&
             match member.ce_visibility with
             | Vprivate _ -> false
             | _ -> true)
    in
    (* All non-private properties defined in this class *)
    let props =
      Cls.props class_type @ Cls.sprops class_type
      |> ListLabels.filter ~f:(fun (_name, member) ->
             String.equal member.ce_origin class_name
             &&
             match member.ce_visibility with
             | Vprivate _ -> false
             | _ -> true)
    in
    (* All ancestors. This is overkill: we should only need to check direct ancestors *)
    let res =
      List.fold_right
        (Cls.all_ancestors class_type)
        ~init:(SMap.empty, SMap.empty)
        ~f:(fun (_, ty) acc ->
          Typing_variance.get_positive_negative_generics
            ~tracked
            ~is_mutable:false
            (Tast_env.tast_env_as_typing_env env)
            acc
            ty)
    in
    let res =
      methods
      |> List.fold ~init:res ~f:(fun acc (_, ce) ->
             Typing_variance.get_positive_negative_generics
               ~tracked
               ~is_mutable:false
               (Tast_env.tast_env_as_typing_env env)
               acc
               (force ce.ce_type))
    in
    let res =
      props
      |> List.fold ~init:res ~f:(fun acc (_, ce) ->
             Typing_variance.get_positive_negative_generics
               ~tracked
               ~is_mutable:true
               (Tast_env.tast_env_as_typing_env env)
               acc
               (force ce.ce_type))
    in
    let (positive, negative) = res in
    List.iter tparams ~f:(fun t ->
        match t.tp_reified with
        (* Reified type parameters can't be marked with a variance *)
        | SoftReified
        | Reified ->
          ()
        | Erased ->
          let (pos, name) = t.tp_name in
          let pos =
            Naming_provider.resolve_position (Tast_env.get_ctx env) pos
          in
          (match
             ( t.tp_variance,
               SMap.find_opt name positive,
               SMap.find_opt name negative )
           with
          | (Ast_defs.Invariant, None, None) ->
            Lints_errors.inferred_variance
              pos
              "covariant or contravariant"
              "`+` or `-`"
          | (Ast_defs.Invariant, Some _, None) ->
            Lints_errors.inferred_variance pos "covariant" "`+`"
          | (Ast_defs.Invariant, None, Some _) ->
            Lints_errors.inferred_variance pos "contravariant" "`-`"
          | _ -> ()))

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ env c =
      let cid = snd c.c_name in
      match Decl_provider.get_class (Tast_env.get_ctx env) cid with
      | None -> ()
      | Some cls -> infer_variance_class env (snd c.c_name) cls
  end
