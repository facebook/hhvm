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

type class_or_typedef_result =
  | ClassResult of Decl_provider.class_decl
  | TypedefResult of Typing_defs.typedef_type

(** {!Tast_env.env} is just an alias to {!Typing_env.env}, and the functions we
    provide for it are largely just aliases to functions that take a
    {!Typing_env.env}.

    If you find that you need to add a new alias here, please take care to
    ensure that it always works with the information available in the {!env}
    constructed by {!Tast_visitor} classes. Only a subset of the information
    available in the inference phase ({!module:Typing}) will be persisted in a
    {!Tast.program} (and thus available to {!Tast_visitor}). Most of the
    persisted information comes from {!Typing_env.save}. *)

type env = Typing_env_types.env

type t = env

exception Not_in_class

let print_ty = Typing_print.full_strip_ns

let print_decl_ty = Typing_print.full_strip_ns_decl ~verbose_fun:false

let print_error_ty = Typing_print.error

let print_ty_with_identity env phase_ty sym_occurrence sym_definition =
  match phase_ty with
  | Typing_defs.DeclTy ty ->
    let ((env, ty_err_opt), ty) =
      Typing_phase.localize_no_subst env ~ignore_errors:true ty
    in
    Option.iter ty_err_opt ~f:(Typing_error_utils.add_typing_error ~env);
    Typing_print.full_with_identity env ty sym_occurrence sym_definition
  | Typing_defs.LoclTy ty ->
    Typing_print.full_with_identity env ty sym_occurrence sym_definition

let ty_to_json env ?show_like_ty ty = Typing_print.to_json env ?show_like_ty ty

let json_to_locl_ty = Typing_print.json_to_locl_ty

let get_self_id = Typing_env.get_self_id

let get_self_ty = Typing_env.get_self_ty

let get_parent_id = Typing_env.get_parent_id

let get_self_ty_exn env =
  match get_self_ty env with
  | Some self_ty -> self_ty
  | None -> raise Not_in_class

let get_class = Typing_env.get_class

let get_class_or_typedef env x =
  match Typing_env.get_class_or_typedef env x with
  | Decl_entry.Found (Typing_env.ClassResult cd) ->
    Decl_entry.Found (ClassResult cd)
  | Decl_entry.Found (Typing_env.TypedefResult td) ->
    Decl_entry.Found (TypedefResult td)
  | Decl_entry.DoesNotExist -> Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable -> Decl_entry.NotYetAvailable

let is_in_expr_tree = Typing_env.is_in_expr_tree

let inside_expr_tree = Typing_env.inside_expr_tree

let outside_expr_tree = Typing_env.outside_expr_tree

let is_static = Typing_env.is_static

let is_strict = Typing_env.is_strict

let get_mode = Typing_env.get_mode

let get_tcopt = Typing_env.get_tcopt

let get_ctx = Typing_env.get_ctx

let expand_type = Typing_env.expand_type

let strip_dynamic = Typing_utils.strip_dynamic

let set_static = Typing_env.set_static

let set_val_kind = Typing_env.set_val_kind

let set_inside_constructor env =
  { env with Typing_env_types.inside_constructor = true }

let get_inside_constructor env = env.Typing_env_types.inside_constructor

let get_decl_env env = env.Typing_env_types.decl_env

let get_val_kind = Typing_env.get_val_kind

let get_file = Typing_env.get_file

let get_deps_mode = Typing_env.get_deps_mode

let fully_expand = Typing_expand.fully_expand

(*****************************************************************************)
(* Given some class type or unresolved union of class types, return the
 * identifiers of all classes the type may represent.
 *
 * Intended for uses like constructing call graphs and finding references, where
 * we have the statically known class type of some runtime value or class ID and
 * we would like the name of that class. *)
(*****************************************************************************)

type receiver_identifier =
  | RIclass of string
  | RIdynamic
  | RIerr
  | RIany

