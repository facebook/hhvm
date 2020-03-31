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
module ShapeMap = Aast.ShapeMap
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

type sub_type = env -> locl_ty -> locl_ty -> Errors.typing_error_callback -> env

let (sub_type_ref : sub_type ref) = ref (not_implemented "sub_type")

let sub_type x = !sub_type_ref x

type sub_type_i =
  env -> internal_type -> internal_type -> Errors.typing_error_callback -> env

let (sub_type_i_ref : sub_type_i ref) = ref (not_implemented "sub_type_i")

let sub_type_i x = !sub_type_i_ref x

type sub_type_with_dynamic_as_bottom =
  env -> locl_ty -> locl_ty -> Errors.typing_error_callback -> env

let (sub_type_with_dynamic_as_bottom_ref : sub_type_with_dynamic_as_bottom ref)
    =
  ref (not_implemented "sub_type_with_dynamic_as_bottom")

let sub_type_with_dynamic_as_bottom x = !sub_type_with_dynamic_as_bottom_ref x

type is_sub_type_type = env -> locl_ty -> locl_ty -> bool

type is_sub_type_i_type = env -> internal_type -> internal_type -> bool

let (is_sub_type_ref : is_sub_type_type ref) =
  ref (not_implemented "is_sub_type")

let is_sub_type x = !is_sub_type_ref x

let (is_sub_type_for_union_ref : is_sub_type_type ref) =
  ref (not_implemented "is_sub_type_for_union")

let is_sub_type_for_union x = !is_sub_type_for_union_ref x

let (is_sub_type_for_union_i_ref : is_sub_type_i_type ref) =
  ref (not_implemented "is_sub_type_for_union_i")

let is_sub_type_for_union_i x = !is_sub_type_for_union_i_ref x

let (is_sub_type_ignore_generic_params_ref : is_sub_type_type ref) =
  ref (not_implemented "is_sub_type_ignore_generic_params")

let is_sub_type_ignore_generic_params x =
  !is_sub_type_ignore_generic_params_ref x

type add_constraint =
  Pos.Map.key -> env -> Ast_defs.constraint_kind -> locl_ty -> locl_ty -> env

let (add_constraint_ref : add_constraint ref) =
  ref (not_implemented "add_constraint")

let add_constraint x = !add_constraint_ref x

type expand_typeconst =
  expand_env ->
  env ->
  ?ignore_errors:bool ->
  ?as_tyvar_with_cnstr:bool ->
  locl_ty ->
  Aast.sid ->
  on_error:Errors.typing_error_callback ->
  allow_abstract_tconst:bool ->
  env * locl_ty

let (expand_typeconst_ref : expand_typeconst ref) =
  ref (not_implemented "expand_typeconst")

let expand_typeconst x = !expand_typeconst_ref x

type expand_pocket_universes =
  env ->
  ety_env:expand_env ->
  Reason.t ->
  locl_ty ->
  Pos.t * string ->
  (Pos.t * string) * Aast_defs.pu_loc ->
  Pos.t * string ->
  env * locl_ty

let (expand_pocket_universes_ref : expand_pocket_universes ref) =
  ref (not_implemented "expand_pocket_universes")

let expand_pocket_universes x = !expand_pocket_universes_ref x

type union = env -> locl_ty -> locl_ty -> env * locl_ty

let (union_ref : union ref) = ref (not_implemented "union")

let union x = !union_ref x

type union_i =
  env -> Reason.t -> internal_type -> locl_ty -> env * internal_type

let (union_i_ref : union_i ref) = ref (not_implemented "union")

let union_i x = !union_i_ref x

type union_list = env -> Reason.t -> locl_ty list -> env * locl_ty

let (union_list_ref : union_list ref) = ref (not_implemented "union_list")

let union_list x = !union_list_ref x

type fold_union = env -> Reason.t -> locl_ty list -> env * locl_ty

let (fold_union_ref : fold_union ref) = ref (not_implemented "fold_union")

let fold_union x = !fold_union_ref x

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

type localize_with_self =
  env -> ?pos:Pos.t -> ?quiet:bool -> decl_ty -> env * locl_ty

let (localize_with_self_ref : localize_with_self ref) =
  ref (not_implemented "localize_with_self")

let localize_with_self x = !localize_with_self_ref x

(* Convenience function for creating `this` types *)
let this_of ty = Tdependent (DTthis, ty)

(*****************************************************************************)
(* Returns true if a type is optional *)
(*****************************************************************************)

