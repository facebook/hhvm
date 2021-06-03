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
  expand_env -> env -> Reason.t -> string -> locl_ty list -> env * locl_ty

let (expand_typedef_ref : expand_typedef ref) =
  ref (not_implemented "expand_typedef")

let expand_typedef x = !expand_typedef_ref x

type sub_type =
  env ->
  ?coerce:Typing_logic.coercion_direction option ->
  ?is_coeffect:bool ->
  locl_ty ->
  locl_ty ->
  Errors.error_from_reasons_callback ->
  env

let (sub_type_ref : sub_type ref) = ref (not_implemented "sub_type")

let sub_type x = !sub_type_ref x

type sub_type_res =
  env ->
  ?coerce:Typing_logic.coercion_direction option ->
  locl_ty ->
  locl_ty ->
  Errors.error_from_reasons_callback ->
  (env, env) result

let (sub_type_res_ref : sub_type_res ref) = ref (not_implemented "sub_type_res")

let sub_type_res x = !sub_type_res_ref x

type sub_type_i =
  env ->
  ?is_coeffect:bool ->
  internal_type ->
  internal_type ->
  Errors.error_from_reasons_callback ->
  env

let (sub_type_i_ref : sub_type_i ref) = ref (not_implemented "sub_type_i")

let sub_type_i ?(is_coeffect = false) x = !sub_type_i_ref ~is_coeffect x

type sub_type_i_res =
  env ->
  internal_type ->
  internal_type ->
  Errors.error_from_reasons_callback ->
  (env, env) result

let (sub_type_i_res_ref : sub_type_i_res ref) =
  ref (not_implemented "sub_type_i_res")

let sub_type_i_res x = !sub_type_i_res_ref x

type sub_type_with_dynamic_as_bottom =
  env -> locl_ty -> locl_ty -> Errors.error_from_reasons_callback -> env

let (sub_type_with_dynamic_as_bottom_ref : sub_type_with_dynamic_as_bottom ref)
    =
  ref (not_implemented "sub_type_with_dynamic_as_bottom")

let sub_type_with_dynamic_as_bottom x = !sub_type_with_dynamic_as_bottom_ref x

type sub_type_with_dynamic_as_bottom_res =
  env ->
  locl_ty ->
  locl_ty ->
  Errors.error_from_reasons_callback ->
  (env, env) result

let (sub_type_with_dynamic_as_bottom_res_ref :
      sub_type_with_dynamic_as_bottom_res ref) =
  ref (not_implemented "sub_type_with_dynamic_as_bottom_res")

let sub_type_with_dynamic_as_bottom_res x =
  !sub_type_with_dynamic_as_bottom_res_ref x

type is_sub_type_type = env -> locl_ty -> locl_ty -> bool

let (is_sub_type_ref : is_sub_type_type ref) =
  ref (not_implemented "is_sub_type")

let is_sub_type x = !is_sub_type_ref x

let (is_sub_type_for_coercion_ref : is_sub_type_type ref) =
  ref (not_implemented "is_sub_type_for_coercion")

let (is_sub_type_for_union_ref :
      (env ->
      ?coerce:Typing_logic.coercion_direction option ->
      locl_ty ->
      locl_ty ->
      bool)
      ref) =
  ref (not_implemented "is_sub_type_for_union")

let is_sub_type_for_union x = !is_sub_type_for_union_ref x

let (is_sub_type_for_union_i_ref :
      (env ->
      ?coerce:Typing_logic.coercion_direction option ->
      internal_type ->
      internal_type ->
      bool)
      ref) =
  ref (not_implemented "is_sub_type_for_union_i")

let is_sub_type_for_union_i x = !is_sub_type_for_union_i_ref x

let (is_sub_type_ignore_generic_params_ref : is_sub_type_type ref) =
  ref (not_implemented "is_sub_type_ignore_generic_params")

let is_sub_type_ignore_generic_params x =
  !is_sub_type_ignore_generic_params_ref x

type add_constraint =
  env ->
  Ast_defs.constraint_kind ->
  locl_ty ->
  locl_ty ->
  Errors.error_from_reasons_callback ->
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
  root_pos:Pos_or_decl.t ->
  allow_abstract_tconst:bool ->
  env * locl_ty

let (expand_typeconst_ref : expand_typeconst ref) =
  ref (not_implemented "expand_typeconst")

let expand_typeconst x = !expand_typeconst_ref x

