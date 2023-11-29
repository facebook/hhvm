(*
 * Copyright (c) 2018, Facebook, Inc.
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
module SN = Naming_special_names

let ft_redundant_tparams env tparams ty =
  let tracked =
    List.fold_left
      tparams
      ~f:(fun tracked t -> SSet.add (snd t.tp_name) tracked)
      ~init:SSet.empty
  in
  let (positive, negative) =
    Typing_variance.get_positive_negative_generics
      ~is_mutable:false
      ~tracked
      (Tast_env.tast_env_as_typing_env env)
      (SMap.empty, SMap.empty)
      ty
  in
  List.iter tparams ~f:(fun t ->
      let (pos, name) = t.tp_name in
      let pos = Naming_provider.resolve_position (Tast_env.get_ctx env) pos in
      (* It's only redundant if it's erased and inferred *)
      if
        equal_reify_kind t.tp_reified Erased
        && (not
              (Attributes.mem
                 SN.UserAttributes.uaNonDisjoint
                 t.tp_user_attributes))
        && not
             (Attributes.mem SN.UserAttributes.uaExplicit t.tp_user_attributes)
      then
        let super_bounds =
          List.filter
            ~f:(fun (ck, _) ->
              Ast_defs.(equal_constraint_kind ck Constraint_super))
            t.tp_constraints
        in
        let as_bounds =
          List.filter
            ~f:(fun (ck, _) ->
              Ast_defs.(equal_constraint_kind ck Constraint_as))
            t.tp_constraints
        in
        match (SMap.find_opt name positive, SMap.find_opt name negative) with
        | (Some _, Some _) -> ()
        | (Some _positions, None) ->
          let bounds_message =
            if List.is_empty as_bounds then
              ""
            else
              " with useless `as` bound"
          in
          (* If there is more than one `super` bound, we can't replace,
           * because we don't support explicit union types
           *)
          begin
            match super_bounds with
            | [] ->
              Lints_errors.redundant_covariant pos name bounds_message "nothing"
            | [(_, t)] ->
              Lints_errors.redundant_covariant
                pos
                name
                bounds_message
                (Tast_env.print_decl_ty env t)
            | _ -> ()
          end
        | (None, Some _positions) ->
          let bounds_message =
            if List.is_empty super_bounds then
              ""
            else
              " with useless `super` bound"
          in
          (* If there is more than one `as` bound, we can't replace,
           * because we don't support explicit intersection types
           *)
          begin
            match as_bounds with
            | [] ->
              Lints_errors.redundant_contravariant
                pos
                name
                bounds_message
                "mixed"
            | [(_, t)] ->
              Lints_errors.redundant_contravariant
                pos
                name
                bounds_message
                (Tast_env.print_decl_ty env t)
            | _ -> ()
          end
        | (None, None) -> Lints_errors.redundant_generic pos name)

let check_redundant_generics_class_method env (_method_name, method_) =
  match method_.ce_type with
  | (lazy (ty as ft)) -> begin
    match get_node ty with
    | Tfun { ft_tparams; _ } -> ft_redundant_tparams env ft_tparams ft
    | _ -> assert false
  end

let check_redundant_generics_fun env ft =
  ft_redundant_tparams env ft.ft_tparams (mk (Reason.Rnone, Tfun ft))

let check_redundant_generics_class env class_name class_type =
  Cls.methods class_type
  |> ListLabels.filter ~f:(fun (_, meth) ->
         String.equal meth.ce_origin class_name)
  |> List.iter ~f:(check_redundant_generics_class_method env);
  Cls.smethods class_type
  |> List.filter ~f:(fun (_, meth) -> String.equal meth.ce_origin class_name)
  |> List.iter ~f:(check_redundant_generics_class_method env)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def env fd =
      match Decl_provider.get_fun (Tast_env.get_ctx env) (snd fd.fd_name) with
      | Decl_entry.Found { fe_type; _ } -> begin
        match get_node fe_type with
        | Tfun ft -> check_redundant_generics_fun env ft
        | _ -> ()
      end
      | _ -> ()

    method! at_class_ env c =
      let cid = snd c.c_name in
      match Decl_provider.get_class (Tast_env.get_ctx env) cid with
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        ()
      | Decl_entry.Found cls ->
        check_redundant_generics_class env (snd c.c_name) cls
  end
