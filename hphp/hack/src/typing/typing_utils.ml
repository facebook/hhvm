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
open Typing_defs_constraints
open Typing_env_types
module SN = Naming_special_names
module Reason = Typing_reason
module Env = Typing_env
module TySet = Typing_set
module ITySet = Internal_type_set
module Cls = Folded_class
module MakeType = Typing_make_type

(*****************************************************************************)
(* Importing what is necessary *)
(*****************************************************************************)
let not_implemented s _ =
  failwith (Printf.sprintf "Function %s not implemented" s)

type expand_typedef_result = {
  env: env;
  ty_err_opt: Typing_error.t option;
  cycles: Type_expansions.cycle_reporter list;
  ty: locl_ty;
  bound: locl_ty;
}

type expand_typedef =
  expand_env ->
  env ->
  Reason.t ->
  string ->
  locl_ty list ->
  expand_typedef_result

let (expand_typedef_ref : expand_typedef ref) =
  ref (fun _ -> not_implemented "expand_typedef")

let expand_typedef x = !expand_typedef_ref x

type sub_type =
  env ->
  ?is_dynamic_aware:bool ->
  ?is_coeffect:bool ->
  ?ignore_readonly:bool ->
  ?class_sub_classname:bool ->
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

type is_sub_type_type = env -> locl_ty -> locl_ty -> bool

type is_sub_type_opt_type = env -> locl_ty -> locl_ty -> bool option

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

let (is_sub_type_opt_ignore_generic_params_ref : is_sub_type_opt_type ref) =
  ref (not_implemented "is_sub_type_opt_ignore_generic_params")

let is_sub_type_opt_ignore_generic_params x =
  !is_sub_type_opt_ignore_generic_params_ref x

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

let (can_sub_type_ref : is_sub_type_type ref) =
  ref (not_implemented "can_sub_type_ref")

let can_sub_type x = !can_sub_type_ref x

type expand_typeconst =
  expand_env ->
  env ->
  ?ignore_errors:bool ->
  ?as_tyvar_with_cnstr:Pos.t option ->
  locl_ty ->
  pos_id ->
  allow_abstract_tconst:bool ->
  (env * Typing_error.t option * Type_expansions.cycle_reporter list) * locl_ty

let (expand_typeconst_ref : expand_typeconst ref) =
  ref (not_implemented "expand_typeconst")

let expand_typeconst x = !expand_typeconst_ref x

type union =
  env ->
  ?reason:Typing_reason.t ->
  ?approx_cancel_neg:bool ->
  locl_ty ->
  locl_ty ->
  env * locl_ty

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
  ?on_tyvar:(env -> Reason.t -> Tvid.t -> env * locl_ty) ->
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
  ?on_tyvar:(env -> Reason.t -> Tvid.t -> env * locl_ty) ->
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

let localize ~ety_env = !localize_ref ~ety_env

let (localize_disjoint_union_ref : localize ref) =
  ref (fun ~ety_env:_ -> not_implemented "localize_disjoint_union")

let localize_disjoint_union ~ety_env = !localize_disjoint_union_ref ~ety_env

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
  let mixed = LoclType (MakeType.mixed Reason.none) in
  is_sub_type_for_union_i env mixed ty

let is_mixed env ty = is_mixed_i env (LoclType ty)

let is_nothing_i env ty =
  let nothing = LoclType (MakeType.nothing Reason.none) in
  is_sub_type_for_union_i env ty nothing

let is_nothing env ty = is_nothing_i env (LoclType ty)

let is_null env ty =
  let null = MakeType.null Reason.none in
  is_sub_type_for_union env ty null

let is_dynamic env ty =
  let dynamic = MakeType.dynamic Reason.none in
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

let is_tyvar_error Typing_env_types.{ inference_env; _ } ty =
  match deref ty with
  | (_, Tvar tvid) -> Typing_inference_env.is_error inference_env tvid
  | _ -> false

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