let is_option env ty =
  let null = MakeType.null Reason.Rnone in
  is_sub_type_for_union env null ty

let is_mixed_i env ty =
  let mixed = LoclType (MakeType.mixed Reason.Rnone) in
  is_sub_type_for_union_i env mixed ty

let is_mixed env ty = is_mixed_i env (LoclType ty)

let is_nothing_i env ty =
  let nothing = LoclType (MakeType.nothing Reason.Rnone) in
  is_sub_type_for_union_i env ty nothing

let is_nothing env ty = is_nothing_i env (LoclType ty)

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

(* Grab all supertypes of a given type, recursively *)
let get_all_supertypes env ty =
  let rec iter seen env acc tyl =
    match tyl with
    | [] -> (env, acc)
    | ty :: tyl ->
      let (env, ty) = Env.expand_type env ty in
      (match get_node ty with
      | Tnewtype (_, _, ty)
      | Tdependent (_, ty) ->
        iter seen env (TySet.add ty acc) tyl
      | Tgeneric n ->
        if SSet.mem n seen then
          iter seen env acc tyl
        else
          iter
            (SSet.add n seen)
            env
            acc
            (TySet.elements (Env.get_upper_bounds env n) @ tyl)
      | _ -> iter seen env (TySet.add ty acc) tyl)
  in
  let (env, resl) = iter SSet.empty env TySet.empty [ty] in
  (env, TySet.elements resl)

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
      | Tgeneric n ->
        if SSet.mem n seen then
          iter seen env acc tyl
        else
          iter
            (SSet.add n seen)
            env
            acc
            (TySet.elements (Env.get_upper_bounds env n) @ tyl)
      | Tunion tyl' ->
        let tys = TySet.of_list tyl' in
        begin
          match TySet.elements tys with
          | [ty] -> iter seen env acc (ty :: tyl)
          | _ -> iter seen env (TySet.add ty acc) tyl
        end
      | _ -> iter seen env (TySet.add ty acc) tyl)
  in
  let (env, resl) = iter SSet.empty env TySet.empty [ty] in
  (env, TySet.elements resl)

(* Try running function on each concrete supertype in turn. Return all
 * successful results
 *)
let try_over_concrete_supertypes env ty f =
  let (env, tyl) = get_concrete_supertypes env ty in
  (* If there is just a single result then don't swallow errors *)
  match tyl with
  | [ty] -> [f env ty]
  | _ ->
    let rec iter_over_types env resl tyl =
      match tyl with
      | [] -> resl
      | ty :: tyl ->
        Errors.try_
          (fun () -> iter_over_types env (f env ty :: resl) tyl)
          (fun _ -> iter_over_types env resl tyl)
    in
    iter_over_types env [] tyl

(** Run a function on an intersection represented by a list of types.
Similarly to try_over_concrete_supertypes, we stay liberal with errors:
discard the result of any run which has produced an error.
If all runs have produced an error, gather all errors and results and add errors. *)
let run_on_intersection :
    'env -> f:('env -> locl_ty -> 'env * 'a) -> locl_ty list -> 'env * 'a list =
 fun env ~f tyl ->
  let (env, resl_errors) =
    List.map_env env tyl ~f:(fun env ty ->
        Errors.try_with_result
          (fun () ->
            let (env, res) = f env ty in
            (env, (res, None)))
          (fun (_, (res, _)) err -> (env, (res, Some err))))
  in
  let valid_resl =
    List.filter resl_errors ~f:(fun (_, err) -> Option.is_none err)
    |> List.map ~f:fst
  in
  let resl =
    if not (List.is_empty valid_resl) then
      valid_resl
    else (
      List.iter resl_errors ~f:(fun (_, err) ->
          Option.iter err ~f:Errors.add_error);
      List.map ~f:fst resl_errors
    )
  in
  (env, resl)

(*****************************************************************************)
(* Dynamicism  *)
(*****************************************************************************)
let is_dynamic env ty =
  let dynamic = MakeType.dynamic Reason.Rnone in
  (is_sub_type_for_union env dynamic ty && not (is_mixed env ty))
  || (is_sub_type_for_union env ty dynamic && not (is_nothing env ty))

(*****************************************************************************)
(* Check if type is any or a variant thereof  *)
(*****************************************************************************)

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

(*****************************************************************************)
(* Gets the base type of an abstract type *)
(*****************************************************************************)