let get_receiver_ids env ty =
  let open Typing_defs in
  let rec aux seen acc ty =
    match get_node ty with
    | Tclass ((_, cid), _, _) -> RIclass cid :: acc
    | Toption ty
    | Tdependent (_, ty)
    | Tnewtype (_, _, ty) ->
      aux seen acc ty
    | Tunion tys
    | Tintersection tys ->
      List.fold tys ~init:acc ~f:(aux seen)
    | Tgeneric (name, targs) when not (List.mem ~equal:String.equal seen name)
      ->
      let seen = name :: seen in
      let upper_bounds = Typing_env.get_upper_bounds env name targs in
      Typing_set.fold (fun ty acc -> aux seen acc ty) upper_bounds acc
    | Tdynamic -> [RIdynamic]
    | Tany _ -> [RIany]
    | _ -> acc
  in
  List.rev (aux [] [] (Typing_expand.fully_expand env ty))

let get_class_ids env ty =
  get_receiver_ids env ty
  |> List.filter_map ~f:(function
         | RIclass cid -> Some cid
         | _ -> None)

let non_null = Typing_solver.non_null

let get_concrete_supertypes =
  Typing_utils.get_concrete_supertypes ~include_case_types:false

let is_visible = Typing_visibility.is_visible

let assert_nontrivial = Typing_equality_check.assert_nontrivial

let hint_to_ty env = Decl_hint.hint env.Typing_env_types.decl_env

let localize env ety_env dty =
  let ((env, ty_err_opt), lty) = Typing_phase.localize ~ety_env env dty in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  (env, lty)

let localize_hint_for_refinement env h =
  let ((env, _ty_err_opt), lty) =
    Typing_phase.localize_hint_for_refinement env h
  in
  (env, lty)

let localize_no_subst env ~ignore_errors dty =
  let ((env, ty_err_opt), lty) =
    Typing_phase.localize_no_subst env ~ignore_errors dty
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  (env, lty)

let get_upper_bounds = Typing_env.get_upper_bounds

let fresh_type = Typing_env.fresh_type

let is_fresh_generic_parameter = Typing_env.is_fresh_generic_parameter

let simplify_unions env ty = Typing_union.simplify_unions env ty

let union_list env r tyl = Typing_union.union_list env r tyl

let get_reified = Typing_env.get_reified

let get_enforceable = Typing_env.get_enforceable

let get_newable = Typing_env.get_newable

let assert_subtype p reason env ty_have ty_expect on_error =
  let (env, ty_err_opt) =
    Typing_ops.sub_type p reason env ty_have ty_expect on_error
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  env

let is_sub_type env ty_sub ty_super =
  Typing_subtype.is_sub_type env ty_sub ty_super

let is_dynamic_aware_sub_type env ty_sub ty_super =
  Typing_subtype.is_dynamic_aware_sub_type env ty_sub ty_super

let can_subtype env ty_sub ty_super =
  Typing_subtype.can_sub_type env ty_sub ty_super

let is_sub_type_for_union env ty_sub ty_super =
  Typing_subtype.is_sub_type_for_union env ty_sub ty_super

let referenced_typeconsts env root ids =
  let root = hint_to_ty env root in
  let ety_env = Typing_defs.empty_expand_env in
  let (tcs, ty_err_opt) =
    Typing_taccess.referenced_typeconsts env ety_env (root, ids)
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  tcs

let empty ctx = Typing_env_types.empty ctx Relative_path.default ~droot:None

let restore_saved_env env saved_env =
  let module Env = Typing_env_types in
  let ctx =
    Provider_context.map_tcopt env.Env.decl_env.Decl_env.ctx ~f:(fun _tcopt ->
        saved_env.Tast.tcopt)
  in
  let decl_env = { env.Env.decl_env with Decl_env.ctx } in
  {
    env with
    Env.decl_env;
    Env.genv = { env.Env.genv with Env.tcopt = saved_env.Tast.tcopt };
    Env.inference_env =
      Typing_inference_env.simple_merge
        env.Env.inference_env
        saved_env.Tast.inference_env;
    Env.tpenv = saved_env.Tast.tpenv;
    Env.fun_tast_info = saved_env.Tast.fun_tast_info;
    Env.checked = saved_env.Tast.checked;
  }

