(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Typing_defs
open Typing_env_types
module SN = Naming_special_names
module Reason = Typing_reason
module Env = Typing_env
module TySet = Typing_set
module Cls = Decl_provider.Class
module MakeType = Typing_make_type

(*****************************************************************************)
(* Importing what is necessary *)
(*****************************************************************************)
let not_implemented s _ =
  failwith (Printf.sprintf "Function %s not implemented" s)

type expand_typedef =
  expand_env ->
  env ->
  Reason.t ->
  string ->
  locl_ty list ->
  (env * Typing_error.t option) * locl_ty

let (expand_typedef_ref : expand_typedef ref) =
  ref (not_implemented "expand_typedef")

let expand_typedef x = !expand_typedef_ref x

type sub_type =
  env ->
  ?coerce:Typing_logic.coercion_direction option ->
  ?is_coeffect:bool ->
  locl_ty ->
  locl_ty ->
  Typing_error.Reasons_callback.t option ->
  env * Typing_error.t option

let (sub_type_ref : sub_type ref) = ref (not_implemented "sub_type")

let sub_type x = !sub_type_ref x

type sub_type_i =
  env ->
  ?is_coeffect:bool ->
  internal_type ->
  internal_type ->
  Typing_error.Reasons_callback.t option ->
  env * Typing_error.t option

let (sub_type_i_ref : sub_type_i ref) = ref (not_implemented "sub_type_i")

let sub_type_i env ?(is_coeffect = false) x = !sub_type_i_ref env ~is_coeffect x

type sub_type_with_dynamic_as_bottom =
  env ->
  locl_ty ->
  locl_ty ->
  Typing_error.Reasons_callback.t option ->
  env * Typing_error.t option

let (sub_type_with_dynamic_as_bottom_ref : sub_type_with_dynamic_as_bottom ref)
    =
  ref (not_implemented "sub_type_with_dynamic_as_bottom")

let sub_type_with_dynamic_as_bottom x = !sub_type_with_dynamic_as_bottom_ref x

type is_sub_type_type = env -> locl_ty -> locl_ty -> bool

let (is_sub_type_ref : is_sub_type_type ref) =
  ref (not_implemented "is_sub_type")

let is_sub_type x = !is_sub_type_ref x

let (is_sub_type_for_union_ref : (env -> locl_ty -> locl_ty -> bool) ref) =
  ref (not_implemented "is_sub_type_for_union")

let is_sub_type_for_union x = !is_sub_type_for_union_ref x

let (is_sub_type_for_union_i_ref :
      (env -> internal_type -> internal_type -> bool) ref) =
  ref (not_implemented "is_sub_type_for_union_i")

let is_sub_type_for_union_i x = !is_sub_type_for_union_i_ref x

let (is_type_disjoint_ref : (env -> locl_ty -> locl_ty -> bool) ref) =
  ref (not_implemented "is_type_disjoint")

let is_type_disjoint x = !is_type_disjoint_ref x

let (is_sub_type_ignore_generic_params_ref : is_sub_type_type ref) =
  ref (not_implemented "is_sub_type_ignore_generic_params")

let is_sub_type_ignore_generic_params x =
  !is_sub_type_ignore_generic_params_ref x

type add_constraint =
  env ->
  Ast_defs.constraint_kind ->
  locl_ty ->
  locl_ty ->
  Typing_error.Reasons_callback.t option ->
  env

let (add_constraint_ref : add_constraint ref) =
  ref (not_implemented "add_constraint")

let add_constraint x = !add_constraint_ref x

type expand_typeconst =
  expand_env ->
  env ->
  ?ignore_errors:bool ->
  ?as_tyvar_with_cnstr:Pos.t option ->
  locl_ty ->
  pos_id ->
  allow_abstract_tconst:bool ->
  (env * Typing_error.t option) * locl_ty

let (expand_typeconst_ref : expand_typeconst ref) =
  ref (not_implemented "expand_typeconst")

let expand_typeconst x = !expand_typeconst_ref x

type union =
  env -> ?approx_cancel_neg:bool -> locl_ty -> locl_ty -> env * locl_ty

let (union_ref : union ref) = ref (not_implemented "union")

let union x = !union_ref x

type make_union =
  env ->
  Reason.t ->
  locl_ty list ->
  Reason.t option ->
  Reason.t option ->
  env * locl_ty

let (make_union_ref : make_union ref) = ref (not_implemented "make_union")

let make_union env = !make_union_ref env

type union_i =
  env ->
  ?approx_cancel_neg:bool ->
  Reason.t ->
  internal_type ->
  locl_ty ->
  env * internal_type

let (union_i_ref : union_i ref) = ref (not_implemented "union")

let union_i x = !union_i_ref x

type union_list =
  env -> ?approx_cancel_neg:bool -> Reason.t -> locl_ty list -> env * locl_ty

let (union_list_ref : union_list ref) = ref (not_implemented "union_list")

let union_list x = !union_list_ref x

type fold_union =
  env -> ?approx_cancel_neg:bool -> Reason.t -> locl_ty list -> env * locl_ty

let (fold_union_ref : fold_union ref) = ref (not_implemented "fold_union")

type simplify_unions =
  env ->
  ?approx_cancel_neg:bool ->
  ?on_tyvar:(env -> Reason.t -> Ident.t -> env * locl_ty) ->
  locl_ty ->
  env * locl_ty

let (simplify_unions_ref : simplify_unions ref) =
  ref (not_implemented "simplify_unions")

let simplify_unions x = !simplify_unions_ref x

type intersect_list = env -> Reason.t -> locl_ty list -> env * locl_ty

let (intersect_list_ref : intersect_list ref) =
  ref (not_implemented "intersect_list")

let intersect_list x = !intersect_list_ref x

type approx =
  | ApproxUp
  | ApproxDown
[@@deriving eq]

type negate_type = env -> Reason.t -> locl_ty -> approx:approx -> env * locl_ty

let (negate_type_ref : negate_type ref) = ref (not_implemented "negate_type")

let negate_type x = !negate_type_ref x

type simplify_intersections =
  env ->
  ?on_tyvar:(env -> Reason.t -> int -> env * locl_ty) ->
  locl_ty ->
  env * locl_ty

let (simplify_intersections_ref : simplify_intersections ref) =
  ref (not_implemented "simplify_intersections")

let simplify_intersections x = !simplify_intersections_ref x

type localize_no_subst =
  env ->
  ignore_errors:bool ->
  decl_ty ->
  (env * Typing_error.t option) * locl_ty

let (localize_no_subst_ref : localize_no_subst ref) =
  ref (not_implemented "localize_no_subst")

let localize_no_subst x = !localize_no_subst_ref x

type localize =
  ety_env:expand_env ->
  env ->
  decl_ty ->
  (env * Typing_error.t option) * locl_ty

let (localize_ref : localize ref) =
  ref (fun ~ety_env:_ -> not_implemented "localize")

let localize x = !localize_ref x

(*****************************************************************************)
(* Checking properties of types *)
(*****************************************************************************)

let is_class ty =
  match get_node ty with
  | Tclass _ -> true
  | _ -> false

let is_class_i ty =
  match ty with
  | ConstraintType _ -> false
  | LoclType ty -> is_class ty

let is_mixed_i env ty =
  let mixed = LoclType (MakeType.mixed Reason.Rnone) in
  is_sub_type_for_union_i env mixed ty

let is_mixed env ty = is_mixed_i env (LoclType ty)

let is_nothing_i env ty =
  let nothing = LoclType (MakeType.nothing Reason.Rnone) in
  is_sub_type_for_union_i env ty nothing

let is_nothing env ty = is_nothing_i env (LoclType ty)

let is_dynamic env ty =
  let dynamic = MakeType.dynamic Reason.Rnone in
  (is_sub_type_for_union env dynamic ty && not (is_mixed env ty))
  || (is_sub_type_for_union env ty dynamic && not (is_nothing env ty))

let rec is_any env ty =
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tany _ -> true
  | Tunion tyl -> List.for_all tyl ~f:(is_any env)
  | Tintersection tyl -> List.exists tyl ~f:(is_any env)
  | _ -> false

let is_tunion env ty =
  let (_env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tunion _ -> true
  | _ -> false

let is_tintersection env ty =
  let (_env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tintersection _ -> true
  | _ -> false

let is_tyvar env ty =
  let (_env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tvar _ -> true
  | _ -> false

let is_opt_tyvar env ty =
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Toption ty -> is_tyvar env ty
  | _ -> false

let is_tyvar_error env ty =
  let (_env, ty) = Env.expand_type env ty in
  let rec is_tyvar_error_reason r =
    match r with
    | Reason.Rtype_variable_error _ -> true
    | Reason.Rtype_access (r, _) -> is_tyvar_error_reason r
    | Reason.Rtypeconst (r, _, _, _) -> is_tyvar_error_reason r
    | _ -> false
  in
  match deref ty with
  | (r, Tvar _) -> is_tyvar_error_reason r
  | _ -> false

(** Simplify unions and intersections of constraint
types which involve mixed or nothing. *)
let simplify_constraint_type env ty =
  match deref_constraint_type ty with
  | (_, TCunion (lty, cty)) ->
    if is_nothing env lty then
      (env, ConstraintType cty)
    else if is_mixed env lty then
      (env, LoclType lty)
    else
      (env, ConstraintType ty)
  | (_, TCintersection (lty, cty)) ->
    if is_nothing env lty then
      (env, LoclType lty)
    else if is_mixed env lty then
      (env, ConstraintType cty)
    else
      (env, ConstraintType ty)
  | (_, Thas_member _)
  | (_, Thas_type_member _)
  | (_, Tcan_index _)
  | (_, Tcan_traverse _)
  | (_, Tdestructure _) ->
    (env, ConstraintType ty)

let contains_unresolved_tyvars env ty =
  let finder =
    object (this)
      inherit [env * bool] Type_visitor.locl_type_visitor as super

      method! on_tvar (env, occurs) r v =
        let (env, ty) = Env.expand_var env r v in
        if Typing_defs.is_tyvar ty then
          (env, true)
        else
          this#on_type (env, occurs) ty

      method! on_type (env, occurs) ty =
        if occurs then
          (env, occurs)
        else
          super#on_type (env, occurs) ty
    end
  in
  finder#on_type (env, false) ty

let contains_tvar_decl (t : decl_ty) =
  let finder =
    object
      inherit [bool] Type_visitor.decl_type_visitor as parent

      method! on_tvar _found _r _v = true

      method! on_type found ty =
        if found then
          true
        else
          parent#on_type found ty
    end
  in
  finder#on_type false t

exception FoundGeneric of Pos_or_decl.t

let contains_generic_decl : Typing_defs.decl_ty -> Pos_or_decl.t option =
 fun ty ->
  let visitor =
    object
      inherit [_] Type_visitor.decl_type_visitor as super

      method! on_type env ty =
        match get_node ty with
        | Tgeneric _ -> raise (FoundGeneric (get_pos ty))
        | _ -> super#on_type env ty
    end
  in
  try
    visitor#on_type () ty;
    None
  with
  | FoundGeneric p -> Some p

let wrap_union_inter_ty_in_var env r ty =
  if is_union_or_inter_type ty then
    let (env, contains_unresolved_tyvars) = contains_unresolved_tyvars env ty in
    if contains_unresolved_tyvars then
      Env.wrap_ty_in_var env r ty
    else
      (env, ty)
  else
    (env, ty)

(*****************************************************************************
 * Get the "as" constraints from an abstract type or generic parameter, or
 * return the type itself if there is no "as" constraint.
 * In the case of a generic parameter whose "as" constraint is another
 * generic parameter, repeat the process until a type is reached that is not
 * a generic parameter. Don't loop on cycles.
 * (For example, function foo<Tu as Tv, Tv as Tu>(...))
 * Also breaks apart intersections.
 *****************************************************************************)
let get_concrete_supertypes ?(expand_supportdyn = true) ~abstract_enum env ty =
  let rec iter seen env acc tyl =
    match tyl with
    | [] -> (env, acc)
    | ty :: tyl ->
      let (env, ty) = Env.expand_type env ty in
      (match get_node ty with
      (* Enums with arraykey upper bound are treated as "abstract" *)
      | Tnewtype (cid, _, bound_ty)
        when abstract_enum
             && is_prim Aast.Tarraykey bound_ty
             && Env.is_enum env cid ->
        iter seen env acc tyl
      (* Don't expand enums or newtype; just return the type itself *)
      | Tnewtype (n, _, ty)
        when expand_supportdyn || not (String.equal n SN.Classes.cSupportDyn) ->
        iter seen env (TySet.add ty acc) tyl
      | Tdependent (_, ty) -> iter seen env (TySet.add ty acc) tyl
      | Tgeneric (n, targs) ->
        if SSet.mem n seen then
          iter seen env acc tyl
        else
          iter
            (SSet.add n seen)
            env
            acc
            (TySet.elements (Env.get_upper_bounds env n targs) @ tyl)
      | Tintersection tyl' -> iter seen env acc (tyl' @ tyl)
      | _ -> iter seen env (TySet.add ty acc) tyl)
  in
  let (env, resl) = iter SSet.empty env TySet.empty [ty] in
  (env, TySet.elements resl)

(** Run a function on an intersection represented by a list of types.
    We stay liberal with errors:
    discard the result of any run which has produced an error.
    If all runs have produced an error, gather all errors and results and add errors. *)
let run_on_intersection :
    'env -> f:('env -> locl_ty -> 'env * 'a) -> locl_ty list -> 'env * 'a list =
 fun env ~f tyl ->
  let (env, resl_errors) =
    List.map_env env tyl ~f:(fun env ty ->
        let (errors, (env, result)) = Errors.do_ @@ fun () -> f env ty in
        (env, (result, errors)))
  in
  let valid_resl =
    List.filter resl_errors ~f:(fun (_, err) -> Errors.is_empty err)
    |> List.map ~f:fst
  in
  let resl =
    if not (List.is_empty valid_resl) then
      valid_resl
    else (
      List.iter resl_errors ~f:(fun (_, err) -> Errors.merge_into_current err);
      List.map ~f:fst resl_errors
    )
  in
  (env, resl)

let run_on_intersection_with_ty_err env ~f tys =
  let (env, resl) =
    List.map_env env tys ~f:(fun env ty ->
        let ((env, ty_err_opt), res) = f env ty in
        (env, (res, ty_err_opt)))
  in
  (* Partition the results into those with and without errors *)
  let res =
    List.partition_map resl ~f:(function
        | (res, None) -> Either.first res
        | (res, Some err) -> Either.second (res, err))
  in
  match res with
  | ([], with_err) ->
    (* We have no element of the intersection without error
       Take the intersection of the errors so we remember where they came from
       when applying error suppression
    *)
    let (res, errs) = List.unzip with_err in
    ((env, Typing_error.intersect_opt errs), res)
  | (without_err, _) ->
    (* We have at least one that is without error so return the elements without
       error and indicate there is no error for the intersection *)
    ((env, None), without_err)

(** As above but allow functions which also return subtyping/coercion error
    information *)
let run_on_intersection_res env ~f tyl =
  let g env ty =
    let (env, a, b) = f env ty in
    (env, (a, b))
  in
  let (env, pairs) = run_on_intersection env ~f:g tyl in
  let (res, errs) = List.unzip pairs in
  (env, res, errs)

let run_on_intersection_key_value_res env ~f tyl =
  let g env ty =
    let (env, a, b, c) = f env ty in
    (env, (a, b, c))
  in
  let (env, triples) = run_on_intersection env ~f:g tyl in
  let (res, key_errs, errs) = List.unzip3 triples in
  (env, res, key_errs, errs)

let run_on_intersection_array_key_value_res env ~f tyl =
  let g env ty =
    let (env, a, b, c, d) = f env ty in
    (env, (a, b, c, d))
  in
  let (env, quads) = run_on_intersection env ~f:g tyl in
  let (res, arr_errs, key_errs, val_errs) = List.unzip4 quads in
  (env, res, arr_errs, key_errs, val_errs)

let rec strip_supportdyn env ty =
  let (env, ty) = Typing_env.expand_type env ty in
  match get_node ty with
  | Tnewtype (name, [tyarg], _) when String.equal name SN.Classes.cSupportDyn ->
    let (_, env, ty) = strip_supportdyn env tyarg in
    (true, env, ty)
  | _ -> (false, env, ty)

(* The list of types should not be considered to be a bound if it is all mixed
   or supportdyn<mixed> (when include_sd_mixed is set *)
let no_upper_bound ~include_sd_mixed env tyl =
  List.for_all_env env tyl ~f:(fun env ty ->
      if is_mixed env ty then
        (env, true)
      else if include_sd_mixed then
        let (stripped, env, ty) = strip_supportdyn env ty in
        (env, stripped && is_mixed env ty)
      else
        (env, false))

(* Gets the base type of an abstract type *)
let get_base_type ?(expand_supportdyn = true) env ty =
  let rec loop seen_generics ty =
    let (env, ty) = Env.expand_type env ty in
    let r = get_reason ty in
    match get_node ty with
    | Tnewtype (classname, _, _)
      when String.equal classname SN.Classes.cClassname ->
      ty
    | Tnewtype (n, _, ty) when String.equal n SN.Classes.cSupportDyn ->
      let ty = loop seen_generics ty in
      if expand_supportdyn then
        ty
      else
        MakeType.supportdyn r ty
    (* If we have an expression dependent type and it only has one super
       type, we can treat it similarly to AKdependent _, Some ty *)
    | Tgeneric (n, targs) when DependentKind.is_generic_dep_ty n -> begin
      match TySet.elements (Env.get_upper_bounds env n targs) with
      | ty2 :: _ when ty_equal ty ty2 -> ty
      (* If it's exactly equal, then the base ty is just this one *)
      | ty :: _ ->
        if TySet.mem ty (Env.get_lower_bounds env n targs) then
          ty
        else if SSet.mem n seen_generics then
          ty
        else
          loop (SSet.add n seen_generics) ty
      | [] -> ty
    end
    | Tnewtype (cid, _, bound_ty)
      when is_prim Aast.Tarraykey bound_ty && Env.is_enum env cid ->
      ty
    | Tgeneric _
    | Tnewtype _
    | Tdependent _ ->
      let (env, tys) =
        get_concrete_supertypes ~expand_supportdyn ~abstract_enum:true env ty
      in
      let (_env, has_no_bounds) =
        no_upper_bound ~include_sd_mixed:expand_supportdyn env tys
      in
      (match tys with
      | [ty] when not has_no_bounds -> loop seen_generics ty
      | _ -> ty)
    | _ -> ty
  in
  loop SSet.empty ty

let get_printable_shape_field_name = Typing_defs.TShapeField.name

let shape_field_name_with_ty_err env (_, p, field) =
  match field with
  | Aast.Int name -> (Some (Ast_defs.SFlit_int (p, name)), None)
  | Aast.String name -> (Some (Ast_defs.SFlit_str (p, name)), None)
  | Aast.Class_const ((_, _, Aast.CI x), y) ->
    (Some (Ast_defs.SFclass_const (x, y)), None)
  | Aast.Class_const ((_, _, Aast.CIself), y) ->
    let this =
      match Env.get_self_ty env with
      | None -> None
      | Some c_ty ->
        (match get_node c_ty with
        | Tclass (sid, _, _) -> Some (Positioned.unsafe_to_raw_positioned sid)
        | _ -> None)
    in
    (match this with
    | Some sid -> (Some (Ast_defs.SFclass_const (sid, y)), None)
    | None ->
      ( None,
        Some
          Typing_error.(
            primary @@ Primary.Expected_class { pos = p; suffix = None }) ))
  | _ ->
    let err =
      Typing_error.Primary.Shape.(
        Invalid_shape_field_name { pos = p; is_empty = false })
    in
    (None, Some (Typing_error.shape err))

let shape_field_name env field =
  let (res, error) = shape_field_name_with_ty_err env field in
  (match error with
  | Some error -> Errors.add_typing_error error
  | None -> ());
  res

(*****************************************************************************)
(* Class types *)
(*****************************************************************************)

let unwrap_class_type ty =
  match deref ty with
  | (r, Tapply (name, tparaml)) -> (r, name, tparaml)
  | _ -> raise @@ Invalid_argument "unwrap_class_type got non-class"

let try_unwrap_class_type x = Option.try_with (fun () -> unwrap_class_type x)

let class_is_final_and_invariant class_ty =
  Cls.final class_ty
  && List.for_all (Cls.tparams class_ty) ~f:(function
         | { tp_variance = Ast_defs.Invariant; _ } -> true
         | _ -> false)

(*****************************************************************************)
(* Check if a type is not fully constrained *)
(*****************************************************************************)

module HasTany : sig
  val check : locl_ty -> bool

  val check_why : locl_ty -> Reason.t option
end = struct
  let visitor =
    object (_this)
      inherit [Reason.t option] Type_visitor.locl_type_visitor

      method! on_tany _ r = Some r
    end

  let check_why ty = visitor#on_type None ty

  let check ty = Option.is_some (check_why ty)
end

(*****************************************************************************)
(* Function parameters *)
(*****************************************************************************)

let default_fun_param ?(pos = Pos_or_decl.none) ty : 'a fun_param =
  {
    fp_pos = pos;
    fp_name = None;
    fp_type = { et_type = ty; et_enforced = Unenforced };
    fp_flags =
      make_fp_flags
        ~mode:FPnormal
        ~accept_disposable:false
        ~has_default:false
        ~ifc_external:false
        ~ifc_can_call:false
        ~readonly:false;
  }

let tany = Env.tany

let mk_tany env p = mk (Reason.Rwitness p, tany env)

let collect_enum_class_upper_bounds env name =
  (* the boolean ok is here to see if we find anything at all,
   * and prevents us to return the initial mixed value if nothing
   * is to be found.
   *)
  let rec collect seen ok result name =
    let upper_bounds = Env.get_upper_bounds env name [] in
    Typing_set.fold
      (fun lty (seen, ok, result) ->
        match get_node lty with
        | Tclass ((_, name), _, _) when Env.is_enum_class env name ->
          let result =
            (* We build an intersection, but only care about the result
             * if it is simplified to a singleton type, so the following
             * reason is not useful. We could add a dedicated reason
             * for more precise error reporting.
             *)
            let r = Reason.Rnone in
            mk (r, Tintersection [result; lty])
          in
          (seen, true, result)
        | Tgeneric (name, _) when not (SSet.mem name seen) ->
          collect (SSet.add name seen) ok result name
        | _ -> (seen, ok, result))
      upper_bounds
      (seen, ok, result)
  in
  let mixed = MakeType.mixed Reason.Rnone in
  let (_, ok, upper_bound) = collect SSet.empty false mixed name in
  if ok then
    let (env, upper_bound) = simplify_intersections env upper_bound in
    (env, Some upper_bound)
  else
    (env, None)

let make_locl_subst_for_class_tparams classdef tyl =
  if List.is_empty tyl then
    SMap.empty
  else
    Decl_subst.make_locl (Cls.tparams classdef) tyl

let is_sub_class_refl env c_sub c_super =
  String.equal c_sub c_super
  ||
  match Env.get_class env c_sub with
  | None -> false
  | Some cls_sub -> Cls.has_ancestor cls_sub c_super

let class_has_no_params env c =
  match Env.get_class env c with
  | Some cls -> List.is_empty (Decl_provider.Class.tparams cls)
  | None -> false

(* Is super_id an ancestor of sub_id, including through requires steps? *)
let rec has_ancestor_including_req env cls super_id =
  Cls.has_ancestor cls super_id
  ||
  let kind = Cls.kind cls in
  (Ast_defs.is_c_trait kind || Ast_defs.is_c_interface kind)
  && (Cls.requires_ancestor cls super_id
     ||
     let bounds = Cls.upper_bounds_on_this cls in
     List.exists bounds ~f:(fun ty ->
         match get_node ty with
         | Tapply ((_, name), _) ->
           has_ancestor_including_req_refl env name super_id
         | _ -> false))

and has_ancestor_including_req_refl env sub_id super_id =
  String.equal sub_id super_id
  ||
  match Env.get_class env sub_id with
  | None -> false
  | Some cls -> has_ancestor_including_req env cls super_id

let rec try_strip_dynamic_from_union _env r tyl =
  (* search through tyl, and any unions directly-recursively contained in tyl,
     and return those that satisfy f, and those that do not, separately.*)
  let rec partition_union tyl ~f =
    match tyl with
    | [] -> ([], [])
    | t :: tyl ->
      let (dyns, nondyns) = partition_union tyl ~f in
      if f t then
        (t :: dyns, nondyns)
      else (
        match get_node t with
        | Tunion tyl ->
          (match strip_union (get_reason t) tyl with
          | Some (sub_dyns, sub_nondyns) ->
            (sub_dyns @ dyns, sub_nondyns :: nondyns)
          | None -> (dyns, t :: nondyns))
        | _ -> (dyns, t :: nondyns)
      )
  and strip_union r tyl =
    let (dyns, nondyns) = partition_union tyl ~f:Typing_defs.is_dynamic in
    match (dyns, nondyns) with
    | ([], _) -> None
    | (_, _) -> Some (dyns, Typing_make_type.union r nondyns)
  in
  match strip_union r tyl with
  | None -> None
  | Some (_, ty) -> Some ty

and try_strip_dynamic env ty =
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tunion tyl -> try_strip_dynamic_from_union env (get_reason ty) tyl
  | _ -> None

and strip_dynamic env ty =
  match try_strip_dynamic env ty with
  | None -> ty
  | Some ty -> ty

let is_supportdyn env ty =
  is_sub_type_for_union env ty (MakeType.supportdyn_mixed Reason.Rnone)

let rec make_supportdyn r env ty =
  let (env, ty) = Env.expand_type env ty in
  match deref ty with
  | (r', Tintersection tyl) ->
    let (env, tyl) = List.map_env env tyl ~f:(make_supportdyn r) in
    (env, mk (r', Tintersection tyl))
  | (r', Tunion tyl) ->
    let (env, tyl) = List.map_env env tyl ~f:(make_supportdyn r) in
    (env, mk (r', Tunion tyl))
  | _ ->
    if is_supportdyn env ty then
      (env, ty)
    else
      (env, Typing_make_type.supportdyn r ty)

let simple_make_supportdyn r env ty =
  let (env, ty) = Env.expand_type env ty in
  ( env,
    match get_node ty with
    | Tnewtype (n, _, _)
      when String.equal n Naming_special_names.Classes.cSupportDyn ->
      ty
    | _ -> Typing_make_type.supportdyn r ty )

let make_supportdyn_decl_type p r ty =
  mk (r, Tapply ((p, Naming_special_names.Classes.cSupportDyn), [ty]))

let make_like env ty =
  if Typing_defs.is_dynamic ty || Option.is_some (try_strip_dynamic env ty) then
    ty
  else
    let r = get_reason ty in
    Typing_make_type.locl_like r ty

let make_like_if_enforced env ety =
  match ety.et_enforced with
  | Enforced
    when TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env)
         && Env.get_support_dynamic_type env ->
    { ety with et_type = make_like env ety.et_type }
  | _ -> ety

let rec is_capability ty =
  match get_node ty with
  | Tclass ((_, name), _, _)
    when String.is_prefix ~prefix:SN.Capabilities.prefix name ->
    true
  | Tintersection [] -> false
  | Tintersection tyl -> List.for_all ~f:is_capability tyl
  | Tgeneric (s, []) when SN.Coeffects.is_generated_generic s -> true
  | _ -> false

let is_capability_i ty =
  match ty with
  | LoclType ty -> is_capability ty
  | ConstraintType _ -> false

let supports_dynamic env ty =
  let r = get_reason ty in
  sub_type env ty (MakeType.supportdyn_mixed r)
