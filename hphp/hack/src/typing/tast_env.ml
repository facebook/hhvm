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

include Typing_env

type class_or_typedef_result =
  | ClassResult of Decl_provider.class_decl
  | TypedefResult of Typing_defs.typedef_type

exception Not_in_class

let get_decl_env env = env.Typing_env_types.decl_env

let print_ty = Typing_print.full_strip_ns ~hide_internals:true

let print_decl_ty = Typing_print.full_strip_ns_decl ~verbose_fun:false

let print_error_ty = Typing_print.error

let print_hint env hint =
  print_decl_ty env @@ Decl_hint.hint (get_decl_env env) hint

let print_ty_with_identity env ty sym_occurrence sym_definition =
  Typing_print.full_with_identity
    ~hide_internals:true
    env
    ty
    sym_occurrence
    sym_definition

let print_decl_ty_with_identity env ty sym_occurrence sym_definition =
  Typing_print.full_decl_with_identity
    ~verbose_fun:false
    ~omit_likes:true
    env
    ty
    sym_occurrence
    sym_definition

let ty_to_json env ?show_like_ty ty =
  Typing_json.from_locl_ty env ?show_like_ty ty

let json_to_locl_ty = Typing_json.to_locl_ty

let get_self_ty_exn env =
  match get_self_ty env with
  | Some self_ty -> self_ty
  | None -> raise Not_in_class

let get_class_or_typedef env x : class_or_typedef_result Decl_entry.t =
  match Typing_env.get_class_or_typedef env x with
  | Found (ClassResult cd) -> Found (ClassResult cd)
  | Found (TypedefResult td) -> Found (TypedefResult td)
  | DoesNotExist -> DoesNotExist
  | NotYetAvailable -> NotYetAvailable

let outside_expr_tree = Typing_env.outside_expr_tree ~macro_variables:None

let strip_dynamic env ty = snd (Typing_dynamic_utils.strip_dynamic env ty)

type is_disjoint_result =
  | NonDisjoint
  | Disjoint
  | DisjointIgnoringDynamic of Typing_defs.locl_ty * Typing_defs.locl_ty