let get_newtype_super env r name tyargs =
  if Env.is_enum env name then
    match Env.get_enum_constraint env name with
    | Decl_entry.Found (Some ty) ->
      let ((env, _err), ty) = localize ~ety_env:empty_expand_env env ty in
      (env, ty)
    | _ ->
      ( env,
        (match Env.get_enum env name with
        | Decl_entry.Found tc ->
          MakeType.arraykey
            (Reason.implicit_upper_bound (Cls.pos tc, "arraykey"))
        | _ ->
          MakeType.arraykey
            (Reason.implicit_upper_bound (Reason.to_pos r, "arraykey"))) )
  else begin
    match Env.get_typedef env name with
    | Decl_entry.Found _
      when (not (Typing_reason.Predicates.is_opaque_type_from_module r))
           || String.equal name SN.Classes.cSupportDyn ->
      let { env; ty_err_opt = _; cycles; ty = _; bound } =
        expand_typedef empty_expand_env env r name tyargs
      in
      let (env, bound) =
        if List.is_empty cycles then
          (env, bound)
        else
          let r = Typing_reason.illegal_recursive_type (get_pos bound) name in
          (env, MakeType.mixed r)
      in
      ( env,
        (* It's not helpful to report the trivial bound for FunctionRef. Also,
         * preserve the reason for opaque types created during localization *)
        if
          String.equal name Naming_special_names.Classes.cFunctionRef
          || Typing_reason.Predicates.is_opaque_type_from_module r
        then
          with_reason bound r
        else
          bound )
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable
    | _ ->
      let ty =
        if TypecheckerOptions.everything_sdt env.genv.tcopt then
          Typing_make_type.supportdyn_mixed r
        else
          Typing_make_type.mixed r
      in
      (env, ty)
  end

(*****************************************************************************
 * Get the "as" constraints from an abstract type or generic parameter, or
 * return the type itself if there is no "as" constraint.
 * In the case of a generic parameter whose "as" constraint is another
 * generic parameter, repeat the process until a type is reached that is not
 * a generic parameter. Don't loop on cycles.
 * (For example, function foo<Tu as Tv, Tv as Tu>(...))
 * Also breaks apart intersections.
 *****************************************************************************)
let get_concrete_supertypes
    ?(expand_supportdyn = true)
    ?(include_case_types = false)
    ~abstract_enum
    env
    ty =
  let rec iter seen env acc tyl =
    match tyl with
    | [] -> (env, acc)
    | ty :: tyl ->
      let (env, ty) = Env.expand_type env ty in
      (match get_node ty with
      | Tnewtype (cid, _, _)
        when (not expand_supportdyn) && String.equal cid SN.Classes.cSupportDyn
        ->
        iter seen env (TySet.add ty acc) tyl
      | Tnewtype (cid, tyargs, _) ->
        let (env, as_ty) = get_newtype_super env (get_reason ty) cid tyargs in
        let (env, as_ty) = Env.expand_type env as_ty in
        (* Enums with arraykey upper bound are treated as "abstract" *)
        if abstract_enum && is_prim Aast.Tarraykey as_ty && Env.is_enum env cid
        then
          iter seen env acc tyl
        else if String.equal cid SN.Classes.cFunctionRef then
          iter seen env acc (as_ty :: tyl)
        (* Don't expand enums or newtype; just return the type itself *)
        else begin
          let acc = TySet.add as_ty acc in
          let acc =
            if include_case_types then
              TySet.add ty acc
            else
              acc
          in
          iter seen env acc tyl
        end
      | Tdependent (_, ty) -> iter seen env (TySet.add ty acc) tyl
      | Tgeneric n ->
        if SSet.mem n seen then
          iter seen env acc tyl
        else
          iter
            (SSet.add n seen)
            env
            acc
            (TySet.elements (Env.get_upper_bounds env n) @ tyl)
      | Tintersection tyl' -> iter seen env acc (tyl' @ tyl)
      | _ -> iter seen env (TySet.add ty acc) tyl)
  in
  let (env, resl) = iter SSet.empty env TySet.empty [ty] in
  (env, TySet.elements resl)

(** Run a function on an intersection represented by a list of types.
    We stay liberal with errors:
    discard the result of any run which has produced an error.
    If all runs have produced an error, gather all errors and results and add errors.
    Note: warnings should NOT affect which results are considered valid
     *)