let rec get_base_type env ty =
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tnewtype (classname, _, _) when String.equal classname SN.Classes.cClassname
    ->
    ty
  (* If we have an expression dependent type and it only has one super
    type, we can treat it similarly to AKdependent _, Some ty  *)
  | Tgeneric n when DependentKind.is_generic_dep_ty n ->
    begin
      match TySet.elements (Env.get_upper_bounds env n) with
      | ty2 :: _ when ty_equal ty ty2 -> ty
      (* If it's exactly equal, then the base ty is just this one *)
      | ty :: _ ->
        if TySet.mem ty (Env.get_lower_bounds env n) then
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

(*****************************************************************************)
(* Given some class type or unresolved union of class types, return the
 * identifiers of all classes the type may represent.
 *
 * Intended for uses like constructing call graphs and finding references, where
 * we have the statically known class type of some runtime value or class ID and
 * we would like the name of that class. *)
(*****************************************************************************)
let get_class_ids env ty =
  let rec aux seen acc ty =
    match get_node ty with
    | Tclass ((_, cid), _, _) -> cid :: acc
    | Toption ty
    | Tdependent (_, ty)
    | Tnewtype (_, _, ty) ->
      aux seen acc ty
    | Tunion tys
    | Tintersection tys ->
      List.fold tys ~init:acc ~f:(aux seen)
    | Tgeneric name when not (List.mem ~equal:String.equal seen name) ->
      let seen = name :: seen in
      let upper_bounds = Env.get_upper_bounds env name in
      TySet.fold (fun ty acc -> aux seen acc ty) upper_bounds acc
    | _ -> acc
  in
  List.rev (aux [] [] (Typing_expand.fully_expand env ty))

(*****************************************************************************)
(* Reactivity *)
(*****************************************************************************)

let reactivity_to_string env r =
  let cond_reactive prefix t =
    let str = Typing_print.full_decl (Env.get_ctx env) t in
    prefix ^ " (condition type: " ^ str ^ ")"
  in
  let rec aux r =
    match r with
    | Pure None -> "pure"
    | Pure (Some ty) -> cond_reactive "conditionally pure" ty
    | Reactive None -> "reactive"
    | Reactive (Some ty) -> cond_reactive "conditionally reactive" ty
    | Shallow None -> "shallow reactive"
    | Shallow (Some ty) -> cond_reactive "conditionally shallow reactive" ty
    | Local None -> "local reactive"
    | Local (Some ty) -> cond_reactive "conditionally local reactive" ty
    | MaybeReactive n -> "maybe (" ^ aux n ^ ")"
    | Nonreactive -> "non-reactive"
    | RxVar _ -> "maybe reactive"
  in
  aux r

let get_printable_shape_field_name = Env.get_shape_field_name

let shape_field_name_ env field =
  Aast.(
    match field with
    | (p, Int name) -> Ok (Ast_defs.SFlit_int (p, name))
    | (p, String name) -> Ok (Ast_defs.SFlit_str (p, name))
    | (_, Class_const ((_, CI x), y)) -> Ok (Ast_defs.SFclass_const (x, y))
    | (_, Class_const ((_, CIself), y)) ->
      let c_ty = get_node (Env.get_self env) in
      (match c_ty with
      | Tclass (sid, _, _) -> Ok (Ast_defs.SFclass_const (sid, y))
      | _ -> Error `Expected_class)
    | _ -> Error `Invalid_shape_field_name)

let shape_field_name env (p, field) =
  match shape_field_name_ env (p, field) with
  | Ok x -> Some x
  | Error `Expected_class ->
    Errors.expected_class p;
    None
  | Error `Invalid_shape_field_name ->
    Errors.invalid_shape_field_name p;
    None

(*****************************************************************************)
(* *)
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
      | Tarray (_, _)
      | Tdarray (_, _)
      | Tvarray _ | Tvarray_or_darray _ | Tgeneric _ | Toption _ | Tlike _
      | Tprim _ | Tfun _ | Ttuple _ | Tshape _ | Tunion _ | Tintersection _
      | Taccess (_, _)
      | Tthis | Tpu_access _ | Tvar _ ) ) ->
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
  let merge x y = Option.merge x y (fun x _ -> x)

  let visitor =
    object (this)
      inherit [Reason.t option] Type_visitor.locl_type_visitor

      method! on_tany _ r = Some r

      method! on_tarraykind acc _r akind =
        match akind with
        | AKvarray ty -> this#on_type acc ty
        | AKdarray (tk, tv)
        | AKvarray_or_darray (tk, tv) ->
          merge (this#on_type acc tk) (this#on_type acc tv)
    end

  let check_why ty = visitor#on_type None ty

  let check ty = Option.is_some (check_why ty)
end

(*****************************************************************************)
(* Function parameters *)
(*****************************************************************************)

let default_fun_param ?(pos = Pos.none) ty : 'a fun_param =
  {
    fp_pos = pos;
    fp_name = None;
    fp_type = { et_type = ty; et_enforced = false };
    fp_kind = FPnormal;
    fp_accept_disposable = false;
    fp_mutability = None;
    fp_rx_annotation = None;
  }

let fun_mutable user_attributes =
  let rec go = function
    | [] -> None
    | { Aast.ua_name = (_, n); _ } :: _
      when String.equal n SN.UserAttributes.uaMutable ->
      Some Param_borrowed_mutable
    | { Aast.ua_name = (_, n); _ } :: _
      when String.equal n SN.UserAttributes.uaMaybeMutable ->
      Some Param_maybe_mutable
    | _ :: tl -> go tl
  in
  go user_attributes

let tany = Env.tany

let mk_tany env p = mk (Reason.Rwitness p, tany env)

let decl_tany = Env.decl_tany

let terr env r =
  let dynamic_view_enabled =
    TypecheckerOptions.dynamic_view (Typing_env.get_tcopt env)
  in
  if dynamic_view_enabled then
    MakeType.dynamic r
  else
    MakeType.err r

(* Hacked version of Typing_subtype.try_intersect for collecting function types *)
let add_function_type env fty logged =
  let (untyped_ftys, ftys) = logged in
  let rec try_intersect env ty tyl =
    match tyl with
    | [] -> [ty]
    | ty' :: tyl' ->
      if is_sub_type_for_union env ty ty' && not (HasTany.check ty) then
        try_intersect env ty tyl'
      else if is_sub_type_for_union env ty' ty && not (HasTany.check ty') then
        try_intersect env ty' tyl'
      else
        ty' :: try_intersect env ty tyl'
  in
  if HasTany.check fty then
    (try_intersect env fty untyped_ftys, ftys)
  else
    (untyped_ftys, try_intersect env fty ftys)