let is_disjoint ~is_dynamic_call env ty1 ty2 =
  if
    (not is_dynamic_call)
    && Typing_utils.(
         (not (is_nothing env ty1))
         && (not (is_nothing env ty2))
         && is_type_disjoint env ty1 ty2)
  then
    Disjoint
  else
    let ty1 = strip_dynamic env ty1 in
    let ty2 = strip_dynamic env ty2 in
    (* We don't flag ~null because typically this is a false positive *)
    if
      Typing_utils.(
        (not (is_nothing env ty1 || is_null env ty1))
        && (not (is_nothing env ty2 || is_null env ty2))
        && is_type_disjoint env ty1 ty2)
    then
      DisjointIgnoringDynamic (ty1, ty2)
    else
      NonDisjoint

let strip_supportdyn env ty =
  let (sd, _, ty) = Typing_utils.strip_supportdyn env ty in
  (sd, ty)

let get_underlying_function_type env ty =
  let (_, opt_ft) = Typing_utils.get_underlying_function_type env ty in
  opt_ft

let set_inside_constructor env =
  {
    env with
    Typing_env_types.genv =
      { env.Typing_env_types.genv with fun_is_ctor = true };
  }

let get_inside_constructor env = env.Typing_env_types.genv.fun_is_ctor

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
  | RIclass of string * Tast.ty list
  | RIdynamic
  | RIerr
  | RIany

let get_receiver_ids env ty =
  let open Typing_defs in
  let rec aux seen acc ty =
    match get_node ty with
    | Tclass ((_, cid), _, tyl) -> RIclass (cid, tyl) :: acc
    | Toption ty
    | Tdependent (_, ty) ->
      aux seen acc ty
    | Tnewtype (n, tyargs, _) ->
      let (_env, ty) =
        Typing_utils.get_newtype_super env (get_reason ty) n tyargs
      in
      aux seen acc ty
    | Tunion tys
    | Tintersection tys ->
      List.fold tys ~init:acc ~f:(aux seen)
    | Tgeneric name when not (List.mem ~equal:String.equal seen name) ->
      let seen = name :: seen in
      let upper_bounds = Typing_env.get_upper_bounds env name in
      Typing_set.fold (fun ty acc -> aux seen acc ty) upper_bounds acc
    | Tdynamic -> [RIdynamic]
    | Tany _ -> [RIany]
    | _ -> acc
  in
  List.rev (aux [] [] (Typing_expand.fully_expand env ty))

let get_class_ids env ty =
  get_receiver_ids env ty
  |> List.filter_map ~f:(function
         | RIclass (cid, _) -> Some cid
         | _ -> None)

let get_label_receiver_ty env ty =
  let env = Typing_env.open_tyvars env Pos.none in
  let (env, ty_in) = Typing_env.fresh_type env Pos.none in
  let label_ty =
    Typing_defs.(
      mk
        ( Reason.none,
          Tnewtype
            ( Naming_special_names.Classes.cEnumClassLabel,
              [ty_in; Typing_make_type.nothing Reason.none],
              Typing_make_type.mixed Reason.none ) ))
  in
  let env = Typing_env.set_tyvar_variance ~flip:true env label_ty in
  let (env, _) = Typing_subtype.sub_type env label_ty ty None in
  let (env, _ty_err) = Typing_solver.close_tyvars_and_solve env in
  Typing_env.expand_type env ty_in

let intersect_with_nonnull = Typing_intersection.intersect_with_nonnull

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

let supports_new_refinement env h =
  let (env, ty) = localize_hint_for_refinement env h in
  if Typing_defs.is_nonnull ty then
    Result.Ok ()
  else
    match Typing_refinement.TyPredicate.of_ty env ty with
    | Result.Error err -> Result.Error err
    | Result.Ok _ -> Result.Ok ()

let localize_no_subst env ~ignore_errors dty =
  let ((env, ty_err_opt), lty) =
    Typing_phase.localize_no_subst env ~ignore_errors dty
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  (env, lty)

let simplify_unions env ty = Typing_union.simplify_unions env ty

let as_bounds_to_non_intersection_type env tys =
  let t = Typing_defs.(mk (Reason.none, Tintersection tys)) in
  (* Need to localize in order to simplify *)
  let (_, locl_ty) = localize_no_subst ~ignore_errors:true env t in
  (* Multiple bounds can be expressed as an intersection,
   * but it might be possible to simplify this *)
  let (_, locl_ty) = Typing_intersection.simplify_intersections env locl_ty in
  let (_, locl_ty) = strip_supportdyn env locl_ty in
  begin
    match Typing_defs.get_node locl_ty with
    | Typing_defs.Tintersection (_ :: _) -> None
    | _ -> Some locl_ty
  end

let simplify_intersections env ty =
  Typing_intersection.simplify_intersections env ty

let union_list env r tyl = Typing_union.union_list env r tyl

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

let restore_saved_env (env : t) (saved_env : Tast.saved_env) : t =
  let ctx =
    Provider_context.map_tcopt env.decl_env.ctx ~f:(fun _tcopt ->
        saved_env.tcopt)
  in
  let decl_env = { env.decl_env with ctx } in
  {
    env with
    decl_env;
    genv = { env.genv with tcopt = saved_env.tcopt };
    inference_env =
      Typing_inference_env.simple_merge
        env.inference_env
        saved_env.inference_env;
    tpenv = saved_env.tpenv;
    fun_tast_info = saved_env.fun_tast_info;
    checked = saved_env.checked;
  }

module EnvFromDef = Typing_env_from_def
open Tast

let check_fun_tast_info_present env = function
  | Some _ -> ()
  | None ->
    Diagnostics.internal_error
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

let module_env ctx md =
  let ctx =
    Provider_context.map_tcopt ctx ~f:(fun _tcopt -> md.md_annotation.tcopt)
  in
  let env = EnvFromDef.module_env ~origin:Decl_counters.Tast ctx md in
  restore_saved_env env md.md_annotation

let def_env ctx d =
  match d with
  | Fun x -> fun_env ctx x
  | Class x -> class_env ctx x
  | Typedef x -> typedef_env ctx x
  | Constant x -> gconst_env ctx x
  | Module x -> module_env ctx x
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
  | FileAttributes _ ->
    empty ctx

let eq_typing_env : (env, Typing_env_types.env) Type.eq = Equal

let is_xhp_child = Typing_xhp.is_xhp_child

let set_allow_wildcards env =
  { env with Typing_env_types.allow_wildcards = true }

let get_allow_wildcards env = env.Typing_env_types.allow_wildcards

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

let fill_in_pos_filename_if_in_current_decl =
  Typing_env.fill_in_pos_filename_if_in_current_decl

let get_check_status env : check_status = env.Typing_env_types.checked

let derive_instantiation env =
  Derive_type_instantiation.derive_instantiation env

let add_typing_error = Typing_error_utils.add_typing_error

let add_warning = Typing_warning_utils.add

let get_tcopt env =
  let open Typing_env_types in
  env.genv.tcopt