let run_on_intersection :
    'env -> f:('env -> locl_ty -> 'env * 'a) -> locl_ty list -> 'env * 'a list =
 fun env ~f tyl ->
  let (env, resl_errors) =
    List.map_env env tyl ~f:(fun env ty ->
        let (errors, (env, result)) = Diagnostics.do_ @@ fun () -> f env ty in
        (env, (result, errors)))
  in

  let has_only_warnings err =
    Diagnostics.is_empty (Diagnostics.filter_out_warnings err)
  in
  let valid_resl =
    List.filter resl_errors ~f:(fun (_, err) -> has_only_warnings err)
    |> List.map ~f:fst
  in
  (* Always merge warnings from all branches so they're not lost *)
  List.iter resl_errors ~f:(fun (_, err) ->
      let warnings_only =
        Diagnostics.filter err ~f:(fun _path error ->
            match error.User_diagnostic.severity with
            | User_diagnostic.Warning -> true
            | User_diagnostic.Err -> false)
      in
      Diagnostics.merge_into_current warnings_only);
  let resl =
    if not (List.is_empty valid_resl) then
      valid_resl
    else (
      List.iter resl_errors ~f:(fun (_, err) ->
          Diagnostics.merge_into_current (Diagnostics.filter_out_warnings err));
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
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tnewtype (name, [tyarg], _) when String.equal name SN.Classes.cSupportDyn ->
    let (_, env, ty) = strip_supportdyn env tyarg in
    (true, env, ty)
  | _ -> (false, env, ty)

let rec get_underlying_function_type env ty =
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tnewtype (name, [tyarg], _)
    when String.equal name SN.Classes.cSupportDyn
         || String.equal name SN.Classes.cFunctionRef ->
    get_underlying_function_type env tyarg
  | Tfun ft -> (env, Some (get_reason ty, ft))
  | _ -> (env, None)

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
  let rec loop env seen_generics ty =
    let (env, ty) = Env.expand_type env ty in
    let r = get_reason ty in
    let default env =
      let (env, tys) =
        get_concrete_supertypes ~expand_supportdyn ~abstract_enum:true env ty
      in
      let (env, has_no_bounds) =
        no_upper_bound ~include_sd_mixed:expand_supportdyn env tys
      in
      match tys with
      | [ty] when not has_no_bounds -> loop env seen_generics ty
      | _ -> (env, ty)
    in
    match get_node ty with
    | Tnewtype (classname, _, _)
      when String.equal classname SN.Classes.cClassname ->
      (env, ty)
    | Tnewtype (n, [ty], _) when String.equal n SN.Classes.cSupportDyn ->
      let (env, ty) = loop env seen_generics ty in
      if expand_supportdyn then
        (env, ty)
      else
        (env, MakeType.supportdyn r ty)
    (* If we have an expression dependent type and it only has one super
       type, we can treat it similarly to AKdependent _, Some ty *)
    | Tgeneric n when DependentKind.is_generic_dep_ty n -> begin
      match TySet.elements (Env.get_upper_bounds env n) with
      | ty2 :: _ when ty_equal ty ty2 -> (env, ty)
      (* If it's exactly equal, then the base ty is just this one *)
      | ty :: _ ->
        if TySet.mem ty (Env.get_lower_bounds env n) then
          (env, ty)
        else if SSet.mem n seen_generics then
          (env, ty)
        else
          loop env (SSet.add n seen_generics) ty
      | [] -> (env, ty)
    end
    | Tnewtype (cid, tyargs, _) when Env.is_enum env cid ->
      let (env, bound_ty) = get_newtype_super env r cid tyargs in
      if is_prim Aast.Tarraykey bound_ty then
        (env, ty)
      else
        default env
    | Tgeneric _
    | Tnewtype _
    | Tdependent _ ->
      default env
    | _ -> (env, ty)
  in
  loop env SSet.empty ty

let get_printable_shape_field_name = Typing_defs.TShapeField.name

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
(* Function parameters *)
(*****************************************************************************)

let default_fun_param ~readonly ?(pos = Pos_or_decl.none) ty : 'a fun_param =
  {
    fp_pos = pos;
    fp_name = None;
    fp_type = ty;
    fp_flags =
      make_fp_flags
        ~mode:FPnormal
        ~accept_disposable:false
        ~is_optional:false
        ~readonly
        ~ignore_readonly_error:false
        ~splat:false
        ~named:false;
    fp_def_value = None;
  }

let tany = Env.tany

let mk_tany env p = mk (Reason.witness p, tany env)

let collect_enum_class_upper_bounds env name =
  (* the boolean ok is here to see if we find anything at all,
   * and prevents us to return the initial mixed value if nothing
   * is to be found.
   *)
  let rec collect seen ok result name =
    let upper_bounds = Env.get_upper_bounds env name in
    TySet.fold
      (fun lty (seen, ok, result) ->
        match get_node lty with
        | Tclass ((_, name), _, _) when Env.is_enum_class env name ->
          let result =
            (* We build an intersection, but only care about the result
             * if it is simplified to a singleton type, so the following
             * reason is not useful. We could add a dedicated reason
             * for more precise error reporting.
             *)
            let r = Reason.none in
            mk (r, Tintersection [result; lty])
          in
          (seen, true, result)
        | Tgeneric name when not (SSet.mem name seen) ->
          collect (SSet.add name seen) ok result name
        | _ -> (seen, ok, result))
      upper_bounds
      (seen, ok, result)
  in
  let mixed = MakeType.mixed Reason.none in
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
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    false
  | Decl_entry.Found cls_sub -> Cls.has_ancestor cls_sub c_super

let class_has_no_params env c =
  match Env.get_class env c with
  | Decl_entry.Found cls -> List.is_empty (Cls.tparams cls)
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    false

(* Is super_id an ancestor of sub_id, including through requires steps?
 * Maintain a visited set to catch cycles (these are rejected in folding but
 * the definitions survive).
 *)
let rec has_ancestor_including_req ~visited env cls super_id =
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
           has_ancestor_including_req_refl ~visited env name super_id
         | _ -> false))

and has_ancestor_including_req_refl ~visited env sub_id super_id =
  (not (SSet.mem sub_id visited))
  && (String.equal sub_id super_id
     ||
     match Env.get_class env sub_id with
     | Decl_entry.DoesNotExist
     | Decl_entry.NotYetAvailable ->
       false
     | Decl_entry.Found cls ->
       has_ancestor_including_req
         ~visited:(SSet.add sub_id visited)
         env
         cls
         super_id)

let has_ancestor_including_req = has_ancestor_including_req ~visited:SSet.empty

let has_ancestor_including_req_refl =
  has_ancestor_including_req_refl ~visited:SSet.empty

(* Return true if the type is known to support dynamic (i.e. is a subtype of supportdyn<mixed>)
 * Note that we look at bounds on type variables, so vec<#0> produces true if #0 has a
 * bound that supports dynamic
 *)
let rec is_supportdyn ~visited_tyvars ~visited_typarams env ty =
  let (env, ty) = Env.expand_type env ty in
  let recurse ty = is_supportdyn ~visited_tyvars ~visited_typarams env ty in
  match get_node ty with
  | Tprim _
  | Tdynamic
  | Tlabel _
  | Tany _ ->
    true
  | Tunion tyl -> List.for_all tyl ~f:recurse
  | Tintersection tyl -> List.exists tyl ~f:recurse
  | Tnonnull -> false
  | Tfun ft -> get_ft_support_dynamic_type ft
  | Toption ty -> recurse ty
  | Ttuple { t_required; t_optional; t_extra = Tvariadic t_vs | Tsplat t_vs } ->
    List.for_all t_required ~f:recurse
    && List.for_all t_optional ~f:recurse
    && recurse t_vs
  | Tshape { s_origin = _; s_unknown_value; s_fields } ->
    recurse s_unknown_value
    && TShapeMap.fold
         (fun _ { sft_ty; _ } d -> d && recurse sft_ty)
         s_fields
         true
  | Tvec_or_dict (ty1, ty2) -> recurse ty1 && recurse ty2
  | Tnewtype (n, [_], _) when String.equal n SN.Classes.cSupportDyn -> true
  | Tnewtype (n, tyl, _) ->
    let (_, bound) = get_newtype_super env (get_reason ty) n tyl in
    recurse bound
  | Tdependent (_, ty) -> recurse ty
  | Tclass ((_, class_id), _exact, tyargs) ->
    List.for_all tyargs ~f:recurse
    &&
    let class_def_sub = Env.get_class env class_id in
    (match class_def_sub with
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      true
    | Decl_entry.Found class_sub ->
      Cls.get_support_dynamic_type class_sub || Env.is_enum env class_id)
  | Tvar id ->
    if Tvid.Set.mem id visited_tyvars || is_tyvar_error env ty then
      false
    else
      let upper_bounds = Env.get_tyvar_upper_bounds env id in
      let visited_tyvars = Tvid.Set.add id visited_tyvars in
      ITySet.exists
        (is_supportdyn_i ~visited_tyvars ~visited_typarams env)
        upper_bounds
  | Tgeneric name ->
    if SSet.mem name visited_typarams then
      false
    else
      let upper_bounds = Env.get_upper_bounds env name in
      let visited_typarams = SSet.add name visited_typarams in
      Typing_set.exists
        (is_supportdyn ~visited_tyvars ~visited_typarams env)
        upper_bounds
  | Tclass_ptr ty -> recurse ty
  | _ -> false

and is_supportdyn_i ~visited_tyvars ~visited_typarams env ty =
  match ty with
  | LoclType ty -> is_supportdyn ~visited_tyvars ~visited_typarams env ty
  | _ -> false

(* Alternative implementation of is_supportdyn (compare below) that makes use of tyvar
 * bounds e.g. if #0 <: supportdyn<mixed> then function will return true on vec<#0>.
 * Ideally we would have a variant of subtyping that does this, but until that's the
 * case we can use this function to get the same behaviour.
 *)
let is_supportdyn_use_tyvar_bounds env ty =
  is_supportdyn
    ~visited_tyvars:Tvid.Set.empty
    ~visited_typarams:SSet.empty
    env
    ty

let is_supportdyn env ty =
  is_sub_type_for_union env ty (MakeType.supportdyn_mixed Reason.none)

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
      (env, MakeType.supportdyn r ty)

let simple_make_supportdyn r env ty =
  let (env, ty) = Env.expand_type env ty in
  ( env,
    match get_node ty with
    | Tnewtype (n, _, _) when String.equal n SN.Classes.cSupportDyn -> ty
    | _ -> MakeType.supportdyn r ty )

let map_supportdyn env ty f =
  let (env, ty) = Env.expand_type env ty in
  match deref ty with
  | (r, Tnewtype (name, [tyarg], _))
    when String.equal name SN.Classes.cSupportDyn ->
    let (_, env, ty) = strip_supportdyn env tyarg in
    let (env, ty) = f env ty in
    make_supportdyn r env ty
  | _ -> f env ty

let make_supportdyn_decl_type p r ty =
  mk (r, Tapply ((p, SN.Classes.cSupportDyn), [ty]))

let make_like ?reason env ty =
  let (_env, _) = Env.expand_type env ty in
  match get_node ty with
  | Tdynamic -> ty
  | Tunion tyl when List.exists tyl ~f:Typing_defs.is_dynamic -> ty
  | _ ->
    let r =
      match reason with
      | None -> get_reason ty
      | Some r -> r
    in
    MakeType.locl_like r ty

let make_like_if_enforced env et_enforced et_type =
  match et_enforced with
  | Enforced when Env.get_support_dynamic_type env -> make_like env et_type
  | _ -> et_type

let rec is_capability ty =
  match get_node ty with
  | Tclass ((_, name), _, _)
    when String.is_prefix ~prefix:SN.Capabilities.prefix name ->
    true
  | Tintersection [] -> false
  | Tintersection tyl -> List.for_all ~f:is_capability tyl
  | Tgeneric s when SN.Coeffects.is_generated_generic s -> true
  | _ -> false

let is_capability_i ty =
  match ty with
  | LoclType ty -> is_capability ty
  | ConstraintType _ -> false

let supports_dynamic env ty =
  let r = get_reason ty in
  sub_type env ty (MakeType.supportdyn_mixed r)

let get_case_type_variants_as_type
    ((first_ty, _where_constraints) : typedef_case_type_variant)
    (variants : typedef_case_type_variant list) =
  (* TODO T201569125 audit callers to see if they need to account for where clauses *)
  match List.last variants with
  | None -> first_ty
  | Some (last_ty, _where_constraints) ->
    let pos =
      Pos_or_decl.btw
        (Typing_reason.to_pos @@ get_reason first_ty)
        (Typing_reason.to_pos @@ get_reason last_ty)
    in
    let tyl = first_ty :: List.map variants ~f:fst in
    Typing_make_type.union (Typing_reason.hint pos) tyl