type union = env -> locl_ty -> locl_ty -> env * locl_ty

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
  env -> Reason.t -> internal_type -> locl_ty -> env * internal_type

let (union_i_ref : union_i ref) = ref (not_implemented "union")

let union_i x = !union_i_ref x

type union_list = env -> Reason.t -> locl_ty list -> env * locl_ty

let (union_list_ref : union_list ref) = ref (not_implemented "union_list")

let union_list x = !union_list_ref x

type fold_union = env -> Reason.t -> locl_ty list -> env * locl_ty

let (fold_union_ref : fold_union ref) = ref (not_implemented "fold_union")

type simplify_unions =
  env ->
  ?on_tyvar:(env -> Reason.t -> Ident.t -> env * locl_ty) ->
  locl_ty ->
  env * locl_ty

let (simplify_unions_ref : simplify_unions ref) =
  ref (not_implemented "simplify_unions")

let simplify_unions x = !simplify_unions_ref x

type approx =
  | ApproxUp
  | ApproxDown
[@@deriving eq]

type non = env -> Reason.t -> locl_ty -> approx:approx -> env * locl_ty

let (non_ref : non ref) = ref (not_implemented "non")

let non x = !non_ref x

type simplify_intersections =
  env ->
  ?on_tyvar:(env -> Reason.t -> int -> env * locl_ty) ->
  locl_ty ->
  env * locl_ty

let (simplify_intersections_ref : simplify_intersections ref) =
  ref (not_implemented "simplify_intersections")

let simplify_intersections x = !simplify_intersections_ref x

type localize_no_subst = env -> ignore_errors:bool -> decl_ty -> env * locl_ty

let (localize_no_subst_ref : localize_no_subst ref) =
  ref (not_implemented "localize_no_subst")

let localize_no_subst x = !localize_no_subst_ref x

type localize = ety_env:expand_env -> env -> decl_ty -> env * locl_ty

let (localize_ref : localize ref) =
  ref (fun ~ety_env:_ -> not_implemented "localize")

let localize x = !localize_ref x

(*****************************************************************************)
(* Checking properties of types *)
(*****************************************************************************)

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
  (is_sub_type_for_union ~coerce:None env dynamic ty && not (is_mixed env ty))
  || is_sub_type_for_union ~coerce:None env ty dynamic
     && not (is_nothing env ty)

let rec is_any env ty =
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tany _
  | Terr ->
    true
  | Tunion tyl -> List.for_all tyl (is_any env)
  | Tintersection tyl -> List.exists tyl (is_any env)
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
  | (_, Tdestructure _) ->
    (env, ConstraintType ty)

let contains_unresolved_tyvars env ty =
  let finder =
    object (this)
      inherit [env * bool] Type_visitor.locl_type_visitor as super

      method! on_tvar (env, occurs) r v =
        let (env, ty) = Env.expand_var env r v in
        if is_tyvar ty then
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
 *****************************************************************************)