let rec class_get_pu_ env cty name =
  let (env, ety) = Env.expand_type env cty in
  match get_node ety with
  | Tany _
  | Terr
  | Tdynamic
  | Tunion _ ->
    (env, None)
  | Tgeneric _ ->
    (* TODO(T36532263) check for PU in upper bounds (see D18395326) *)
    (env, None)
  | Tvar _
  | Tnonnull
  | Tarraykind _
  | Toption _
  | Tprim _
  | Tfun _
  | Ttuple _
  | Tobject
  | Tshape _ ->
    (env, None)
  | Tintersection _ -> (env, None)
  | Tpu_type_access _
  | Tpu _ ->
    (env, None)
  | Tnewtype (_, _, ty)
  | Tdependent (_, ty) ->
    class_get_pu_ env ty name
  | Tclass ((_, c), _, paraml) ->
    let class_ = Env.get_class env c in
    begin
      match class_ with
      | None -> (env, None)
      | Some class_ ->
        (match Env.get_pu_enum env class_ name with
        | Some et ->
          (env, Some (cty, Decl_subst.make_locl (Cls.tparams class_) paraml, et))
        | None -> (env, None))
    end

let class_get_pu ?from_class env ty name =
  let open Option in
  let (env, pu) = class_get_pu_ env ty name in
  ( env,
    pu >>= fun (this_ty, substs, et) ->
    let ety_env =
      {
        type_expansions = [];
        this_ty;
        substs;
        from_class;
        quiet = false;
        on_error = Errors.unify_error;
      }
    in
    Some (ety_env, et) )

let class_get_pu_member ?from_class env ty enum name =
  let open Option in
  let (env, enum) = class_get_pu ?from_class env ty enum in
  ( env,
    enum >>= fun (ety_env, pu) ->
    SMap.find_opt name pu.tpu_members >>= fun member -> Some (ety_env, member)
  )

let class_get_pu_member_type ?from_class env ty enum member name =
  let open Option in
  let (env, member) = class_get_pu_member ?from_class env ty enum member in
  ( env,
    member >>= fun (ety_env, member) ->
    SMap.find_opt name member.tpum_types >>= fun dty -> Some (ety_env, dty) )