module EnvFromDef = Typing_env_from_def
open Tast

let check_fun_tast_info_present env = function
  | Some _ -> ()
  | None ->
    Errors.internal_error
      env.Typing_env_types.genv.Typing_env_types.callable_pos
      "fun_tast_info of a function or method was not filled in before TAST checking"

let restore_method_env env m =
  let se = m.m_annotation in
  restore_saved_env env se

let restore_fun_env env f =
  let se = f.f_annotation in
  restore_saved_env env se

let fun_env ctx fd =
  let f = fd.fd_fun in
  let ctx =
    Provider_context.map_tcopt ctx ~f:(fun _tcopt -> f.f_annotation.tcopt)
  in
  let env = EnvFromDef.fun_env ~origin:Decl_counters.Tast ctx fd in
  restore_fun_env env f

let class_env ctx c =
  let ctx =
    Provider_context.map_tcopt ctx ~f:(fun _tcopt -> c.c_annotation.tcopt)
  in
  let env = EnvFromDef.class_env ~origin:Decl_counters.Tast ctx c in
  restore_saved_env env c.c_annotation

let typedef_env ctx t =
  let ctx =
    Provider_context.map_tcopt ctx ~f:(fun _tcopt -> t.t_annotation.tcopt)
  in
  let env = EnvFromDef.typedef_env ~origin:Decl_counters.Tast ctx t in
  restore_saved_env env t.t_annotation

let gconst_env ctx cst =
  let ctx =
    Provider_context.map_tcopt ctx ~f:(fun _tcopt -> cst.cst_annotation.tcopt)
  in
  let env = EnvFromDef.gconst_env ~origin:Decl_counters.Tast ctx cst in
  restore_saved_env env cst.cst_annotation

let def_env ctx d =
  match d with
  | Fun x -> fun_env ctx x
  | Class x -> class_env ctx x
  | Typedef x -> typedef_env ctx x
  | Constant x -> gconst_env ctx x
  (* TODO T44306013 *)
  (* The following nodes are included in the TAST, but are not typechecked.
   * However, we need to return an env here so for now create an empty env using
   * the default typechecker options.
   *)
  | Stmt _
  | Namespace _
  | NamespaceUse _
  | SetNamespaceEnv _
  | SetModule _
  | FileAttributes _
  | Module _ ->
    empty ctx

let typing_env_as_tast_env env = env

let tast_env_as_typing_env env = env

let is_xhp_child = Typing_xhp.is_xhp_child

let get_enum = Typing_env.get_enum

let is_typedef = Typing_env.is_typedef

let get_typedef = Typing_env.get_typedef

let is_typedef_visible = Typing_env.is_typedef_visible

let is_enum = Typing_env.is_enum

let get_fun = Typing_env.get_fun

let set_allow_wildcards env =
  { env with Typing_env_types.allow_wildcards = true }

let get_allow_wildcards env = env.Typing_env_types.allow_wildcards

let is_enum_class env c = Typing_env.is_enum_class env c

let extract_from_fun_tast_info env extractor default_value =
  let fun_tast_info = env.Typing_env_types.fun_tast_info in
  check_fun_tast_info_present env fun_tast_info;
  match fun_tast_info with
  | Some fun_tast_info -> extractor fun_tast_info
  | None ->
    (* In this case, check_fun_tast_info_present reported an error already *)
    default_value

let fun_has_implicit_return (env : t) =
  extract_from_fun_tast_info env (fun info -> info.has_implicit_return) false

let fun_has_readonly (env : t) =
  extract_from_fun_tast_info env (fun info -> info.has_readonly) false

let get_const env cls name = Typing_env.get_const env cls name

let consts env cls = Typing_env.consts env cls

let fill_in_pos_filename_if_in_current_decl =
  Typing_env.fill_in_pos_filename_if_in_current_decl

let is_hhi = Typing_env.is_hhi

let get_check_status env = env.Typing_env_types.checked

let get_current_decl_and_file = Typing_env.get_current_decl_and_file
