(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Tast
open Typing_defs
open Utils

module Env = Tast_env
module Cls = Typing_classes_heap
module MakeType = Typing_make_type

let get_constant tc (seen, has_default) = function
  | Default _ -> (seen, true)
  | Case (((pos, _), Class_const ((_, CI (_, cls)), (_, const))), _) ->
    if cls <> Cls.name tc then
      (Errors.enum_switch_wrong_class pos (strip_ns (Cls.name tc)) (strip_ns cls);
       (seen, has_default))
    else
      (match SMap.get const seen with
        | None -> (SMap.add const pos seen, has_default)
        | Some old_pos ->
          Errors.enum_switch_redundant const old_pos pos;
          (seen, has_default))
  | Case (((pos, _), _), _) ->
    Errors.enum_switch_not_const pos;
    (seen, has_default)

let check_enum_exhaustiveness pos tc caselist coming_from_unresolved =
  (* If this check comes from an enum inside a Tunresolved, then
     don't punish for having an extra default case *)
  let (seen, has_default) =
    List.fold_left ~f:(get_constant tc) ~init:(SMap.empty, false) caselist in
  let unhandled =
    Cls.consts tc
    |> Sequence.map ~f:fst
    |> Sequence.filter ~f:((<>) SN.Members.mClass)
    |> Sequence.filter ~f:(fun id -> not (SMap.mem id seen))
    |> Sequence.to_list_rev
  in
  let all_cases_handled = List.is_empty unhandled in
  match (all_cases_handled, has_default, coming_from_unresolved) with
    | false, false, _ ->
      Errors.enum_switch_nonexhaustive pos unhandled (Cls.pos tc)
    | true, true, false -> Errors.enum_switch_redundant_default pos (Cls.pos tc)
    | _ -> ()

let rec check_exhaustiveness_ env pos ty caselist enum_coming_from_unresolved =
  (* Right now we only do exhaustiveness checking for enums. *)
  (* This function has a built in hack where if Tunresolved has an enum
     inside then it tells the enum exhaustiveness checker to
     not punish for extra default *)
  let env, (_, ty) = Env.expand_type env ty in
  match ty with
    | Tunresolved tyl ->
      let new_enum = enum_coming_from_unresolved ||
        (List.length tyl> 1 && List.exists tyl ~f:begin fun cur_ty ->
        let _, (_, cur_ty) = Env.expand_type env cur_ty in
        match cur_ty with
          | Tabstract (AKenum _, _) -> true
          | _ -> false
      end) in
      List.fold_left tyl ~init:env ~f:begin fun env ty ->
        check_exhaustiveness_ env pos ty caselist new_enum
      end
    | Tabstract (AKenum id, _) ->
      let dep = Typing_deps.Dep.AllMembers id in
      let decl_env = Env.get_decl_env env in
      Option.iter decl_env.Decl_env.droot
        (fun root -> Typing_deps.add_idep root dep);
      let tc = unsafe_opt @@ Env.get_enum env id in
      check_enum_exhaustiveness pos tc
        caselist enum_coming_from_unresolved;
      env
    | Terr | Tany | Tnonnull | Tarraykind _ | Tclass _ | Toption _
      | Tprim _ | Tvar _ | Tfun _ | Tabstract (_, _) | Ttuple _ | Tanon (_, _)
      | Tobject | Tshape _ | Tdynamic -> env

let check_exhaustiveness env pos ty caselist =
  ignore (check_exhaustiveness_ env pos ty caselist false)

let ensure_valid_switch_case_value_types env scrutinee_ty casel errorf =
  let is_subtype ty_sub ty_super = snd (Env.subtype env ty_sub ty_super) in
  let ty_num = (Reason.Rnone, Tprim Nast.Tnum) in
  let ty_arraykey = (Reason.Rnone, Tprim Nast.Tarraykey) in
  let ty_mixed = MakeType.mixed Reason.Rnone in
  let ty_traversable = MakeType.traversable Typing_reason.Rnone ty_mixed in
  let compatible_types ty1 ty2 =
    (is_subtype ty1 ty_num && is_subtype ty2 ty_num) ||
    (is_subtype ty1 ty_arraykey && is_subtype ty2 ty_arraykey) ||
    (is_subtype ty1 ty_traversable && is_subtype ty2 ty_traversable &&
      (is_subtype ty1 ty2 || is_subtype ty2 ty1)) ||
    (is_subtype ty1 ty2 && is_subtype ty2 ty1) in
  let ensure_valid_switch_case_value_type = function
    | Default _ -> ()
    | Case (((case_value_p, case_value_ty), _), _) ->
      if not (compatible_types case_value_ty scrutinee_ty) then
        errorf (Env.get_tcopt env) case_value_p
          (Env.print_ty env case_value_ty) (Env.print_ty env scrutinee_ty) in
  List.iter casel ensure_valid_switch_case_value_type

let handler errorf = object
  inherit Tast_visitor.handler_base

  method! at_stmt env x =
    match snd x with
    | Switch (((scrutinee_pos, scrutinee_ty), _), casel) ->
      check_exhaustiveness env scrutinee_pos scrutinee_ty casel;
      ensure_valid_switch_case_value_types env scrutinee_ty casel errorf
    | _ -> ()
end