let get_concrete_supertypes env ty =
  let rec iter seen env acc tyl =
    match tyl with
    | [] -> (env, acc)
    | ty :: tyl ->
      let (env, ty) = Env.expand_type env ty in
      (match get_node ty with
      (* Enums with arraykey upper bound are treated as "abstract" *)
      | Tnewtype (cid, _, bound_ty)
        when is_prim Aast.Tarraykey bound_ty && Env.is_enum env cid ->
        iter seen env acc tyl
      (* Don't expand enums or newtype; just return the type itself *)
      | Tnewtype (_, _, ty)
      | Tdependent (_, ty) ->
        iter seen env (TySet.add ty acc) tyl
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

(* Gets the base type of an abstract type *)
let rec get_base_type env ty =
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tnewtype (classname, _, _) when String.equal classname SN.Classes.cClassname
    ->
    ty
  (* If we have an expression dependent type and it only has one super
    type, we can treat it similarly to AKdependent _, Some ty  *)
  | Tgeneric (n, targs) when DependentKind.is_generic_dep_ty n ->
    begin
      match TySet.elements (Env.get_upper_bounds env n targs) with
      | ty2 :: _ when ty_equal ty ty2 -> ty
      (* If it's exactly equal, then the base ty is just this one *)
      | ty :: _ ->
        if TySet.mem ty (Env.get_lower_bounds env n targs) then
          ty
        else
          get_base_type env ty
      | [] -> ty
    end
  | Tnewtype (cid, _, bound_ty)
    when is_prim Aast.Tarraykey bound_ty && Env.is_enum env cid ->
    ty
  | Tgeneric _
  | Tnewtype _
  | Tdependent _ ->
    begin
      match get_concrete_supertypes env ty with
      (* If the type is exactly equal, we don't want to recurse *)
      | (_, ty2 :: _) when ty_equal ty ty2 -> ty
      | (_, ty :: _) -> get_base_type env ty
      | (_, []) -> ty
    end
  | _ -> ty

let get_printable_shape_field_name = Env.get_shape_field_name

let shape_field_name_ this field =
  Aast.(
    match field with
    | (p, Int name) -> Ok (Ast_defs.SFlit_int (p, name))
    | (p, String name) -> Ok (Ast_defs.SFlit_str (p, name))
    | (_, Class_const ((_, CI x), y)) -> Ok (Ast_defs.SFclass_const (x, y))
    | (_, Class_const ((_, CIself), y)) ->
      (match force this with
      | Some sid -> Ok (Ast_defs.SFclass_const (sid, y))
      | None -> Error `Expected_class)
    | _ -> Error `Invalid_shape_field_name)

let shape_field_name :
    env -> Pos.t * Nast.expr_ -> Ast_defs.shape_field_name option =
 fun env (p, field) ->
  let this =
    lazy
      (match Env.get_self_ty env with
      | None -> None
      | Some c_ty ->
        (match get_node c_ty with
        | Tclass (sid, _, _) -> Some (Positioned.unsafe_to_raw_positioned sid)
        | _ -> None))
  in
  match shape_field_name_ this (p, field) with
  | Ok x -> Some x
  | Error `Expected_class ->
    Errors.expected_class p;
    None
  | Error `Invalid_shape_field_name ->
    Errors.invalid_shape_field_name p;
    None

(*****************************************************************************)
(* Class types *)
(*****************************************************************************)

let string_of_visibility = function
  | Vpublic -> "public"
  | Vprivate _ -> "private"
  | Vprotected _ -> "protected"

let unwrap_class_type ty =
  match deref ty with
  | (r, Tapply (name, tparaml)) -> (r, name, tparaml)
  | ( _,
      ( Terr | Tdynamic | Tany _ | Tmixed | Tnonnull
      | Tdarray (_, _)
      | Tvarray _ | Tvarray_or_darray _ | Tvec_or_dict _ | Tgeneric _
      | Toption _ | Tlike _ | Tprim _ | Tfun _ | Ttuple _ | Tshape _ | Tunion _
      | Tintersection _
      | Taccess (_, _)
      | Tthis | Tvar _ ) ) ->
    raise @@ Invalid_argument "unwrap_class_type got non-class"

let try_unwrap_class_type x = Option.try_with (fun () -> unwrap_class_type x)

let class_is_final_and_not_contravariant class_ty =
  Cls.final class_ty
  && List.for_all (Cls.tparams class_ty) ~f:(function
         | { tp_variance = Ast_defs.Invariant | Ast_defs.Covariant; _ } -> true
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
        ~is_atom:false
        ~readonly:false;
  }

let tany = Env.tany

let mk_tany env p = mk (Reason.Rwitness p, tany env)

let mk_tany_ env p = mk (Reason.Rwitness_from_decl p, tany env)

let terr env r =
  let dynamic_view_enabled =
    TypecheckerOptions.dynamic_view (Typing_env.get_tcopt env)
  in
  if dynamic_view_enabled then
    MakeType.dynamic r
  else
    MakeType.err r

let collect_enum_class_upper_bounds env name =
  let rec collect seen result name =
    let upper_bounds = Env.get_upper_bounds env name [] in
    Typing_set.fold
      (fun lty (seen, result) ->
        match get_node lty with
        | Tclass ((_, name), _, _) when Env.is_enum_class env name ->
          (seen, SSet.add name result)
        | Tgeneric (name, _) when not (SSet.mem name seen) ->
          collect (SSet.add name seen) result name
        | _ -> (seen, result))
      upper_bounds
      (seen, result)
  in
  let (_, upper_bounds) = collect SSet.empty SSet.empty name in
  upper_bounds

let make_locl_subst_for_class_tparams classdef tyl =
  if List.is_empty tyl then
    SMap.empty
  else
    Decl_subst.make_locl (Cls.tparams classdef) tyl
